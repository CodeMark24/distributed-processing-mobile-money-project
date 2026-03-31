# Mobile Money — Distributed UDP Cluster

A 4-node distributed mobile money system in C using UDP sockets and Vector Clocks. **Windows only** (Winsock2).

## Configuration (`protocol.h`)

Edit the `CLUSTER` array to set node IPs and ports:
```c
static const Peer CLUSTER[] = {
    {"127.0.0.1", 9001},  // Node 0
    {"127.0.0.1", 9002},  // Node 1
    {"127.0.0.1", 9003},  // Node 2
    {"127.0.0.1", 9004}   // Node 3
};
```
Change `MAX_USERS` (default `100`) to adjust ledger capacity. Client timeout is set via `DWORD timeout = 1500` (ms) in `client.c`.

## Build & Run

```bash
gcc server.c -o server.exe -lws2_32
gcc client.c -o client.exe -lws2_32

server.exe 0  # repeat for indices 1, 2, 3 in separate terminals
client.exe
```

## Vector Clocks

Each node maintains a 4-slot clock `int v[4]` — one slot per node. On every incoming packet, the node merges the remote clock (taking the max per slot) then increments its own slot. This tracks causal ordering of events across the cluster. After any write (deposit/withdraw/register), the handling node broadcasts an `ACTION_SYNC` to all peers carrying the updated clock and account state.

## Deploying on Separate Machines

Replace the loopback IPs in `protocol.h` with each machine's actual local or public IP:
```c
static const Peer CLUSTER[] = {
    {"192.168.1.10", 9001},  // Machine A — Node 0
    {"192.168.1.11", 9001},  // Machine B — Node 1
    {"192.168.1.12", 9001},  // Machine C — Node 2
    {"192.168.1.13", 9001}   // Machine D — Node 3
};
```
This **same updated `protocol.h`** must be used when compiling both `server.c` and `client.c` on every machine. Each machine then runs `server.exe` with its own index (e.g. Machine B runs `server.exe 1`). Ensure the chosen port is open in each machine's firewall, and that all nodes are reachable from the client machine.
