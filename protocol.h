#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define ACTION_WITHDRAW 1
#define ACTION_DEPOSIT  2
#define ACTION_BALANCE  3
#define ACTION_SYNC     4
#define ACTION_STARTUP_REQ 5
#define ACTION_REGISTER 6 

#define MAX_USERS 100

typedef struct {
    int v[4]; 
} VectorClock;

typedef struct {
    int accountID;
    int balance;
    int active; 
} UserAccount;

typedef struct {
    char ip[20];
    int port;
} Peer;

static const Peer CLUSTER[] = {
    {"127.0.0.1", 9001}, 
    {"127.0.0.1", 9002},
    {"127.0.0.1", 9003}, 
    {"127.0.0.1", 9004}
};
#define CLUSTER_SIZE (sizeof(CLUSTER) / sizeof(Peer))

typedef struct {
    int action;
    int clientID;
    int amount;
    int balance;
    VectorClock vclock;
    char message[100];
} MobileMoneyPacket;

#endif