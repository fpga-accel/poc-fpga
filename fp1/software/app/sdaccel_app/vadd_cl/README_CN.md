Kernel基于opencl c语言实现的矢量相加实例
===============================
[Switch to the English version](./README.md)

这是一个基于opencl的矢量相加实例，其kernel部分基于opencl c语言实现. 

这个示例主要包含:
---------------------
应用层host代码

- vadd.cpp
- vadd.h


编译脚本
--------------------------------
* Makefile : 编译应用层（host代码）的编译脚本

** 说明 **
--------------------------------
* 必须已经安装SDx，然后才能使用xcpp工具 


先编译host代码然后执行 **run.sh** 命令来测试该用例，具体的操作命令如下：
--------------------------------

```
make
sh run.sh vadd ./bin_vadd_hw.xclbin

```