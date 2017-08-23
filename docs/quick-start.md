
## 简介

wdbg是基于微软发布的调试引擎`dbgeng`(WinDbg的调试引擎)开发的调试器，wdbg并不直接给用户提供UI，而是以`RPC`的形式提供API接口

wdbg只是一个调试器后台，并不能直接与用户交互，所以本项目提供了一个python版本的前台 --- pywdbg，以下介绍皆基于pywdbg来说明

## 安装

使用pywdbg之前，先确认你的电脑已经安装了如下组件：
* vs2017再发行包
* python 3.2+，pip

从 https://github.com/luzhlon/wdbg/releases 获取最新版本，将压缩包里的目录出来并进入此目录，在此目录下打开命令行执行`python setup.py install`安装pywdbg

## 使用

如果安装过程顺利的话，你将得到**pywdbg**和**pywdbg64**两个命令。这两个命令的用法是一样的，不同的是启用的调试器架构不同，分别对应32位和64位的调试器，下面皆以pywdbg为例进行介绍

### 调试会话

开始一个调试会话有两种基本途径：创建进程或附加一个目标(附加到进程或者内核调试)

* 创建进程：`pywdbg XXX.exe`，XXX.exe是所要调试的程序路径
* 附加进程：`pywdbg -a PID`，PID是所要附加的进程的进程ID
* 附加内核，双机调试：`pywdbg -k OPTIONS`，OPTIONS一般为`com:port=\\.\pipe\com_1,baud=115200,pipe,resets=0`

### 执行自定义脚本

使用pywdbg的-x参数指定所要执行的python脚本，如果指定了调试目标，这个脚本会在连接到调试目标后被pywdbg执行

pywdbg会向被执行的脚本中传入一个全局变量wd，用于调用API接口

> wd是一个WDbg类的实例，所有的API调用都要通过这个对象进行

### 交互式脚本执行环境

如果启动pywdbg时没有指定任何调试目标，用户需要手动调用wd.create或wd.attach等API来启动一个调试会话，进入调试会话后用户将进入一个python的交互式命令行(ptpython)，使用wd.xxx(...)来调用API接口，xxx为具体的API函数名称

常用的流程控制API有：
* wd.go() 继续运行
* wd.stepinto() 单步进入
* wd.stepover() 单步跳过

### 执行windbg命令

使用wd.cmdloop()来进入一个类似于cdbg和windbg的交互式命令行环境，在里面直接执行g,p,t等dbgeng支持的命令

使用退出命令可退出windbg命令的执行，默认的退出命令是`..`，你也可以通过`wd._exitcmd = XXX`来指定

> 也可以使用Ctrl-C来退出，但是不建议使用，因为有时候会导致死循环，无法继续工作

### 下断点

使用wd.addbp来添加一个断点，添加成功后会返回一个Breakpoint对象，对断点的所有操作都会通过这个对象进行，具体的使用方法可参考PYWDBG-API中的说明

### 事件处理

wdbg支持模块加载/卸载、线程创建/退出、进程创建/退出、异常等事件，具体使用方法可参考PYWDBG-API中的说明

### 退出(调试会话)

* wd.end_detach() ----------- 分离调试目标
* wd.end_terminate() -------- 终止调试目标
