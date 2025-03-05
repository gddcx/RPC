#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include "keeper_server.h"

using namespace crpc;

int main() {
    KeeperServer keeperServer;

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    return 0;
}