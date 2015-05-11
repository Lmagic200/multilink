import socket
import struct
import json
import threading
import time
import sys

class Connection(object):
    def __init__(self, path='../build/app.sock'):
        self.sock = socket.socket(socket.AF_UNIX)
        self.sock.connect(path)
        self.file = self.sock.makefile('r+')

    def send(self, msg):
        msg = json.dumps(msg)
        self.sock.sendall(struct.pack('I', len(msg)) + msg)

    def provide_stream(self, num):
        self.send({'type': 'provide-stream', 'num': num})
        resp = self.file.read(3)
        if resp != 'ok\n':
            raise IOError('bad response to provide-stream')
        return self.file

    def make_multilink(self, num, stream_fd, free=False):
        self.send({'type': 'multilink',
                   'num': num, 'stream_fd': stream_fd,
                   'free': free})

    def add_link(self, num, stream_fd, name):
        self.send({'type': 'add-link', 'name': name,
                   'num': num, 'stream_fd': stream_fd})

    def limit_stream(self, stream_fd, buffsize, delay, mbps):
        self.send({'type': 'limit-stream', 'stream_fd': stream_fd,
                   'buffsize': buffsize, 'mbps': mbps,
                   'delay': delay})

class LengthPacketStream(object):
    def __init__(self, f):
        self.f = f

    def recv(self):
        length, = struct.unpack('I', self.f.read(4))
        return self.f.read(length)

    def send(self, data):
        self.f.write(struct.pack('I', len(data)))
        self.f.write(data)
        self.f.flush()

def pipe(label, a, b):
    next_msg = [0]
    count = [0]

    def do():
        while True:
            d = a._sock.recv(4096)
            if not d:
                break

            b.write(d)
            b.flush()

            count[0] += len(d)
            if time.time() > next_msg[0]:
                sys.stdout.write('%s receiving (%d bytes)\n' % (label, count[0]))
                next_msg[0] = time.time() + 0.1

    t = threading.Thread(target=do)
    t.daemon = True
    t.start()

def pipe2(label, a, b):
    pipe(label + ' a->b', a, b)
    pipe(label + ' b->a', b, a)

class HandlerBase(object):
    def provide_stream(self, stream, name='provide'):
        id = self.stream_counter
        stream1 = Connection(self.sock_path).provide_stream(id)

        self.stream_counter += 1

        pipe2(name, stream, stream1)

        return id
