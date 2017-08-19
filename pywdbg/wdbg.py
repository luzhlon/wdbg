
import time, os, re

from . import srpc
from .K import ERROR, ENGOPT, EVENT, STATUS, END
from threading import Thread, Event

# WDbg object
_wdbg = None

class Breakpoint:
    '''
    Breakpoint represents a breakpoint

    Attribute:
        callback:callable         will be invoked when the breakpoint is occured
    '''
    _dict = {}

    def __init__(self, id):
        self.id = id
        Breakpoint._dict[id] = self

    def _get(id):
        return Breakpoint._dict.get(id)

    def enable(self, b = True):
        '''enable or disable this breakpoint'''
        global _wdbg
        return _wdbg.enablebp(self.id, b)

    def remove(self):
        '''remove this breakpoint'''
        global _wdbg
        del Breakpoint._dict[self.id]
        return _wdbg.rmbp(self.id)

# Auxiliary session's handler
class AuxHandler:
    def heartbeat(ss, n):
        ss.notify('heartbeat', n)

# Main session's handler
class MainHandler:
    def tellinfo(ss, data):
        global _wdbg

        _wdbg._arch = data

    def Input(ss, data):
        return input()

    def output(ss, data):
        print(data.decode('gbk'), end = '')

    def BREAKPOINT(ss, id):
        bp = Breakpoint._get(id)
        if bp and 'callback' in vars(bp):
            try:
                return bp.callback(bp)
            except Exception as e:
                print(e)

class WDbg:
    def __init__(self):
        self._taskev = Event()    # event for waitting task
        self._compev = Event()    # event for waitting to complete task

        def backthread():
            while True:
                self._taskev.clear()
                self._taskev.wait()
                self._result = self._back_fun(*self._back_args)
                self._compev.set()
        self._back = Thread(target = backthread)
        self._back.start()

    def __getattr__(self, name):
        mss = self._mss

        def func(*args):
            return mss.call(name, *args)
        setattr(self, name, func)
        return func

    def _back_run(self, fun, args = ()):
        '''run some procedure in background'''
        self._back_fun = fun
        self._back_args = args
        self._compev.clear()
        # run in background
        self._taskev.set()

    # wait the backrun return
    def _wait_back(self, timeout = None):
        if timeout:
            return self._compev.wait(timeout)
        while True:
            try:
                if self._compev.wait(0.2):
                    break
            except KeyboardInterrupt:
                self.interrupt()
        return self._result

    def _load_config(self):
        f = os.path.expanduser('~/_wdbgconf.py')
        try:
            with open(f) as conf:
                exec(conf.read(), {'wdbg': self})
            print('[configure] loaded success!')
        except OSError:
            pass
        except Exception as e:
            print('[configure] loaded failure:', e)

    def init(self):
        # output callback
        self.setoutput('output')
        self.event('BREAKPOINT', MainHandler.BREAKPOINT)
        # load configure file
        self._load_config()

    def waitevent(self, timeout = None):
        self._back_run(self._mss.call, ('waitevent', timeout))
        return self._wait_back()

    def interrupt(self):
        self._ass.notify('interrupt')
        return self

    def addbp(self, pos, callback = None, *args):
        '''add a breakpoint'''
        id = self._mss.call('addbp', pos, *args)
        if type(id) is int:
            bp = Breakpoint(id)
            bp.pos = pos
            if callback:
                bp.callback = callback
            return bp

    def stepinto(self):
        self.status(STATUS.STEP_INTO)
        return self

    def stepover(self):
        self.status(STATUS.STEP_OVER)
        return self

    def go(self):
        '''continue to run'''
        self.status(STATUS.GO)
        return self

    def end_detach(self):
        '''end the debug session and detach target'''
        return self.end(END.ACTIVE_DETACH)

    def end_term(self):
        '''end the debug session and terminate target'''
        return self.end(END.ACTIVE_TERMINATE)

    def cmdloop(self):
        '''startup a command loop'''

        exitcmd = self._exitcmd if hasattr(self, '_exitcmd') else '..'
        while True:
            try:
                cmd = input(self.prompt() + ' > ')
                if cmd == exitcmd:
                    break
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
        '''register a event handler,'''
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

def start_local(arch = 'x86', path = None):
    '''start the wdbg subprocess, and return its listened port'''

    path = path or search_wdbg(arch)
    if not path:
        return None

    from subprocess import STARTUPINFO, SW_HIDE, PIPE
    from subprocess import CREATE_NEW_CONSOLE, STARTF_USESHOWWINDOW
    from subprocess import SubprocessError, Popen

    si = STARTUPINFO()
    si.dwFlags = STARTF_USESHOWWINDOW
    si.wShowWindow = SW_HIDE
    try:
        return Popen([path, '-D', '-a', '127.0.0.1'],
                  bufsize = 0, startupinfo = si, stdin = PIPE,
                  creationflags = CREATE_NEW_CONSOLE, stdout = PIPE)
    except SubprocessError:
        return None

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

def read_port(wdbg):
    line = wdbg.stdout.readline().decode()
    port = re.search(r'\d+', line)
    if port:
        return int(port.group(0))

def startup(arch = 'x86', path = None):
    '''startup the wdbg subprocess and connect it, return the WDbg object'''

    wdbg = start_local(arch, path)
    if not wdbg:
        return
    port = read_port(wdbg)
    if port:
        wd = connect('127.0.0.1', port)
        return wd
