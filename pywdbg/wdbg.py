
import srpc
from K import ERROR, ENGOPT, EVENT, STATUS
import time
from threading import Thread, Condition, Lock
# Breakpoint callbacks
bp_callback = {}
# WDbg object
dbg = None
# Custom command
class cmd:
    pass

# Auxiliary session's handler
class AuxHandler:
    def heartbeat(ss, n):
        ss.notify('heartbeat', n)

inp = input
# Main session's handler
class MainHandler:
    def input(ss, data):
        global inp
        return inp()

    def output(ss, data):
        print(data.decode('gbk'), end = '')

    def breakpoint(ss, id, offset):
        global bp_callback
        # print('[Breakpoint]\t%x occured, offset: %x' % (id, offset))
        ret = None
        if id in bp_callback:
            ret = bp_callback[id](id, offset)
        if ret is int:
            return ret
        return STATUS.BREAK

on = MainHandler

backfset = set(['stepinto', 'stepover', 'run', 'waitevent', 'exec'])

class WDbg:
    def __init__(self):
        self._taskcond = Condition()    # condition for waitting task
        self._compcond = Condition()    # condition for waitting to complete task
        self._compcond.acquire()
        def backthread():
            self._taskcond.acquire()
            c = self._compcond
            while True:
                self._taskcond.wait()
                self._result = self._fun(*self._args)
                c.acquire(); c.notify(); c.release()
        self._back = Thread(target = backthread)
        self._back.start()

    def __getattr__(self, name):
        global backfset

        mss = self._mss
        f = None
        if name in backfset:
            def fun(*args):
                self._args = (name, *args)
                self._fun = mss.call
                return self._backrun()
            f = fun
        else:
            def func(*args):
                return mss.call(name, *args)
            f = func
        setattr(self, name, f)
        return f

    def _backrun(self):
        c = self._taskcond
        c.acquire(); c.notify(); c.release()
        while True:
            try:
                if self._compcond.wait(0.2):
                    break
            except KeyboardInterrupt:
                self.interrupt()
        return self._result

    def init(self):
        self.addopts(ENGOPT.INITIAL_BREAK | ENGOPT.FINAL_BREAK)
        # output callback
        if getattr(MainHandler, 'output'):
            self.setoutput('output')
        # register events callbacks
        events = 0
        def reg(eid, fname):
            nonlocal events
            try:
                getattr(MainHandler, fname)
                events |= eid
                self.setevent(eid, fname)
            except AttributeError:
                pass
        reg(EVENT.BREAKPOINT, 'breakpoint')
        reg(EVENT.CREATE_PROCESS, 'createprocess')
        reg(EVENT.CREATE_THREAD, 'createthread')
        reg(EVENT.LOAD_MODULE, 'loadmodule')
        reg(EVENT.EXIT_PROCESS, 'exitprocess')

    def interrupt(self):
        return self._ass.notify('interrupt')

    def cmdloop(self):
        while True:
            try:
                cmd = input('> ')
                b = self.exec(cmd)
                if b:
                    print('[Execute failure]', cmd)
            except EOFError:
                break
            except KeyboardInterrupt:
                break

def connect(addr, port):
    global dbg
    if dbg: return None
    dbg = WDbg()
    dbg._mss = srpc.Client()
    dbg._mss.handler = MainHandler
    dbg._mss.connect(addr, port)

    dbg._ass = srpc.Client()
    dbg._ass.handler = AuxHandler
    dbg._ass.connect(addr, port)
    Thread(target = dbg._ass.run).start()

    return dbg
