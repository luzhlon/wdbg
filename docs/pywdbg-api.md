
## WDbg对象

WDbg对象代表了wdbg所提供的接口，所有的RPC调用都通过WDbg对象来进行，看起来就像本地调用一样

WDbg的大多数方法都是对RPC-API简单的一层封装，不过有些RPC-API对用户并不友好，下面要介绍的是pwdbg特有的接口

* WDbg.cmdloop()
  - 启动**命令循环**，和windbg的命令一致
  - 此函数会循环要求用户输入一行命令，并发送给wdbg后台执行，执行结果会通过wdbg内部的函数输出到控制台上
  - 按CTRL-C可以终止命令行循环

* WDbg.stepover() ------ 单步跳过
* WDbg.stepinto() ------ 单步进入
* WDbg.go() ------------ 继续运行

### 断点处理

断点处理原本是事件处理的一部分，但是pywdbg在其之上封装了一层，便于用户使用

调用WDbg.addbp(offset)可获取到一个断点(Breakpoint)对象

属性：
* callback --- 用户可以设置断点对象的callback属性，callback为一个callable对象，当断点触发时被调用，并传入断点对象作为参数
* id --------- 此断点的ID
* type ------- 断点类型，'code'或者'data'
* cmd -------- 断点触发时执行的命令(windbg命令)
* thread ----- 断点匹配的线程

方法：
* enable(b) --- 启用/禁用此断点
* remove() ---- 删除此断点

### 事件处理

* WDbg.event(name, func = None)
  - 若传递func参数，表示**注册事件处理函数**；若不传递，表示**获取事件处理函数**
  - 在注册事件处理函数之前需要先获取事件处理函数，可以在自己的事件处理函数中调用之前获取到的事件处理函数。当然，这不是必须的
  - name可接受的事件类型(字符串)，如果name参数不在这些里面，则会抛出异常
    * CREATE_PROCESS
    * CREATE_THREAD
    * EXIT_PROCESS
    * EXIT_THREAD
    * BREAKPOINT
    * EXCEPTION
    * LOAD_MODULE
    * UNLOAD_MODULE
    * SYSTEM_ERROR
  - 每个事件处理函数所接收的参数是不同的，不过第一个参数总是当前的RPC会话对象。具体的可以参考IDebugEventCallbacks，如果你不确定，可以使用变长参数

## wdbg模块

pywdbg是wdbg调试器的前台，使用前需要先启动wdbg后台程序，根据输出的端口号调用wdbg.connect(addr, port)连接到后台，连接成功后会返回一个wdbg.WDbg对象

不过对于一般的本地调试，不需要手动去连接。使用wdbg.startup(arch = 'x86')函数，pywdbg可以自动搜寻wdbg的路径，然后wdbg作为子进程被启动，pywdbg调用connect连接到wdbg后台，返回结果和connect是一样的

* wdbg.connect(addr, port)
  - 连接到wdbg后台，addr为wdbg后台所在的IP地址，port为wdbg监听的端口号
  - 成功返回WDbg对象，否则返回None
* wdbg.startup(arch = 'x86')
  - 启动本地的wdbg并连接，arch为要启动的版本(32位的还是64位的)，只能为'x86'或者'x64'
  - 返回值和connect一样，成功返回WDbg对象，否则返回None

