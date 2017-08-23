
## 前言

* 大多数函数失败返回nil(None)，对于获取数据的函数，成功返回0；其它的大多数函数成功返回0

## 数据访问

### 读写内存

* read(pos:int, size:int) -> bytes | nil
  - 读取(被调试进程的)内存
  - 成功返回存放所读数据的字节数组
* write(pos:int, data:bytes) -> byteswritten | nil
  - 写入(被调试进程的)内存
  - 成功返回写入的字节数
* readphy(pos:int, size:int) -> bytes | nil
  - 读取目标的**物理内存**
  - 成功返回存放所读数据的字节数组
* writephy(pos:int, data:bytes) -> byteswritten | nil
  - 写入目标的**物理内存**
  - 成功返回写入的字节数
* readstr(pos:int) -> str
  - 从pos位置读取字符串(ASCII编码)
* readustr(pos:int) -> ustr
  - 从pos位置读取UNICODE字符串(ASCII编码)
* readptr(pos:int, count = 1) -> (pointer ...)
  - 从pos位置开始读取count个指针
  - 若count == 1，返回所读的指针的值；`> 1`则返回这些指针的值构成的列表
* search(pos:int, len:int, pattern:bytes)
  - 从pos位置开始搜索二进制数据pattern，最多搜索len个字节
  - 成功返回pattern所在的地址

### 读写寄存器

* getreg(name:str | index:int) -> (value, type)
  - 获取寄存器的值，参数可以是寄存器的名字(字符串)，也可以是寄存器的索引(整数)
  - 成功返回寄存器的值和类型
  - 参考DEBUG_VALUE_XXX
* setreg(name:str | index:int, value:int, type = DEBUG_VALUE_INT64)
  - 设置寄存器的值，name参数可以是寄存器的名字(字符串)，也可以是寄存器的索引(整数)，value为要设置的值
  - 成功返回0
* getregs(name:str | index:int, ...) -> (value ...)
  - 获取多个寄存器的值
  - 成功返回多个值组成的列表
* iptr() -> current instruction position
  - 获取当前指令的地址

## 目标信息

* is64()
  - 判断目标是否为64位程序
* psid(id = nil)
  - 获取或设置当前进程的ID(这里的ID指的是调试引擎里的ID，不是系统里的进程ID)
* thid(id = nil)
  - 获取或设置当前线程的ID(这里的ID指的是调试引擎里的ID，不是系统里的线程ID)
* getpeb()
  - 获取PEB在目标内存中的位置
* getteb()
  - 获取TEB在目标内存中的位置
* exename()
  - 获取目标可执行文件的名字
* retpos()
  - 获取当前函数的返回指令所在的地址

## 控制目标

* create(path:str, flags = CREATE_NEW_CONSOLE | DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP | DEBUG_PROCESS) -> result
  - 创建进程，成功返回0
* attach(path:str | pid:int) -> success
  - 附加进程，第1个参数若为字符串，则调用CreateProcessAndAttach，第2个参数和第3个参数分别为创建进程的Flags和附加进程的Flags，若为整数，则调用AttachProcess，第2个参数为附加进程的Flags
  - 成功返回0
* attachkernel(options:str)
  - 附加到内核目标，参数options和windbg的-k后面的参数一样，一般为`com:port=\\.\pipe\com_1,baud=115200,pipe,resets=0`
  - 成功返回0
* terminate()
  - 终结当前(调试的)进程
  - 成功返回0
* abandon()
  - 遗弃当前(调试的)进程
  - 成功返回0
* exitcode()
  - 获取进程的返回值
* status(status = nil) -> status
  - 获取或设置调试器状态，参考GetExecutionStatus和SetExecutionStatus
* waitevent(timeout:int = INFINITE) -> result
  - 等待事件，参考WaitForEvent
* interrupt(flags = DEBUG_INTERRUPT_ACTIVE) -> success
  - 中断调试器等待事件，参考SetInterrupt

## 模块和符号

* name2pos(name:str)
  - 获取符号名的地址，比如`name2pos('kernel32!CreateFileA')`可以获取CreateFile函数的地址
* symbolpath(path = nil)
  - 获取或设置符号搜索路径
  - path如果是字符串，则设置符号搜索路径为path，成功返回0
  - path不是字符串时，成功会返回获取到的符号搜索路径
* modnum()
  - 获取模块数目
  - 返回[已加载的模块数, 未加载的模块数]
* getmod(name:str | index:int)
  - 获取模块基址，第1个参数可以为模块名name，也可以为模块索引index
  - 通常可执行文件模块的索引为0
* ptr2mod(pointer:int)
  - 通过指针获取模块基址
* typeid
* symboltype
* fieldoffset

## 断点管理

* addbp(offset:int|str) -> breakpoint:int
  - 在offset处添加断点，成功返回断点的ID
* addba(offset:int, size = 1, type = DEBUG_BREAK_READ) -> breakpoint:int
  - 在offset处添加数据断点(硬件断点)，成功返回断点的ID
* rmbp(bpid)
  - 删除断点，bpid为addbp返回的断点id值
* enablebp(id:int, enable = true)
  - 启用/禁用断点
* bpcmd(id:int, cmd = nil)
  - 获取/设置断点触发时要执行的命令
* bpthread(id:int, tid = nil)
  - 获取/设置断点匹配的线程，tid为目标线程在调试器里的ID

## DBGENG相关

* addopts(options:int) -> success
  - 添加选项调试器选项，参考AddOptions
* prompt() -> prompt
  - 获取调试器的提示文本，相当于WinDbg输入框左边的文字
* disasm(offset, count = 1) -> asm_text
  - 反汇编offset处的代码。若count > 1，返回由count条汇编代码组成的列表。参考Disassemble
* exec(command:str) -> debug_status | nil
  - 执行(WinDbg)命令，
* eval(expr:str) -> value
  - 评估表达式expr
* asmopts([options]) -> options:int | success:bool | nil
  - 设置或获取汇编(反汇编)选项，参考SetAssemblyOptions
* setoutput(handler:str)
  - 设置调试器输出数据时回调的RPC函数，handler接受两个参数：当前的RPC会话、GBK编码的输出数据

## 事件处理

> 有待完善

* setevent(type:int, callback)
  - 向调试器后台注册事件回调函数，type为事件类型(参考DEBUG_EVENT_XXX)，name一般为一个字符串，指示处理此事件的RPC回调函数名称

关于不同的事件回调函数所传递的参数，可以参考IDebugEventCallbacks接口，大多数都是按照此接口的参数顺序提供给前台，需要特别指出的是

* 异常处理函数(ExceptionAddress, ExceptionCode, ExceptionFlags, ExceptionInformation, ExceptionRecord, FirstChance)
  - ExceptionAddress 异常发生的位置
  - ExceptionCode 异常错误码
  - ExceptionFlags 异常错误标志
  - ExceptionInformation 异常信息
  - ExceptionRecord 异常记录
  - FirstChance 是否为第一次处理异常
  - 此接口待完善

## 其它

* peinfo([module,] attribute ...)
  - 获取module的PE文件属性，可指定多个
  - 若module为指定，则获取索引为0的module的信息
  - 目前可获取的属性有'EntryPoint' 'ImageSize' 'Machine' 'Subsystem'
  - 返回按attribute顺序排列的属性值列表
