# 如何理解c++的编译链接过程

## tips
  本文只讨论linux下c++的编译链接过程。

## 概述  
  编译链接的过程大致如下：  
  源代码 => (demo.cpp) => 预编译 => (demo.i) => 编译 => (demo.s) => 汇编 => (demo.o) => 链接 => (demo.out),得到的demo.out即为可执行程序  
  上面的流程看似清晰，但内部的工作细节确实很复杂的。我只大致了解了一部分内容，所以下面简要的说一说个部分的工作内容。
## 预编译  
  如果你有写过c/c++程序，就一定见过这样的代码：
  ```  
  #include<iostream>
  #include<stdlib.h>

  #define PI=3.1415926

  #ifndef WIN_32
    #include<socket.h>
  #else
    #include<winsock2.h>
  #endif
  ```  
  它们都是一系列的预编译指令，是预编译的主要工作。这些指令的用处很大，有的是用来包含头文件，有的是宏定义，有的是针对不同平台的包含指令。完成这些工作的程序叫做**预编译器**。
  预编译器还有一些其他的工作，比如删除所有注释、为每一行生成一个行号（为编译器报错提供便利）等。
  还有一点要特别说明，**#pragma** 指令由编译器执行，而不是预编译器。这里简单举个例子，```#pragma once```是一条由编译器执行的指令，它指示这个代码文件只用编译一次，这样可以减少重复编译，毕竟很多代码都会使用```#include<vector>```这样的语句，没有必要反复编译vector代码。  
## 编译  
  编译是一门很大的学问，它主要包含词法分析、语法分析、语意分析、中间语言生成、目标代码优化及生成几个步骤。
  这里我只有一点接触。词发分析就是分词的过程，里面用到了状态机（类似数电里面的状态转换图）;词法分析就是把分好的词法单元(tokens)生成为一颗语法树（我在TinyMatlab里就有类似的拙劣模仿）;语义分析就是进一步生成抽象语法树，每个节点会添加额外的信息，如运算符‘\*’会被标识为乘号或者取内容符;在往后的过程我里了解的更少，不再多说了。  
## 汇编  
  我从来没看过汇编的东西，就不好意思胡扯了。
## 链接  
  这篇文章主要就是想谈谈链接这个过程，包括静态链接和动态链接。这个学期踩过无数关于链接的坑，不堪回首，有的至今都没有解决。  
  如果你在windows下翻翻各个程序的目录，会发现很多类似.dll,.lib的文件，这些就是所谓的 **库文件**。  
  我主要想讨论链接过程中的符号加载问题，也就是不同文件之间如何相互引用函数、共享变量。剩下的篇幅主要围绕ELF（Excutable Linkable Format）文件来展开，即经过汇编产生的目标文件。  
### 目标文件
  Linux中，目标文件后缀一般为 __.out__ 或 __.o__,这种文件包含的数据分为两类：程序指令，程序数据。它们又分布在不同的 **段(section)** 。这里只讨论如下段:  
  
|段名称|内容|补充说明|  
|-----|------|------|  
|文件头|关于目标文件本身的一些基本信息，如文件格式、平台、包含的段等||  
|数据段|\.data 包含源文件中定义的全局变量和局部静态变量||  
|BSS段|\.bss 包含源文件中定义的未初始化的全局变量和局部静态变量|这部分的变量可以简单的认为值为0|  
|代码段|\.text 或 .code 包含编译后的机器指令|上面两个数据段是不包含普通局部变量的，因为普通局部变量都是直接通过机器指令在内存上生成的，又因为设计到分支问题，变量不一定会被定义，所以和代码段结合在一起|  
|符号表|\.symtab 记录目标文件中用到的所有符号，包括函数名、变量名等|每个符号都有自己的值，函数和变量的值就是它们的地址|  
  
  Linux下使用 readelf 和 objdump 命令就可以查看目标文件的信息。如  
   查看.data段的信息：`readelf -s .data FILENAME`  
   查看目标文件的汇编代码：`objdump -S FILENAME`  

### 符号表
  链接过程最关心的就是符号表。
  链接解决的问题是 **不同目标文件如何引用彼此函数或变量** 。在汇编代码被cpu执行的时候，有很多的地址跳转工作，它们主要是在访问变量、调用函数时发生的，所以必须知道待访问资源的地址。因此，只要链接能够将所有符号的地址确定就能完成任务，也就是建立符号名和符号地址的映射。
  那么，什么是符号名呢？其实是比价复杂的，这里只做简要介绍，具体可以查阅参考资料。
  * 变量  
   变量的符号名就是变量名。  
  * 函数  
   函数的情况要复杂的多，它们要经过 **符号修饰** 得到 **函数签名** 来作为符号名。因为c++涉及到函数重载、函数重写、名字空间、所属对象的问题，而且不同平台采用的符号修饰方法不同，实在是很难讲明白，而且也没有必要弄明白。我们的目的不是去写编译器，而是为了能够解决编译过程中的链接错误。
   符号修饰的方法大概就是把各种信息列据一遍，这里举几个例子。说明一点，我们的以Linux平台为主，所以下表中vc++的符号不保证完全正确。

   |函数|gcc环境|vc++环境|
   ----|----|----|
   int fun(int)|\_Z3funi  |?fun@@YAHH@Z |
   float fun(int)|\_Z3funf|?fun@@YAMH@Z|
   int moon::fun(float)|\_ZN4moon3funi|?fun@moon@@AAEHM@Z|
   float moon::my_function::fun(int)|\_ZN4moonN11my_function3funf|?fun@moon@my_function@@AAEHH@Z|


   
   其实，如果能看懂这张表格，再遇到编译器报链接错误应该就能猜出来错误处在哪里。windows下的c++环境并不友善，虽然有强大的VS支撑 ，隐藏了很多底层的细节，但正因为如此，出错的时候才不好解决。特别是当你看到VS报出那一大堆链接错误时，密密麻麻的符号挤在那么小的窗口里，字体又小，连仔细看的欲望都没有。
   我遇到的链接错误有两种，一种是自己的代码不规范，声明和定义不分，还有一种是库文件没有设置好，很多符号都加载不出来。

未完待续。