Title: Cost of Padding for Convolution
Date: 2013-09-03
Category: Technology
Tags: Deep Learning


Padding an image for convolution sounds like a pain.  [Cuda-convnet](https://code.google.com/p/cuda-convnet/) has a clever way of implement it, but I found it might be an over-engineering for me to do similar optimization for my CPU-based implementation.  The following operf profiling result shows that even if I simply do padding by copying to a larger matrix the overhead is negligible.  The last two lines are the cost of the padding node.  % of time spent on update and predict are 0.0686% and 8.2e-04%.

operf output:

```bash
CPU: Intel Sandy Bridge microarchitecture, speed 3.201e+06 MHz (estimated)
Counted CPU_CLK_UNHALTED events (Clock cycles when not halted) with a unit mask of 0x00 (No unit mask) count 100000
samples  %        image name               symbol name
1108937  18.1060  libgomp.so.1.0.0         gomp_barrier_wait_end
1082524  17.6747  libgomp.so.1.0.0         gomp_team_barrier_wait_end
1078260  17.6051  cifar.ptblas             MNLOOP
475213    7.7589  cifar.ptblas             MNLOOP
374029    6.1069  no-vmlinux               /no-vmlinux
316764    5.1719  libc-2.15.so             __memmove_ssse3_back
275784    4.5028  cifar.ptblas             _ZN6hiperfit6neural10WindowNode6updateEi._omp_fn.17
272491    4.4490  cifar.ptblas             ATL_gemoveT_aX
143035    2.3354  cifar.ptblas             ATL_scol2blk_a1
110492    1.8040  cifar.ptblas             _ZN6hiperfit6neural8PoolNodeINS0_4pool3maxEE7predictEi._omp_fn.3
102437    1.6725  cifar.ptblas             MNLOOP
83093     1.3567  cifar.ptblas             _ZN6hiperfit5ArrayIfE5applyIZNS_6neural12FunctionNodeINS3_8function4reluEE6updateEiEUlRffffE_EEvRKS1_SB_SB_RKT_._omp_fn.10
79453     1.2973  cifar.ptblas             hiperfit::neural::ArrayNode::preupdate(int)
74185     1.2112  cifar.ptblas             _ZN5cifar7DataSetC2ERKSsbj.constprop.352
68051     1.1111  cifar.ptblas             ATL_sJIK0x0x72TN72x72x0_a1_bX
67752     1.1062  cifar.ptblas             _ZN6hiperfit6neural8PoolNodeINS0_4pool3maxEE6updateEi._omp_fn.2
65919     1.0763  cifar.ptblas             ATL_sJIK0x0x0TN0x0x0_a1_bX
60774     0.9923  cifar.ptblas             _ZN6hiperfit5ArrayIfE5applyIZNS_6neural12FunctionNodeINS3_8function4reluEE7predictEiEUlRffE_EEvRKS1_RKT_._omp_fn.11

......

4201      0.0686  cifar.ptblas             hiperfit::neural::PadNode::update(int)
......
50       8.2e-04  cifar.ptblas             hiperfit::neural::PadNode::predict(int)
......
```
