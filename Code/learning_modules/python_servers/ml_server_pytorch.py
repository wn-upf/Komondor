"""
/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 */
"""

"""
ml_server_pytorch.py - PyTorch inference / online-RL server.

Usage:
    python3 Code/learning_modules/python_servers/ml_server_pytorch.py [socket_path] [model_path]

Wire protocol (matches ExternalModelClient):
    recv: uint32_t n_feat=3, then [arm_ix, reward, num_arms] as float32
    send: 1 * float32  (next arm index)

Offline inference: set MODEL_PATH to a TorchScript model saved with
    torch.jit.save(torch.jit.script(model), "model.pt")
The model must accept a 1-D float tensor of length n_feat and return a scalar.
"""

import socket
import select
import struct
import sys
import os

SOCKET_PATH = sys.argv[1] if len(sys.argv) > 1 else "/tmp/komondor_ml.sock"
MODEL_PATH  = sys.argv[2] if len(sys.argv) > 2 else "model.pt"

try:
    import torch
    MODEL_AVAILABLE = os.path.exists(MODEL_PATH)
    model = torch.jit.load(MODEL_PATH) if MODEL_AVAILABLE else None
    if model is not None:
        model.eval()
        print(f"[ml_server_pytorch] Loaded model from {MODEL_PATH}")
    else:
        print(f"[ml_server_pytorch] WARNING: {MODEL_PATH} not found -- falling back to arm_ix echo")
except ImportError:
    torch = None
    model = None
    print("[ml_server_pytorch] WARNING: torch not installed -- falling back to arm_ix echo")

if os.path.exists(SOCKET_PATH):
    os.remove(SOCKET_PATH)

srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
srv.bind(SOCKET_PATH)
srv.listen(16)
print(f"[ml_server_pytorch] Listening on {SOCKET_PATH} ...")

def recv_exact(s, nbytes):
    buf = b""
    while len(buf) < nbytes:
        chunk = s.recv(nbytes - len(buf))
        if not chunk:
            raise ConnectionError("connection closed mid-receive")
        buf += chunk
    return buf

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
                print(f"[ml_server_pytorch] Client #{len(connections)} connected.")
                continue
            header = s.recv(4)
            if not header:
                connections.remove(s)
                s.close()
                continue
            n = struct.unpack("I", header)[0]
            if n == 0:
                connections.remove(s)
                s.close()
                continue
            raw = recv_exact(s, n * 4)
            features = list(struct.unpack(f"{n}f", raw))
            if model is not None and torch is not None:
                x = torch.tensor(features, dtype=torch.float32)
                with torch.no_grad():
                    y = model(x)
                next_arm = float(y.item())
            else:
                next_arm = features[0]
            print(f"  recv: arm_ix={features[0]:.0f}  reward={features[1]:.4f}  next_arm={next_arm:.2f}")
            s.sendall(struct.pack("f", next_arm))
        if any_connected and not connections:
            print("[ml_server_pytorch] All clients disconnected. Exiting.")
            break
finally:
    for conn in connections:
        conn.close()
    srv.close()
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)
