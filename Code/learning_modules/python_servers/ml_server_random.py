"""
/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */
"""

"""
ml_server_random.py - baseline server: selects a random arm each step.

Usage:
    python3 Code/learning_modules/python_servers/ml_server_random.py [socket_path]

The number of arms is read from features[2] sent by the simulation, so no
command-line argument is needed. Handles any number of agents automatically.
Exits when all clients send the shutdown sentinel (n_feat=0).

Wire protocol (matches ExternalModelClient):
    recv: uint32_t n_feat=3, then [arm_ix, reward, num_arms] as float32
    send: 1 * float32  (randomly chosen arm in [0, num_arms))
"""

import socket
import select
import struct
import sys
import os
import random

SOCKET_PATH = sys.argv[1] if len(sys.argv) > 1 else "/tmp/komondor_ml.sock"

if os.path.exists(SOCKET_PATH):
    os.remove(SOCKET_PATH)

srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
srv.bind(SOCKET_PATH)
srv.listen(16)
print(f"[ml_server_random] Listening on {SOCKET_PATH} ...")

connections = []
any_connected = False

try:
    while True:
        watch = [srv] + connections
        readable, _, _ = select.select(watch, [], [])
        for s in readable:
            if s is srv:
                conn, _ = srv.accept()
                connections.append(conn)
                any_connected = True
                print(f"[ml_server_random] Client #{len(connections)} connected.")
                continue
            header = s.recv(4)
            if not header:
                connections.remove(s)
                s.close()
                continue
            n = struct.unpack("I", header)[0]
            if n == 0:  # shutdown sentinel
                connections.remove(s)
                s.close()
                continue
            raw = b""
            while len(raw) < n * 4:
                chunk = s.recv(n * 4 - len(raw))
                if not chunk:
                    raise ConnectionError("connection closed mid-receive")
                raw += chunk
            features = struct.unpack(f"{n}f", raw)
            num_arms = int(round(features[2])) if n >= 3 else 2
            next_arm = float(random.randint(0, num_arms - 1))
            print(f"  recv: arm_ix={features[0]:.0f}  reward={features[1]:.4f}  num_arms={num_arms}  next_arm={next_arm:.0f}")
            s.sendall(struct.pack("f", next_arm))
        if any_connected and not connections:
            print("[ml_server_random] All clients disconnected. Exiting.")
            break
finally:
    for conn in connections:
        conn.close()
    srv.close()
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)
