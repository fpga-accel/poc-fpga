Kernel基于c语言实现的矩阵乘法实例
================================
[Switch to the English version](./README.md)

这是一个基于opencl的16*16矩阵乘法实例，其kernel部分基于c语言实现. 

此示例主要包含:

示例文件
---------------------
应用层host代码

* test-cl.cpp

编译脚本
--------------------------------
* Makefile : 编译应用层（host代码）的编译脚本

** 说明 **
--------------------------------
* 必须已经安装SDx，然后才能使用xcpp工具 

先编译host代码后执行 **run.sh** 命令来测试该用例，具体的操作命令如下：
--------------------------------

```
make
sh run.sh mmult bin_dir/bin_mmult_hw.xclbin

```