Title: Go是怎么来的?
Date: 2015-10-15
Category: Technology
Tags: Programming, 中文

pthread是包在OS线程外的。light-weighted thread需要用到用户态线程。
微软的fiber是用户态的，但是因为是windows世界的，不怎么招人待见。
在多核上做用户态线程是一件非常非常恶心的事情，GO最主要的贡献其实
就是把这件事做成了。主要的恶心之处就是怎么处理job stealing:一个
操统线程上面的任务都跑完了，就需要去别的操统线程那儿把活弄过来。
这就涉及到各种同步，各种locking。Locking多了，性能就下来了。
这种事情以前应该不少公司内部都有人做过，能做这个的人一般也都
屌得不得了。其实稍微对比一下就能知道multi-core有多难做：
node.js不支持multi-core，python折腾这么多年也还是个残疾。
如果你想更多地了解一下，可以从man makecontext
看起。每个用户态线程其实是一个context。然后底下每个操统线程负责
管一堆context。context切换主要靠cooperative scheduling，而不是操统
用的preemptive scheduling。也就是说一个context运行到某一步自己主动
把执行权让出来。Unix世界的一般没见过cooperative scheduling. 
Windows 3.x是cooperative scheduling，所以线程跑一会就得调用yield
让出执行权。因为不可能要求程序员写几行程序就插入一个yield，所以
其实Windows很多UI和I/O的API都内嵌了yield。那些差的程序员不知道
这回事，有时候进入一个纯计算的循环没有在中间插入yield，就会导致
系统挂起。Unix世界从一开始就是pre-emptive的，操统API没有内嵌yield
这回事。手写程序隔几行插入yield也不可行。这就是unix世界的C/C++做
用户态线程几乎不肯能的原因。这也是为啥rob pike非要搞一个新的语言的
原因：在操统API外包一层，并且嵌入yield（还有就是GC)。GO在语言层面
上其实没有任何创新，甚至比好多现有的语言都要低级。如果从出发点来
看，GO的目的其实已经达到了。相比而言，Windows有在API
里面做yield的传统，这也是为什么这么容易搞出来fiber的原因。

C++11的thread API里有个莫名其妙的this_thread::yield，其实就是为了
给non-preemptive的runtime留下余地。理论上说，如果禁止调用操统API，
全都用C++的I/O库，C++是有可能做出来用户态线程的runtime的。有的嵌入
式系统本就没有preemptive scheduling, yield就成了必须得了
