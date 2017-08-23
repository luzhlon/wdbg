
此篇介绍的是调试器前台进行事件处理的方法，有关使用具体的调试器前台处理事件，可以参考[pywdbg](https://github.com/luzhlon/wdbg/wiki/pywdbg)

调试器前台需要向后台注册某个调试器事件的回调函数(名字)，当发生相应的事件时，调试器会通过RPC调用前台相应的函数

比如前台通过setevent函数向后台注册了DEBUG_EVENT_CREATE_PROCESS的事件处理函数'createprocess'，那么当创建进程的事件发生时，后台便会通过RPC调用前台的createprocess函数，来处理此事件；前台的事件处理函数需要返回一个值，指示调试器引擎该怎么继续处理，对于创建进程的事件，一般返回DEBUG_STATUS_GO_HANDLED，对于断点事件一般返回DEBUG_STATUS_BREAK让调试器中断下来，给用户处理的机会

## API

* setevent(type:int, callback)
  - 向调试器后台注册事件回调函数，type为事件类型(参考DEBUG_EVENT_XXX)，name一般为一个字符串，指示处理此事件的RPC回调函数名称

---------------

关于不同的事件回调函数所传递的参数，可以参考IDebugEventCallbacks接口，大多数都是按照此接口的参数顺序提供给前台，需要特别指出的是

* 异常处理函数(ExceptionAddress, ExceptionCode, ExceptionFlags, ExceptionInformation, ExceptionRecord, FirstChance)
  - ExceptionAddress 异常发生的位置
  - ExceptionCode 异常错误码
  - ExceptionFlags 异常错误标志
  - ExceptionInformation 异常信息
  - ExceptionRecord 异常记录
  - FirstChance 是否为第一次处理异常
  - 此接口待完善
