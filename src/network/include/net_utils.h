#ifndef _UTILS_H_
#define _UTILS_H_

#include "type.h"

void SetNonBlockSock(uint32 fd);
void SetCloexecSock(uint32 fd);
void EpollAddSock(uint32 epoll, uint32 fd, uint32 event);
void EpollDelSock(uint32 epoll, uint32 fd);
void EpollModSock(uint32 epoll, uint32 fd, uint32 event);

#endif