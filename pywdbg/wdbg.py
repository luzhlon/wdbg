
import srpc
from K import ERROR, ENGOPT, EVENT, STATUS
import time
from threading import Thread
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

# Main session's handler
class MainHandler:
    def output(ss, data):
        print(data.decode('gbk'), end = '')

    def breakpoint(ss, id, offset):
        global bp_callback
        print('[Breakpoint]\t%x occured, offset: %x' % (id, offset))
        ret = None
        if id in bp_callback:
            ret = bp_callback[id](id, offset)
        if ret is int:
            return ret
        return STATUS.BREAK

on = MainHandler

class WDbg:
    def __getattr__(self, name):
        mss = self._mss
        def func(*args):
            return mss.call(name, *args)
        return func

    def InitNormally(self):
        self.AddOptions(ENGOPT.INITIAL_BREAK | ENGOPT.FINAL_BREAK)
        # output callback
        if getattr(MainHandler, 'output'):
            self.RegisterOutput('output')
        # register events callbacks
        events = 0
        def reg(eid, fname):
            nonlocal events
            try:
                getattr(MainHandler, fname)
                events |= eid
                self.RegisterEvent(eid, fname)
            except AttributeError:
                pass
        reg(EVENT.BREAKPOINT, 'breakpoint')
        reg(EVENT.CREATE_PROCESS, 'createprocess')
        reg(EVENT.CREATE_THREAD, 'createthread')
        reg(EVENT.LOAD_MODULE, 'loadmodule')
        reg(EVENT.EXIT_PROCESS, 'exitprocess')
        self.SetEventCallbacks(events)

    def SetInterrupt(self):
        return self._ass.notify('SetInterrupt')

    def WaitEvent(self, *timeout):
        self._waiting = True
        def waitevent():
            self._status = self._mss.call('WaitForEvent', *timeout)
            self._waiting = False
        Thread(target=waitevent).start()
        while self._waiting:
            try:
                time.sleep(0.2)
            except KeyboardInterrupt:
                self.SetInterrupt()
        return ERROR.get(self._status, 'FAILURE')

    def cmdloop(self):
        try:
            while True:
                cmd = input('> ')
                b = self.Execute(cmd)
                if not b:
                    print('[Execute failure]', cmd)
                    break
                elif b != STATUS.BREAK:
                    self.WaitEvent()
        except KeyboardInterrupt:
            return

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
