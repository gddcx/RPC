#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
// #include <unistd.h>
#include <iostream>
#include "ipc_msg.h"

#define FILE_NAME "./ipc_msg"
#define MSQ_ID    0x5a
IPCMsgQueue::IPCMsgQueue() {
    _key = ftok(FILE_NAME, MSQ_ID);
    if(_key == -1) {
        std::cout << __func__ << "<<< file not exist" << std::endl;
        FILE* fd;
        if((fd = fopen(FILE_NAME, "w")) == nullptr) {
            std::cout << __func__ << "<<< create file failed" << std::endl;
            return;
        }
        fclose(fd);
        _key = ftok(FILE_NAME, MSQ_ID);
        if(_key == -1) {
            std::cout << __func__ << "<<< create ipc key failed" << std::endl;
            return;
        }
    }
    _msgId = msgget(_key, IPC_CREAT | 0666);
    if(_msgId == -1) {
        std::cout << __func__ << "<<< create msg queue failed" << std::endl;
        return;
    }
}

IPCMsgQueue::~IPCMsgQueue() {
    msgctl(_msgId, IPC_RMID, nullptr);
}

void IPCMsgQueue::SendMsg(const IPCMsg& msg) {
    if(msgsnd(_msgId, &msg, sizeof(msg.data), IPC_NOWAIT) == -1) {
        std::cout << __func__ << "<<< send msg failed. errno:" << errno << std::endl;
    }
}

bool IPCMsgQueue::RecvMsg(long msgType, IPCMsg& msg) {
    if(msgrcv(_msgId, &msg, sizeof(msg.data), msgType, IPC_NOWAIT) == -1) {
        std::cout << __func__ << "<<< recv msg failed. errno:" << errno << std::endl;
        return false;
    }
    return true;
}

bool IPCMsgQueue::IsEmpty() {
    msqid_ds ds;
    if(msgctl(_msgId, IPC_STAT, &ds) == -1) {
        std::cout << __func__ << "<<< get msg queue status failed. errno:" << errno << std::endl;
        return true;
    }
    return ds.msg_qnum == 0;
}