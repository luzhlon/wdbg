
此目录为wdbg的二进制发布目录

## Directory's structure

* bin: wdbg所需的二进制文件(x86, x64分别对应32位和64位)
* inc: 开发wdbg后台插件所需的头文件
* lib: 开发wdbg后台插件所需的库文件
* pywdbg: 使用python封装的wdbg调试器前台

## How to run

1. 根据你的平台选择bin目录下x86或者x64子目录里的wdbg.exe启动，启动成功后wdbg会输出监听的端口号
2. 通过Python脚本加载pywdbg下的wdbg模块，并通过wdbg.connect函数连接到wdbg后台

**Note**，运行pywdbg需要python3环境以及u-msgpack-python包(可通过pip3 install u-msgpack-python安装)
