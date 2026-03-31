#include <stdio.h>
#include <string.h>
#include "protocol.h"

int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server;
    MobileMoneyPacket pkt;
    DWORD timeout = 1500;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    while (1) {
        memset(&pkt, 0, sizeof(pkt));
        printf("\n--- Mobile Money Menu ---\n1. Register (New Account)\n2. Deposit\n3. Withdraw\n4. Check Balance\n5. Exit\nChoice: ");
        int choice; scanf("%d", &choice);
        if (choice == 5) break;

        if (choice == 1) {
            pkt.action = ACTION_REGISTER;
        } else {
            printf("Enter your Account ID: ");
            scanf("%d", &pkt.clientID);
            if (choice == 2) {
                pkt.action = ACTION_DEPOSIT;
                printf("Amount: "); scanf("%d", &pkt.amount);
            } else if (choice == 3) {
                pkt.action = ACTION_WITHDRAW;
                printf("Amount: "); scanf("%d", &pkt.amount);
            } else {
                pkt.action = ACTION_BALANCE;
            }
        }

        int success = 0;
        for (int i = 0; i < CLUSTER_SIZE; i++) {
            server.sin_family = AF_INET;
            server.sin_port = htons(CLUSTER[i].port);
            server.sin_addr.s_addr = inet_addr(CLUSTER[i].ip);
            sendto(s, (char *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&server, sizeof(server));

            int len = sizeof(server);
            if (recvfrom(s, (char *)&pkt, sizeof(pkt), 0, (struct sockaddr *)&server, &len) > 0) {
                printf("\n[Node %d Response] %s\n", i, pkt.message);
                if (pkt.action != ACTION_REGISTER) printf("Current Balance: %d\n", pkt.balance);
                success = 1; break;
            }
        }
        if(!success) printf("\n[ERROR] Cluster connection failed.\n");
    }
    closesocket(s); WSACleanup();
    return 0;
}