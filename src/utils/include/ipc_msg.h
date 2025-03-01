#ifndef _IPC_MSG_H_
#define _IPC_MSG_H_

struct IPCMsg {
    long type;
    uint8_t data;
};

class IPCMsgQueue {
public:
    IPCMsgQueue();
    ~IPCMsgQueue();
    void SendMsg(const IPCMsg& msg);
    bool RecvMsg(long msgType, IPCMsg& msg);
    bool IsEmpty();
private:
    int _key;
    int _msgId;
public:
    static constexpr int IPCMsgLen = sizeof(IPCMsg::data);
};

#endif