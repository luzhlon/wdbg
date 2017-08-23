
import sys
import argparse
from . import wdbg
from ptpython import repl

wd = None

def parse_args():
    parser = argparse.ArgumentParser(
            prog = 'pywdbg',
            description = "PYWDBG",
            epilog = 'v0.1')
    parser.add_argument('path',
            metavar = 'PATH', nargs = '?',
            help = 'Create a process and debug it')
    parser.add_argument('-a', '--attach',
            metavar = 'PID|NAME',
            help = 'attach to PID or NAME')
    parser.add_argument('-k', '--kernel',
            metavar = 'OPTION',
            help = 'attach a kernel target')
    parser.add_argument('-g',
            action = 'store_true',
            help = 'ignore the initial breakpoint')
    parser.add_argument('-G',
            action = 'store_true',
            help = 'ignore the final breakpoint')
    parser.add_argument('--version',
            action='version', version='pywdbg 0.1')
    parser.add_argument('-x',
            metavar = 'SCRIPT',
            help = 'startup with a python script')
    return parser.parse_args()

def run(args):
    global wd

    if args.path:
        assert wd.create(args.path) == 0
        wd.waitevent()
    elif args.attach:
        try:
            pid = int(args.attach)
            if wd.attach(pid):
                print('Attach failure', pid)
        except ValueError:
            if wd.attach(args.attach):
                print('Attach failure', args.attach)
        wd.waitevent()
    elif args.kernel:
        assert wd.attachkernel(args.kernel) == 0
        wd.waitevent()

    if args.x:
        with open(args.x) as f:
            exec(f.read(), {'wd': wd})

    # waiting for initial breakpoint
    print('---------------------------------------------------------')
    repl.embed(globals(), locals())

def main64():
    global wd

    wd = wdbg.startup('x64')
    run(parse_args())

def main():
    global wd

    wd = wdbg.startup('x86')
    run(parse_args())
