Title: MapR File Copy Throughput
Date: 2013-03-17
Modified: 2017-01-01
Category: Technology
Tags: Big Data, Storage

```

$ du -sh /data/local/wdong/data         # the directory contains a bunch of 30MB files.
15G /data/local/wdong/data

$ time cp -R /data/local/wdong/data .   # copy data via fuse

real    3m9.192s
user    0m0.148s
sys 0m19.581s

$ time hadoop fs -put /data/local/wdong/data test/data1

real    2m56.955s
user    0m16.225s
sys 0m30.286s

```

So whether via fuse or hadoop commandline, the write throughput of mapr is about 80MB/s, with the hadoop commandline being slightly faster.  The overhead of java is actually negative compared to that of fuse. I expect the performance of MapR's native NFS server should beat both.

I'm using a $20 [TRENDnet 8-port Gigabit Switch](http://www.newegg.com/Product/Product.aspx?Item=N82E16833156309) and there is a cluster of 7 MapR servers behind it.

