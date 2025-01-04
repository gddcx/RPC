#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <utility>

#include "tcp_server.h"
#include "net_utils.h"

namespace crpc {

TcpServer::~TcpServer() {
    CloseServer();
}

uint32 TcpServer::_WriteSocket(uint32 fd) {
    int byteNum = 0;
    int hasSendByteNum = 0;
    int totalByteNum = 0;

    std::unique_lock<std::mutex> lock(_channelMutex);
    TcpChannel& channel = _channels[fd];
    lock.unlock();
    
    channel._sendBuffer.Lock();
    std::vector<char> sendBuffer = channel._sendBuffer.GetBuffer();
    channel._sendBuffer.Unlock();
    totalByteNum = sendBuffer.size();
    
    while(hasSendByteNum < totalByteNum) {
        byteNum = send(fd, sendBuffer.data() + hasSendByteNum, totalByteNum - hasSendByteNum, MSG_DONTWAIT | MSG_NOSIGNAL);
        if(byteNum < 0) {
            if (errno == EAGAIN) {
                std::cout << __func__ << ">>> " << "EAGAIN. " << hasSendByteNum << "/" << totalByteNum << std::endl;
                continue; // TODO: 长时间EAGAIN的话阻塞线程，怎么优化?
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

uint32 TcpServer::_ReadSocket(uint32 fd) {
    uint32 totalBytes = 0;
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

    uint32 appendByteNum = channel._recvBuffer.AppendBuffer(recvBuffer, 0, totalBytes);
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

void TcpServer::_ProcessMessage() {
    std::cout << "Thread[" << std::this_thread::get_id() << "] start" << std::endl;
    while(true) {
        Message msg;
        {
            std::unique_lock<std::mutex> lock(_taskQueMutex);
            _taskCV.wait(lock, [this](){return !_taskQueue.empty() || !_running;}); // 消息队列不为空 || Server正在关闭
            if(!_running && _taskQueue.empty()) {
                return;
            }

            msg = _taskQueue.front();
            _taskQueue.pop();
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

void TcpServer::_InitThreadPool(uint32 threadNum) {
    std::cout << __func__ << ">>> " << "create thread poll. thread num=" << threadNum << std::endl;
    for(uint32 n = 0; n < threadNum; n++) {
        _threadPool.emplace_back(&TcpServer::_ProcessMessage, this);
    }
}

int TcpServer::_InitServerSocket(uint32 port) {
    _epollFd = epoll_create1(EPOLL_CLOEXEC);

    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_serverFd == -1) {
        std::cout << __func__ << ">>> " << "create server socket fail" << std::endl;
        return -1;
    }

    sockaddr_in sockInfo;
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    sockInfo.sin_port = htons(port);
    bind(_serverFd, (sockaddr*)&sockInfo, sizeof(sockaddr_in));

    SetNonBlockSock(_serverFd);
    EpollAddSock(_epollFd, _serverFd, EPOLLIN);

    if(listen(_serverFd, SOMAXCONN) == 0) {
        std::cout << __func__ << ">>> " << "start listening port:" << port << std::endl;
        return 0;
    }

    std::cout << __func__ << ">>> " << "start server fail" << port << std::endl;
    return -1;
}

uint32 TcpServer::InitServer(uint32 port, uint32 threadNum, uint32 recvBufferSize = 4096) {
    _recvBufferSize = recvBufferSize;
    _InitThreadPool(threadNum);
    return _InitServerSocket(port);
}

void TcpServer::_AcceptNewConn() {
    sockaddr_in sockInfo;
    uint32 sockInfoLen = sizeof(sockaddr_in);

    memset(&sockInfo, 0, sizeof(sockaddr));
    int clientFd = accept(_serverFd, (sockaddr*)&sockInfo, &sockInfoLen);
    if(clientFd == -1) {
        std::cout << __func__ << ">>> " << "accept new connection fail. errno:" << errno << std::endl;
    }
    else {
        SetNonBlockSock(clientFd);
        EpollAddSock(_epollFd, clientFd, EPOLLIN | EPOLLET);
        std::cout << __func__ << ">>> " << "accept new connection succ. peer ip:" << ntohl(sockInfo.sin_addr.s_addr) << ", peer port:" << ntohs(sockInfo.sin_port) << std::endl;
        {
            std::lock_guard<std::mutex> lock(_channelMutex); // TODO: 做定时器关闭连接
            _channels.emplace(clientFd, TcpChannel(clientFd, _recvBufferSize)); // 有三次构造，临时对象生成A，_channel内调用TcpChannel默认构造生成B，A拷贝构造覆盖B
        }
        if(_onConnect) {
            _onConnect(clientFd);
        }
    }
}

void TcpServer::_EventHandle(uint32 fd, uint32 event) {
    switch(event) {
        case EPOLLOUT:
        {
            std::unique_lock<std::mutex> channLock(_channelMutex);
            TcpChannel& channel = _channels[fd];
            channLock.unlock();

            channel._sendBuffer.Lock();
            if(channel._sendBuffer.GetBufferSize() == 0) { // 没有数据要写了
                EpollModSock(_epollFd, fd, EPOLLIN | EPOLLET);
                channel._sendBuffer.Unlock();
                break;
            }
            channel._sendBuffer.Unlock();

            {
                std::lock_guard<std::mutex> queLock(_taskQueMutex);
                _taskQueue.emplace(fd, Writable);
            }
            _taskCV.notify_one();
            break;
        }
        case EPOLLIN:
        {
            {
                std::lock_guard<std::mutex> lock(_taskQueMutex);
                _taskQueue.emplace(fd, Readable);
            }
            _taskCV.notify_one();
            break;
        }
        default:
            break;
    }
}

void TcpServer::StartServer() {
    std::cout << __func__ << ">>> " << "start listening I/O events." << std::endl;

    epoll_event ev[512];
    uint32 maxEvLen = sizeof(ev) / sizeof(ev[0]);
    uint32 eventNum = 0;

    _running = true;
    while(true)
    {
        if(!_running) break;
        eventNum = epoll_wait(_epollFd, &ev[0], maxEvLen, -1);
        for(uint32 idx = 0; idx < eventNum; idx++) {
            if(ev[idx].data.fd == _serverFd) {    // 新连接
                _AcceptNewConn();
            }
            else {    // I/O读写事件
                _EventHandle(ev[idx].data.fd, ev[idx].events);
            }
        }
    }
}

void TcpServer::CloseServer() {
    _running = false;

    if(_epollFd != -1) close(_epollFd); // epoll_wait期间close(_epollFd)会怎么样
    if(_serverFd != -1) close(_serverFd);

    std::lock_guard<std::mutex> lock(_channelMutex);
    for(auto& pair: _channels) {
        if(_onClose) _onClose(pair.first);
        close(pair.first);
    }

    _taskCV.notify_all();
    for(auto& thread: _threadPool) { // 等待还有任务的线程执行完。因为这里join所有任务/空闲线程，所以要notify_all把空闲线程也唤醒
        if(thread.joinable()) thread.join();
    }
}

void TcpServer::SendMsg(int fd, std::vector<char>& sendBuffer) {
    std::unique_lock<std::mutex> lock(_channelMutex);
    TcpChannel& channel = _channels[fd];
    lock.unlock();

    channel._sendBuffer.Lock();
    channel._sendBuffer.AppendBuffer(sendBuffer);
    channel._sendBuffer.Unlock();
    EpollModSock(_epollFd, fd, EPOLLIN | EPOLLOUT | EPOLLET);
}

void TcpServer::Disconnection(int fd) {
    std::lock_guard<std::mutex> lock(_channelMutex);
    _channels[fd].closeFlag = true;
}

void TcpServer::SetOnMessage(const OnMessageCallback& onMessage) {
    _onMessage = onMessage;
}

void TcpServer::SetOnConnect(const OnConnectCallback& onConnect) {
    _onConnect = onConnect;
}

void TcpServer::SetOnClose(const OnCloseCallback& onClose) {
    _onClose = onClose;
}

} // namespace crpc