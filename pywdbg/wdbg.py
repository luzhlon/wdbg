
import time, os
from . import srpc
from .K import ERROR, ENGOPT, EVENT, STATUS
from threading import Thread, Event

# Auxiliary session's handler
class AuxHandler:
    def heartbeat(ss, n):
        ss.notify('heartbeat', n)

INPUT = input
# Main session's handler
class MainHandler:
    def input(ss, data):
        global INPUT
        return INPUT()

    def output(ss, data):
        print(data.decode('gbk'), end = '')

backfset = set(['waitevent'])

class WDbg:
    def __init__(self):
        self._taskcond = Event()    # condition for waitting task
        self._compcond = Event()    # condition for waitting to complete task
        def backthread():
            while True:
                self._taskcond.clear()
                self._taskcond.wait()
                self._result = self._fun(*self._args)
                self._compcond.set()
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
        self._compcond.clear()
        self._taskcond.set()
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

    def interrupt(self):
        self._ass.notify('interrupt')
        return self

    def stepinto(self):
        self.status(STATUS.STEP_INTO)
        return self

    def stepover(self):
        self.status(STATUS.STEP_OVER)
        return self

    def go(self):
        self.status(STATUS.GO)
        return self

    def cmdloop(self):
        while True:
            try:
                cmd = input(self.prompt() + ' > ')
                b = self.exec(cmd)
                if b:
                    print('[Execute failure]', cmd)
                if self.status() != STATUS.BREAK:
                    self.waitevent()
                    self.putstate()
            except EOFError:
                break
            except KeyboardInterrupt:
                break

    def event(self, name, func = None):
        try:
            if func:
                origin = self.event(name)
                code = getattr(EVENT, name)
                setattr(MainHandler, name, func)
                self.setevent(code, name)
            else:
                return MainHandler.__dict__.get(name)
        except AttributeError:
            raise Exception('No such event: ' + name)

PATH = __file__

def search_wdbg(arch):
    '''search the path of the wdbg.exe'''

    global PATH
    assert arch == 'x86' or arch == 'x64'

    curpath = os.path.abspath(PATH)
    upath = os.path.split(os.path.dirname(curpath))[0]
    path = [upath + x + arch + '/wdbg.exe' for x in ['/build/debug/', '/bin/']]

    for p in path:
        if os.path.exists(p):
            return p

def start_local(arch = 'x86'):
    '''start the wdbg subprocess, and return its listened port'''

    path = search_wdbg(arch)
    if not path:
        return None

    from subprocess import STARTUPINFO, Popen, SW_HIDE
    from subprocess import CREATE_NEW_CONSOLE, STARTF_USESHOWWINDOW, PIPE

    si = STARTUPINFO()
    si.dwFlags = STARTF_USESHOWWINDOW
    si.wShowWindow = SW_HIDE
    wdbg = Popen([path, '-D'], bufsize = 0, startupinfo = si,
            creationflags = CREATE_NEW_CONSOLE, stdin = PIPE, stdout = PIPE)
    import re
    line = wdbg.stdout.readline().decode()
    port = re.search(r'\d+', line)
    if port:
        return int(port.group(0))

# WDbg object
_wdbg = None

def connect(addr = None, port = None):
    global _wdbg

    if not _wdbg:
        _wdbg = WDbg()
        _wdbg._mss = srpc.Client()
        _wdbg._mss.handler = MainHandler
        _wdbg._mss.connect(addr, port)

        _wdbg._ass = srpc.Client()
        _wdbg._ass.handler = AuxHandler
        _wdbg._ass.connect(addr, port)
        Thread(target = _wdbg._ass.run).start()

        _wdbg.init()
        return _wdbg

def startup(arch = 'x86'):
    '''startup the wdbg subprocess and connect it, return the WDbg object'''

    port = start_local(arch)
    if port:
        return connect('127.0.0.1', port)

