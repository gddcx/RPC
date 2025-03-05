#include <unistd.h>
#include <iostream>
#include <thread>
#include "rpc_server.h"

using namespace std;
using namespace crpc;

int main(int argc, char *args[]) {
    int netThread = atoi(args[1]);
    int workThread = atoi(args[2]);
    RpcServer rpcServer(netThread, workThread);
    rpcServer.SetKeeper("127.0.0.1", 50001);
    rpcServer.Main();

    return 0;
}