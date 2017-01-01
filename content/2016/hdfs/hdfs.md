Title: HDFS Demistified: How to Manually Assemble an HDFS File When Hadoop is Down
Date: 2015-01-08
Category: Technology
Tags: Storage, Big Data

In this blog I’ll explain how to manually assemble a file stored in HDFS using 1) meta data on namenode and 2) blocks on datanode. The process does not require the hadoop system to be up and running. It’s an interesting exercise to gain some knowledge about the internal mechanisms of Hadoop, and such knowledge can be handy when it comes to data recovery.

(1) Fetch the fsimage.

Below is a tree view of what are stored in the namenode directory.

```
$ cd HADOOP_NAMENODE_DIR
$ tree .
.
|-- current
|   |-- VERSION
|   |-- edits_0000000000005342851-0000000000005440884
|   |-- edits_0000000000006347975-0000000000006347976
... ...
|   |-- edits_0000000000006347977-0000000000006347986
|   |-- edits_inprogress_0000000000006347987
|   |-- fsimage_0000000000006347976
|   |-- fsimage_0000000000006347976.md5
|   |-- fsimage_0000000000006347986
|   |-- fsimage_0000000000006347986.md5
|   `-- seen_txid
`-- in_use.lock
 
1 directory, 50 files
```

We are interested in the fsimage file with the largest postfix number. Copy it out as “fsimage”. If our file in the HDFS is recently uploaded or modified, then its full metadata might not be present in the fsimage. Some of the data could still be in one of the edits_ files. We won’t be able to fully assemble the most recent version of the file. One way to force Hadoop to produce a new checkpoint is to restart Hadoop. Edit logs are merged into a new fsimage upon restart.

Before proceeding to the next step, it is useful to examine the content of the VERSION file.

```
$ cat VERSION
#Wed Oct 01 01:25:37 CST 2014
namespaceID=1453566641
clusterID=CID-a5f06877-24b3-4892-9dcf-05fccf827889
cTime=0
storageType=NAME_NODE
blockpoolID=BP-908018994-10.10.2.27-1412043710870
layoutVersion=-56
```

We’ll need the blockpoolID information.

(2) Examine the content of fsimage.

Use the following command to dump the content of fsimage to an XML file.

```
$ hdfs oiv -i fsimage -o fsimage.xml -p XML
```

Let’s say we are interested in recovering the file “/user/home/playtime_20140915.txt”. We can find the following relavant information in the fsimage dump.

```xml
<inode><id>16392</id><type>FILE</type><name>playtime_20140915.txt</name><replication>2</replication><mtime>1412052903661</mtime><atime>1418937665301</atime><perferredBlockSize>134217728</perferredBlockSize><permission>wdong:supergroup:rw-r--r--</permission><blocks><block><id>1073741825</id><genstamp>1001</genstamp><numBytes>134217728</numBytes></block>
<block><id>1073741826</id><genstamp>1002</genstamp><numBytes>134217728</numBytes></block>
<block><id>1073741827</id><genstamp>1003</genstamp><numBytes>49999484</numBytes></block>
</blocks>
</inode>
```

We can extract the list of block IDs by either eyeballing or programming.

```
1073741825
1073741826
1073741827
```

We can also add up the number of bytes (318434940). If Hadoop is up, we can verify if the file size is correct.

(3). Gather the blocks.

Hadoop does not maintain a on-disk file or database mapping block IDs to nodes. This is actually a nice stateless design. We’ll need to manually enumerate each node to find the blocks we need. Here’s a sample layout of Hadoop data directory.

```
$ tree .
.
|-- current
|   |-- BP-908018994-10.10.2.27-1412043710870
|   |   |-- current
|   |   |   |-- VERSION
|   |   |   |-- dfsUsed
|   |   |   |-- finalized
|   |   |   |   |-- blk_1073743127
|   |   |   |   |   |-- blk_1073834270_93446.meta
|   |   |   |       |-- blk_1073752388_11564.meta
|   |   |   |       |-- blk_1073801146
......
|   |   |   |       `-- blk_1073801146_60322.meta
|   |   |   `-- rbw
|   |   |       |-- blk_1074397675
|   |   |       |-- blk_1074397675_656923.meta
|   |   |       |-- blk_1074397684
|   |   |       `-- blk_1074397684_656932.meta
|   |   |-- dncp_block_verification.log.curr
|   |   |-- dncp_block_verification.log.prev
|   |   `-- tmp
|   `-- VERSION
`-- in_use.lock
 
390 directories, 16252 files
```

Here we see the blockpoolId we noted before as a directory name. Blocks are simply named as blk_ID in one of the sub directories.

In our cluster, the data nodes are mounted as “/data/hadoop/data*/”. So it is quite easy to launch a cluster-wide search with pdsh.

```
$ pdsh "find /data/hadoop/data*/ -name blk_1073741825"
klose4: /data/hadoop/data3/current/BP-908018994-10.10.2.27-1412043710870/current/finalized/subdir56/blk_1073741825
klose2: /data/hadoop/data2/current/BP-908018994-10.10.2.27-1412043710870/current/finalized/blk_1073741825
```

We see that the block has two replicates. We can modify the above command a little bit to copy the file over:

```
$ for B in 1073741825 1073741826 1073741827 ; do  pdsh "find /data/hadoop/data*/ -name blk_$B" | while read a b; do scp $a$b . ; break; done ; done
```

The break command is to stop us from copying more than one replica.

(4) Assemble the file

```
$ cat blk_1073741825 blk_1073741826 blk_1073741827 | md5sum
ad07d7ced9c9210b4a4b14d08c0d146f -
$ hdfs dfs -cat playtime_20140915.txt | md5sum # only when hadoop is up.
ad07d7ced9c9210b4a4b14d08c0d146f -
```

Bingo!
