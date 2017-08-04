
import socket, io

class TcpStream(io.BufferedIOBase):
    def __init__(self, sock = None):
        if not sock is None:
            assert isinstance(sock, socket.socket)
            self._sock = sock
        self._sock = socket.socket()

    def read(self, size = -1):
        buf = bytearray()
        sock = self._sock
        # import pdb; pdb.set_trace()  # XXX BREAKPOINT
        try:
            if size < 0:
                while True:
                    buf += sock.recv(2048)
            else:
                rest = size
                while rest:
                    data = sock.recv(rest)
                    buf += data
                    rest -= len(data)
        finally:
            return bytes(buf)

    def write(self, b):
        return len(b) if self._sock.sendall(b) is None else 0

    def flush(self):
        pass

    def connect(self, addr, port):
        return self._sock.connect((addr, port))

    def close(self):
        return self._sock.close()

def connect(host, port):
    sock = socket.socket()
    sock.connect((host, port))
    return TcpStream(sock)
