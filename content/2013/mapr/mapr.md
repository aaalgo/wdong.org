Title: Pushing Code to Data: A MapR Exerciese
Date: 2013-03-18
Modified: 2017-01-01
Category: Technology
Tags: Big Data

<span style="color:red;font-size:24px;">In this blog I'll demonstrate how to push code to data stored on a MapR (or Hadoop) cluster and achieve an order of magnitude speedup with simple bash and C++ coding and without any of the MapReduce and Java stuff.</span>

I just had a MapR cluster set up; and in this exercise, I'm going to test the data-processing speedup I can gain by pushing the code to run where data is.  The goal here is to gain some knowledge on the overhead of various system component (e.g. the hadoop commandline) so I can design a in-house platform for distributed data processing.

## Setup

The data I have are mostly files of roughly fixed size, about 30MB each.  I need to store a large number of such files in the MapR filesystem, and need to frequently run some processing command on each file.  The output generated from each file is much smaller than the input and is negligible.  In this experiment, I'll use 100 such files as input, and use "md5sum" as the operation.  The files are already stored on the MapR filesystem, with the text file "list" containing a list of the paths to the 100 files.

The hardware setup is as follows.  The cluster has 8 nodes, with 7 forming a MapR cluster.  All the commands and operations are done on the remaining node with the hostname "washtenaw".  

## Pulling Data From MapR FS: The Naive Approach

The simplest approach is simply to run all the processing locally on washtenaw, pulling all the data needed from the MapR FS.  Following is the script "run-local.sh":

```baseh
#!/bin/bash
# run-local.sh
cat list | while read name
do
    printf '%s\t' $name
    hadoop fs -cat $name | md5sum 
done
```

And the performance is
```
wdong@washtenaw $ time ./run-local.sh > md5.hadoop

real    1m14.910s
user    1m10.304s
sys 0m19.573s
```

Roughly we spend 7.5s on each file.  

## Pulling Data From MapR FS without Java </h3>

We already know that MapR FS is able to achieve a throughput of 80MB/s from the [prevous post](/mapr-file-copy-throughput.html).  So it should take only 30/80 = 0.375s to retrieve each file from the cluster (without the name node lookup and all other latencies). This is much smaller than the 7.5s we spent in our first setting.  An obvious overhead is the cost to start the hadoop command line.  So in this setting, I'll test directly fetching the data using the C API.  No java code is involved in this setting, but the data is still loaded remotely from the cluster.

Here's the C++ source code:
```cpp
// run-c++.cpp
#include "hdfs.h" 
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    static size_t BUFFER_SIZE = 64 * 1024 * 1024;
    string buffer(BUFFER_SIZE, '\0');

    hdfsFS fs = hdfsConnect("default", 0); 
    string path;
    while (cin >> path) {
        cout << path << ' ';
        hdfsFile h = hdfsOpenFile(fs, path.c_str(), O_RDONLY, BUFFER_SIZE, 0, 0); 
        cout.flush();
        FILE *cmd = popen("md5sum", "w");
        for (;;) {
            size_t sz = hdfsRead(fs, h, &buffer[0], buffer.size());
            fwrite(&buffer[0], sz, 1, cmd);
            if (sz != buffer.size()) break;
        }   
        pclose(cmd);
        hdfsCloseFile(fs, h); 
    }   
    hdfsDisconnect(fs);
    return 0;
}
```

And here's the performance:

```bash
wdong@washtenaw$ make run-c++
g++ -std=c++11 -Wall -O3 -I/opt/mapr/hadoop/hadoop-0.20.2/src/c++/libhdfs   -L/opt/mapr/lib -Wl,-allow-shlib-undefined  run-c++.cpp  -lMapRClient -o run-c++
wdong@washtenaw$ time ./run-c++  < list > md5.c++

real    0m42.258s
user    0m6.880s
sys 0m16.077s
```

We've reduced per-file processing time from 7.5s to about 4.2s.  The overhead of running the hadoop command line for each file is about 3s -- that is pretty big.

## Pushing Code to Data

Here comes the real stuff.  I'll detect on which node a file is stored, and push our code to where the data is.  I use the following C++ program to query the location of a file, assuming that each file is contained in one filesystem block with no replication, which is the case in my system setup.

```cpp
// hdfs-lookup.cpp
#include "hdfs.h" 
#include <string>
#include <iostream>
#include <boost/assert.hpp>

using namespace std;

int main(int argc, char **argv) {
    hdfsFS fs = hdfsConnect("default", 0); 
    string path;
    while (cin >> path) {
        // get the block of the 1st byte
        char ***hosts = hdfsGetHosts(fs, path.c_str(), 0, 1); 
        BOOST_VERIFY(hosts[0] && hosts[0][0]);
        cout << path << ' ' << hosts[0][0] << endl;
        hdfsFreeHosts(hosts);
    }   
    hdfsDisconnect(fs);
    return 0;
}
```

Running the program produces something like the following: each line containing the path followed by the hostname where the data is:

```bash
wdong@washtenaw$ make hdfs-lookup
g++ -std=c++11 -Wall -O3 -I/opt/mapr/hadoop/hadoop-0.20.2/src/c++/libhdfs   -L/opt/mapr/lib -Wl,-allow-shlib-undefined  hdfs-lookup.cpp  -lMapRClient -o hdfs-lookup
wdong@washtenaw$ ./hdfs-lookup < list
test/data1 fuller
test/data2 ford
test/data3 huron
test/data4 huron
test/data5 plymouth
...
```

I then use the following script to drive the computation:

```bash
#!/bin/bash
# run-remote.sh

mkdir -p input; rm -f input/*
mkdir -p output; rm -f output/*

cat list | ./hdfs-lookup | while read path host
do
    echo $path >> input/$host
done

for host in `ls input`
do
    ssh $host "cd $PWD; cat input/$host | ./run-c++ > output/$host"
done

cat output/*
```

And here's the performance:

```bash
wdong@washtenaw$ time ./run-remote.sh > md5.remote

real    0m13.529s
user    0m0.116s
sys 0m0.040s
```

Substantial speedup!  The time we spent on each file is 0.135s.

## Pushing Code to Data with Parallelization </h3>

Finally, I go ahead to parallelize the above driving script:

```bash
#!/bin/bash
# run-parallel.sh

mkdir -p input; rm -f input/*
mkdir -p output; rm -f output/*

cat list | ./hdfs-lookup | while read path host
do
    echo $path >> input/$host
done

for host in `ls input`
do
    ssh $host "cd $PWD; cat input/$host | ./run-c++ > output/$host" &
done

wait

cat output/*
```

And here's the performance:

```bash
wdong@washtenaw$ wc -l input/*    # just to show the data distribution among the nodes.
  12 input/ford
  10 input/fuller
  21 input/geddes
  19 input/huron
   9 input/maple
  10 input/plymouth
  19 input/wagner
 100 total
wdong@washtenaw$ time ./run-parallel.sh > md5.parallel

real    0m2.987s
user    0m0.120s
sys 0m0.068s

wdong@washtenaw$ for i in md5.* ; do sort $i | md5sum; done  # just to check that all outputs are the same
a3889305ff721ec32ced48a3066ea059  -
a3889305ff721ec32ced48a3066ea059  -
a3889305ff721ec32ced48a3066ea059  -
a3889305ff721ec32ced48a3066ea059  -

```

That is a speedup of 25x over my initial naive approach, but at his point, there's really no surprises.  More speedup can be achieved by parallelizing the program run-c++.cpp, but it would not be much meaningful in this setting as for much larger dataset the bottleneck will be the disks, and I only have one disk attached to each node.

<span style="color:red;font-size:24px;">
A note for hadoop users:  when building the C++ programs, link against libhdfs.so and libjvm.so instead of libMapRClient.so; you'll also have to set up the java environment properly.
