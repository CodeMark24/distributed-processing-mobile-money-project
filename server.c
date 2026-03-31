#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

UserAccount ledger[MAX_USERS] = {0};
VectorClock local_vc = {{0, 0, 0, 0}};
int next_uid = 1000; 

void merge_clocks(VectorClock remote) {
    for(int i = 0; i < 4; i++) {
        if(remote.v[i] > local_vc.v[i]) local_vc.v[i] = remote.v[i];
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) return printf("Usage: server.exe <Index>\n"), 1;
    int myIdx = atoi(argv[1]);
    
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(CLUSTER[myIdx].port), .sin_addr.s_addr = INADDR_ANY };
    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    printf("[NODE %d] Online. Vector Clock enabled.\n", myIdx);

    MobileMoneyPacket pkt;
    struct sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    while (1) {
        if (recvfrom(s, (char *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&clientAddr, &addrLen) > 0) {
            merge_clocks(pkt.vclock);
            local_vc.v[myIdx]++;

            int idx = pkt.clientID % MAX_USERS;

            if (pkt.action == ACTION_REGISTER) {
                int newID = next_uid++;
                ledger[newID % MAX_USERS] = (UserAccount){.accountID = newID, .balance = 5000, .active = 1};
                pkt.clientID = newID;
                pkt.balance = 5000;
                sprintf(pkt.message, "Account Registered. ID: %d", newID);
            } 
            else if (pkt.action == ACTION_DEPOSIT) {
                if(ledger[idx].active) {
                    ledger[idx].balance += pkt.amount;
                    pkt.balance = ledger[idx].balance;
                    sprintf(pkt.message, "Deposit Success.");
                } else sprintf(pkt.message, "ID Not Found.");
            }
            else if (pkt.action == ACTION_WITHDRAW) {
                if(ledger[idx].active && ledger[idx].balance >= pkt.amount) {
                    ledger[idx].balance -= pkt.amount;
                    pkt.balance = ledger[idx].balance;
                    sprintf(pkt.message, "Withdraw Success.");
                } else sprintf(pkt.message, "Invalid ID or Balance.");
            }
            else if (pkt.action == ACTION_SYNC) {
                ledger[idx] = (UserAccount){.accountID = pkt.clientID, .balance = pkt.balance, .active = 1};
                if(pkt.clientID >= next_uid) next_uid = pkt.clientID + 1;
            }

            // Sync and Respond
            if (pkt.action != ACTION_SYNC && pkt.action != ACTION_BALANCE) {
                int original = pkt.action;
                pkt.action = ACTION_SYNC;
                pkt.vclock = local_vc;
                for(int i=0; i<CLUSTER_SIZE; i++) {
                    if(i == myIdx) continue;
                    struct sockaddr_in p = {.sin_family = AF_INET, .sin_port = htons(CLUSTER[i].port), .sin_addr.s_addr = inet_addr(CLUSTER[i].ip)};
                    sendto(s, (char *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&p, sizeof(p));
                }
                pkt.action = original;
            }
            
            if (pkt.action != ACTION_SYNC) {
                pkt.balance = ledger[idx].balance;
                sendto(s, (char *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&clientAddr, addrLen);
            }
            printf("VC: [%d,%d,%d,%d] | Last Action: %d\n", local_vc.v[0], local_vc.v[1], local_vc.v[2], local_vc.v[3], pkt.action);
        }
    }
    return 0;
}