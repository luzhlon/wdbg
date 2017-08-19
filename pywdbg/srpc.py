
import umsgpack as msgpack
from .tstream import TcpStream
from threading import RLock

# Base type
MSG_INVOKE = 0x10
MSG_RETURN = 0x20
# Concrete type
MSG_CALL   = MSG_INVOKE | 0x00
MSG_NOTIFY = MSG_INVOKE | 0x01
MSG_CLOSE  = MSG_INVOKE | 0x02

MSG_NOFUNC = MSG_RETURN | 0x00
MSG_RETVAL = MSG_RETURN | 0x01

class Handler:
    pass

class Session:
    def __init__(self, io):
        self._io = io
        self._mutex = RLock()
        self.isclosed = False
        self.handler = Handler
        self.onopen = lambda session: None
        self.onclose = lambda session: None

    # Call a function
    def call(self, fid, *args):
        self.invoke(MSG_CALL, fid, args)
        return self._wait_return()

    def notify(self, fid, *args):
        self.invoke(MSG_NOTIFY, fid, args)

    def invoke(self, t, fid, args):
        self._mutex.acquire()
        self._pack = [t, fid, *args]
        b = self._send_pack()
        self._mutex.release()
        self._lastfid = fid
        return b

    def close(self):
        self._mutex.acquire()
        self._pack = [MSG_CLOSE]
        b = self._send_pack()
        self._mutex.release()
        self._io.close()
        self._closed = True
        return b

    def _return(self, v, t = MSG_RETVAL):
        self._mutex.acquire()
        self._pack = [t, v]
        b = self._send_pack()
        self._mutex.release()
        return b

    # Return a value
    def run(self):
        self.onopen(self)
        while self._recv_pack():
            self._handle_invoke()
            if self.isclosed:
                break
        self.onclose(self)

    def _recv_pack(self):
        if self.isclosed:
            return False
        try:
            self._pack = msgpack.unpack(self._io)
            return True
        except msgpack.UnpackException as e:
            return False
        except Exception as e:
            print("Receive package failure", e)
            return False

    def _send_pack(self):
        if self.isclosed:
            return False
        try:
            msgpack.pack(self._pack, self._io)
            self._io.flush()
            return True
        except Exception as e:
            # print("Send package failure", e)
            return False

    def _type(self):
        return self._pack[0]

    # Wait a value from subprocess
    def _wait_return(self):
        while self._recv_pack():
            t = self._type()
            if t & 0xF0 == MSG_INVOKE:
                self._handle_invoke()   # handle invoke
            elif t == MSG_RETVAL:
                pack = self._pack
                return pack[1:] if len(pack) > 2 else pack[1]
            elif t == MSG_NOFUNC:
                raise Exception('No such function: ' + self._lastfid)
            else:
                raise Exception('Unkown flag')

    # Handle a invoke
    def _handle_invoke(self):
        t = self._type()
        assert t & 0xF0 == MSG_INVOKE
        if t == MSG_CLOSE:
            self.isclosed = True
        else:
            pack = self._pack
            fid = pack[1]
            ret = None
            try:
                fun = vars(self.handler).get(fid)
                if fun:
                    ret = fun(self, *pack[2:])
                elif self._type() == MSG_CALL:
                    return self._return(None, MSG_NOFUNC)
            except Exception as e:
                print('Throwed a exception during:', fid, e)
                ret = None
            except TypeError as e:
                print('Args\'s number not matched:', fid, e)
            finally:
                if t == MSG_CALL:
                    self._return(ret)


class Client(Session):
    def __init__(self):
        Session.__init__(self, TcpStream())

    def connect(self, addr, port):
        return self._io.connect(addr, port)


class Server:
    def __init__(self, addr, port):
        import socket
        self._server = socket.socket()
        self._server.bind((addr, port))
        self._server.listen()
        self.handler = {}

    def accept(self):
        from tstream import TcpStream
        sock, addr = self._server.accept()
        s = Session(TcpStream(sock), self.handler)
        return s
