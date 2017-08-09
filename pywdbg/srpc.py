
import umsgpack as msgpack
from tstream import TcpStream
from threading import RLock

MSG_INVALID = 0
MSG_CALL    = 1
MSG_NOTIFY  = 2
MSG_RETURN  = 3
MSG_EXCEPT  = 4

ERR_NO_FUNC = 0

class Handler:
    pass

class Session:
    def __init__(self, io):
        self._io = io
        self._mutex = RLock()
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

    def _return(self, v):
        self._mutex.acquire()
        self._pack = [MSG_RETURN, v]
        b = self._send_pack()
        self._mutex.release()
        return b

    def _except(self, code):
        self._mutex.acquire()
        self._pack = [MSG_EXCEPT, code]
        b = self._send_pack()
        self._mutex.release()
        return b

    # Return a value
    def run(self):
        self.onopen(self)
        while self._recv_pack():
            self._handle_invoke()
        self.onclose(self)

    def _recv_pack(self):
        try:
            self._pack = msgpack.unpack(self._io)
            return True
        except msgpack.UnpackException as e:
            print("Unpack failure", e)
            return False
        except Exception as e:
            print("Receive package failure", e)
            return False

    def _send_pack(self):
        try:
            self._pack = msgpack.pack(self._pack, self._io)
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
            if self._type() == MSG_RETURN:
                pack = self._pack
                return pack[1:] if len(pack) > 2 else pack[1]
            elif self._type() == MSG_EXCEPT:
                raise Exception('No such function: ' + self._lastfid)
            else:
                self._handle_invoke()   # handle invoke

    # Handle a invoke
    def _handle_invoke(self):
        t = self._type()
        assert t == MSG_CALL or t == MSG_NOTIFY
        pack = self._pack
        funid = pack[1]
        ret = None
        try:
            fun = self._getfunc(funid)
            if fun:
                ret = fun(self, *pack[2:])
            elif self._type() == MSG_CALL:
                return self._except(ERR_NO_FUNC)
        except Exception as e:
            print('Throwed a exception during:', funid, e)
            ret = None
        except TypeError as e:
            print('Args\'s number not matched:', funid, e)
        finally:
            if t == MSG_CALL:
                self._return(ret)

    # Get function in handler
    def _getfunc(self, fid):
        try:
            return getattr(self.handler, fid)
        except AttributeError:
            return None


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
