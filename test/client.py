import socket
import io
import time

s = socket.socket(socket.AF_INET)
s.connect(('localhost', 49200))
with io.open('./req.txt', 'r', newline="") as f:
    a = f.read()
    print(a.encode('utf-8'))
    s.send(a.encode('utf-8'))

time.sleep(0.1)

print(s.recv(10000).decode('utf-8'))
