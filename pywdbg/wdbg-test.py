import wdbg
from threading import Thread

wd = wdbg.connect('127.0.0.1', 5100)

if wd:
    wd.InitNormally()
    print(wd.CreateProcess('D:\\tool\\Hash.exe'))

    wd.WaitEvent()       # Initial breakpoint

    offset = wd.GetOffsetByName('kernel32!CreateFileA')
    print("Get the CreateFile offset:", offset)
    bp = wd.AddBreakpoint(offset)
    print("bp CreateFile:", bp)

    wd.WaitEvent()
