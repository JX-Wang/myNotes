这个仓库仅用于笔记记录


## 一些点

* eviron的错误访问
  原以为直接导致了cout的崩溃,实际上是```cout<<nullptr<<endl;```的问题.


## summary
|内容|注意|
|-|-|
|eviron表|使用建议:位于栈上方,进程内存启始处,修改请慎重|
|进程限制|个人观点:一般不会改动,应该在程序上优化,而不是资源优化|
