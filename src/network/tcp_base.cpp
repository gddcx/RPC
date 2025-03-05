#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <fcntl.h>
#include "tcp_base.h"
#include "net_utils.h"

namespace crpc
{

TcpBase::TcpBase(int threadNum): _threadNum(threadNum), _threadPool(threadNum) {}

TcpBase::~TcpBase() {
    _CommClose();
}

int TcpBase::_WriteSocket(int fd) {
    int byteNum = 0;
    int hasSendByteNum = 0;
    int totalByteNum = 0;

    std::unique_lock<std::mutex> lock(_channelMutex);
    TcpChannel& channel = _channels[fd];
    lock.unlock();
    
    channel._sendBuffer.Lock();
    std::vector<char> sendBuffer = channel._sendBuffer.GetBuffer(); // TODO：改成用链表，完全send出去后再删除数据，否则等下一次EPOLLOUT
    channel._sendBuffer.Unlock();
    totalByteNum = sendBuffer.size();
    
    while(hasSendByteNum < totalByteNum) {
        byteNum = send(fd, sendBuffer.data() + hasSendByteNum, totalByteNum - hasSendByteNum, MSG_DONTWAIT | MSG_NOSIGNAL);
        if(byteNum < 0) {
            if (errno == EAGAIN) {
                std::cout << __func__ << ">>> " << "EAGAIN. " << hasSendByteNum << "/" << totalByteNum << std::endl;
                continue; // TODO: 这里需要break，等待下一次EPOLLOUT，但是需要保留未发送完毕的数据
            } else if(errno == EINTR) {
                continue;
            } else {
                std::cout << __func__ << ">>> " << "send err. errno=" << errno << std::endl;
                break;
            }
        } else {
            hasSendByteNum += byteNum;
        }
    }

    channel._sendBuffer.pendingTaskNum.fetch_sub(1);
    /* SendMsg和Disconnect是顺序执行，closeFlag为true就不应该再往缓冲队列加数据，所以没必要保证pendingTaskNum.load()和关闭socket的整体原子性 */
    if(channel.closeFlag && channel._sendBuffer.pendingTaskNum.load() == 0) {
        EpollDelSock(_epollFd, fd);
        close(fd);
        std::lock_guard<std::mutex> lock(_channelMutex);
        _channels.erase(fd);
    }

    return 0;
}

int TcpBase::_ReadSocket(int fd) {
    int totalBytes = 0;
    int byteNum = 0;
    std::vector<char> recvBuffer(_recvBufferSize, 0);
    char* bufAddr = recvBuffer.data();

    while(totalBytes < _recvBufferSize) {
        byteNum = recv(fd, bufAddr+totalBytes, _recvBufferSize-totalBytes, MSG_DONTWAIT);
        if(byteNum < 0) {
            if (errno == EAGAIN) break; // 连接正常但是没有更多数据可以读了
            else if(errno == EINTR) continue; // recv被中断
            else { // TODO: 其它异常怎么处理
                std::cout << __func__ << ">>> " << "recv err. errno:" << errno << std::endl;
                return -1;
            }
        }
        else if (byteNum == 0) { // 对端关闭了连接
            std::cout << __func__ << ">>> " << "peer close connection." << std::endl;
            if(_onClose) _onClose(fd); // 业务层决定关闭fd的时机
            return 0;
        }
        else {
            totalBytes += byteNum;
        }
    }

    if(!_onMessage) {
        std::cout << __func__ << ">>> " <<  "No definition of _onMessage function" << std::endl;
        return totalBytes;
    }

    std::unique_lock<std::mutex> lock(_channelMutex);  // 可以利用shared_mutex实现读写锁，读可以并行操作
    TcpChannel& channel = _channels[fd];
    lock.unlock();

    int appendByteNum = channel._recvBuffer.AppendBuffer(recvBuffer, 0, totalBytes);
    if(appendByteNum < totalBytes) {  // 缓冲区的空间不足以写入所有数据
        _onMessage(channel._clientFd, channel._recvBuffer);
        if(channel._recvBuffer.AppendBuffer(recvBuffer, appendByteNum, totalBytes-appendByteNum) != totalBytes-appendByteNum) {
            std::cout << __func__ << ">>> " << "buffer size(" << _recvBufferSize << ") less than a whole protocol data size." << std::endl;
            channel._recvBuffer.ClearBuffer();  // 无效数据永远都处理不了，清除
        } else {
            _onMessage(channel._clientFd, channel._recvBuffer);
        }
    } else {
        _onMessage(channel._clientFd, channel._recvBuffer);
    }

    return totalBytes;
}

void TcpBase::_ProcessMessage(int threadIndex) {
    std::cout << "Thread[" << std::this_thread::get_id() << "] start" << std::endl;
    ThreadPara& thread = _threadPool[threadIndex];
    while(true) {
        Message msg;
        {
            std::unique_lock<std::mutex> lock(thread.taskQueMutex);
            thread.cv.wait(lock, [&](){return !thread.taskQueue.empty() || !_running;}); // 消息队列不为空 || Server正在关闭
            if(!_running && thread.taskQueue.empty()) {
                return;
            }

            msg = thread.taskQueue.front();
            thread.taskQueue.pop();
        }

        switch(msg._msgType) {
            case Writable:
                _WriteSocket(msg._fd);
                break;
            case Readable:
                _ReadSocket(msg._fd);
                break;
            default:
                break;
        }
    }
}

void TcpBase::_InitThreadPool() {
    std::cout << __func__ << ">>> " << "create thread poll. thread num=" << _threadNum << std::endl;
    for(int n = 0; n < _threadNum; n++) {
        /* 
            因为condition_variable、mutex 没有拷贝构造、移动构造函数，push_back、emplace_back这些会触发扩容(新内存上拷贝构造)的操作都会导致编译报错 
            只能先分配足够大的空间，然后修改旧元素
        */
        _threadPool[n].t = std::thread(&TcpBase::_ProcessMessage, this, n);
    }
}

void TcpBase::_EventHandle(uint32 fd, uint32 event) {
    if(event & EPOLLOUT)
    {
        std::unique_lock<std::mutex> channLock(_channelMutex);
        if(_channels.find(fd) == _channels.end()) {
            return;
        }
        TcpChannel& channel = _channels[fd];
        channLock.unlock();

        if(channel._sendBuffer.noBeginTaskNum.load() > 0) { // 判断当前有没有数据要写
            ThreadPara& thread = _threadPool[(fd + 1) % _threadNum]; // 确保同一个fd只在一个线程中send, fd+1实现读写并行
            {
                std::lock_guard<std::mutex> queLock(thread.taskQueMutex);
                thread.taskQueue.emplace(fd, Writable);
            }
            channel._sendBuffer.noBeginTaskNum.fetch_sub(1); // 这里是单线程，所以不用CAS
            thread.cv.notify_one();
        }
    }
    if(event & EPOLLIN)
    { 
        ThreadPara& thread = _threadPool[fd % _threadNum]; // 确保同一个fd只在一个线程中recv
        {
            std::lock_guard<std::mutex> lock(thread.taskQueMutex);
            thread.taskQueue.emplace(fd, Readable);
        }
        thread.cv.notify_one();
    }
}

void TcpBase::_CommonInit(int recvBufferSize) {
    _recvBufferSize = recvBufferSize;
    _epollFd = epoll_create1(EPOLL_CLOEXEC);
    if(_epollFd == -1) {
        std::cout << "_epollFd=" << -1 << "errno:" << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    _InitThreadPool();
}

void TcpBase::SendMsg(int fd, const std::vector<char>& sendBuffer) {
    std::unique_lock<std::mutex> lock(_channelMutex);
    if(_channels.find(fd) == _channels.end()) {
        return;
    }
    TcpChannel& channel = _channels[fd];
    lock.unlock();

    channel._sendBuffer.Lock();
    channel._sendBuffer.AppendBuffer(sendBuffer);
    channel._sendBuffer.Unlock();
    EpollModSock(_epollFd, fd, EPOLLIN | EPOLLOUT | EPOLLET); // 触发EPOLLOUT事件
}

void TcpBase::_CommClose() {
    _running = false;

    for(auto& thread: _threadPool) { // 等待还有任务的线程执行完
        thread.cv.notify_one();
        if(thread.t.joinable()) thread.t.join();
    }

    while(_channels.size() != 0) {
        Disconnection(_channels.begin()->first);
    }

    if(_epollFd != -1) close(_epollFd); // TODO: epoll_wait期间close(_epollFd)会怎么样
}

void TcpBase::Disconnection(int fd) {
    std::lock_guard<std::mutex> lock(_channelMutex);
    if(_channels.find(fd) == _channels.end()) {
        return;
    }

    _channels[fd].closeFlag = true;
    if(_channels[fd]._sendBuffer.pendingTaskNum.load() == 0) {
        EpollDelSock(_epollFd, fd);
        close(fd);
        _channels.erase(fd);
    }
}

void TcpBase::SetOnMessage(const OnMessageCallback& onMessage) {
    _onMessage = onMessage;
}

void TcpBase::SetOnConnect(const OnConnectCallback& onConnect) {
    _onConnect = onConnect;
}

void TcpBase::SetOnClose(const OnCloseCallback& onClose) {
    _onClose = onClose;
}

}