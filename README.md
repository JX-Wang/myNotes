master分支太乱了，暂时不想整理，所以重新开一个分支来放笔记

<!-- TOC -->

- [编程语言](#编程语言)
	- [C++](#c)
	- [Python](#python)
- [TCP/IP](#tcpip)
- [数据库](#数据库)
- [Web](#web)
- [算法数据结构](#算法数据结构)
- [数据结构](#数据结构)
- [操作系统 & Linux](#操作系统--linux)
	- [编程](#编程)
	- [内核](#内核)
		- [文件子系统](#文件子系统)
		- [网络子系统](#网络子系统)
		- [内存管理子系统](#内存管理子系统)
		- [进程调度子系统](#进程调度子系统)
		- [进程通信子系统](#进程通信子系统)
		- [其他](#其他)

<!-- /TOC -->

# 编程语言

## C++

- [x] [STL中迭代器的实现及失效问题](cpp/iterator/README.md)

- [x] [emplace_pack VS push_back](cpp/emplace-vs-insert/README.md)

- [x] [RVO VS std::move](cpp/RVO-vs-move/README.md)

- [x] [内存的分配和释放(malloc/free,new/delete operator,new/delete expression)](cpp/memory-control/README.md)

- [x] [sizeof到底是什么](cpp/sizeof/README.md)

- [x] [static,extern,thread_local的作用](cpp/storage-class-specifiers/README.md)

- [x] [inline函数](cpp/inline-function/README.md)

- [x] [对齐策略](cpp/align/README.md)

- [x] [线程安全,循环引用 与 智能指针](cpp/smart-ptr/README.md)

- [x] [各种继承及其对象内存布局](cpp/oop/README.md)

- [x] [类型转换](cpp/cast/README.md)

- [ ] volatile关键字

- [ ] 线程安全和内存分配,异常处理

- [ ] 继承,虚拟继承,抽象类;不同继承下构造函数的调用顺序,析构函数的调用顺序

- [ ] 什么是异常安全？解决什么问题？如何解决？

- [ ] 如何处理构造函数和析构函数里可能引发的异常？

- [ ] 异常的实现方式

- [ ] 默认,复制,拷贝构造函数的原理与使用

- [ ] 完美转发的实现方式

- [ ] 引用和指针的区别(汇编等角度)

- [ ] extern关键字对于编译流程的影响

- [ ] static关键字对于编译流程,对象构造析构顺序的影响

- [ ] 为什么说allocator是无用的设计(参考陈硕的博客)

## Python

- [ ] 迭代器,生成器,装饰器的使用

- [ ] 函数式编程

# TCP/IP

- [ ] TCP的so\_reuseport和so\_reuseaddr

- [ ] TCP的流量控制(传输速率在不同网络状态下的变化)

- [ ] 什么是路由汇聚?

# 数据库

- [ ] Mysql索引的使用和实现原理

- [ ] 事务提交的注意事项和实现细节

- [ ] SQL注入原理

- [ ] 读写分离的原理

- [ ] innoDB如何管理重做日志

- [ ] NoSQL思想

# Web

- [ ] Restful风格api的设计

# 算法数据结构

- [x] [伙伴系统的实现](alg/buddy)

- [x] [KMP算法的实现](alg/kmp)

- [x] [快速幂](alg/quick-power)

- [x] [快速幂取模](alg/quick-mod)

- [ ] 桶排序,快速排序,堆排序的实现

- [ ] 马太效应

# 数据结构

- [ ] 红黑树,B树的大致工作原理和特性


# 操作系统 & Linux

## 编程

- [x] [进程间通信的几种方式](linux/ipc)

- [x] [epoll,poll和select的使用、实现与区别](linux/select-poll-epoll/README.md)

- [x] [字节序判断问题](os/endian/README.md)

- [ ] ELF文件格式之段分布

## 内核


### 文件子系统

- [x] [VFS的四个关键对象](linux/kernel/vfs/note5-vfs-hierarchy.md)

- [x] [页高速缓存,文件打开关闭和读写](linux/kernel/vfs/note6-page-cache-and-file-ops.md)

- [ ] 文件描述符的实现方式(从普通的文件io和网络io等角度去讨论)

- [ ] VFS的四层抽象是什么？为什么要这样抽象？每层抽象解决什么问题？

- [ ] 内存映射的原理及应用

- [ ] 页高速缓存和缓冲区高速缓存的区别

### 网络子系统

- [x] [socket实现原理](linux/kernel/net/note7-whats-socket.md)

- [ ] epoll和select的实现方式

- [ ] UDP socket在收包发包时的具体细节(为什么可以用一个socket向很多个服务器发包再使用select收包)

- [ ] tcp的关键实现函数

- [ ] udp的关键实现函数

### 内存管理子系统

- [x] [内核申请内存的机制和接口](linux/kernel/mmu/note1-how-kernel-get-memory-for-itself.md)
- [x] [地址空间和内存区域](linux/kernel/mmu/note2-how-kernel-manage-mem-for-process.md)
- [x] [地址翻译的细节](linux/kernel/mmu/note3-how-kernel-manage-memory-page.md)
- [x] [malloc,free的实现方式](linux/kernel/mmu/note4-malloc-and-free.md)
- [x] [再谈mmap和munmap](linux/kernel/mmu/note8-inside-mmap-munmap.md)

- [ ] 缺页中断的细节

### 进程调度子系统

- [ ] 线程的实现方式(从stuct_task的角度去分析group,thread,process)程通信子系统

### 进程通信子系统

- [ ] 几种常见IPC方法的使用和原理

### 其他

- [x] [动态链接 VS 静态链接](os/static-vs-dynamic-link/README.md)

- [ ] 如何编译内核

- [ ] 如何理解struct percpu_struct结构
