Title: fuse_dfs on MapR
Date: 2013-03-017
Modified: 2017-01-01
Category: Technology
Tags: Storage, Big Data

Here's how to get fuse_dfs work with MapR.

<strong>Download the [patched source code]({attach}fuse-dfs.tar.bz2").</strong>

## The fuse_dfs code is at hadoop-hdfs-project/hadoop-hdfs/src/contrib/fuse-dfs/src. 
## Patch the source code a little bit, including the following:
  - in fuse_connect.c, add "#define __USE_GNU 1" before the line "#include &lt;search.h&gt;".
  - in fuse_trash.c, search for "hdfsDelete" and remove the third parameter "1" to the function.
  - in fuse_dfs.c, remove the check for "options.port == 0".  That is because we are going to use exactly the port number 0.

## Use the following Makefile and make fuse_dfs.

```
HADOOP_PREFIX=/opt/mapr/hadoop/hadoop-0.20.2
PACKAGE_VERSION=0.20.2
FUSE_HOME=/usr
PERMS=
PROTECTED_PATHS=
bin_PROGRAMS = fuse_dfs
fuse_dfs_SOURCES = fuse_dfs.o fuse_options.o fuse_trash.o fuse_stat_struct.o fuse_users.o fuse_init.o fuse_connect.o fuse_impls_access.o fuse_impls_chmod.o  fuse_impls_chown.o  fuse_impls_create.o  fuse_impls_flush.o fuse_impls_getattr.o  fuse_impls_mkdir.o  fuse_impls_mknod.o  fuse_impls_open.o fuse_impls_read.o fuse_impls_release.o fuse_impls_readdir.o fuse_impls_rename.o fuse_impls_rmdir.o fuse_impls_statfs.o fuse_impls_symlink.o fuse_impls_truncate.o fuse_impls_utimens.o  fuse_impls_unlink.o fuse_impls_write.o
fuse_dfs:   $(fuse_dfs_SOURCES)
CFLAGS= -Wall -g -DPERMS=$(PERMS) -D_FILE_OFFSET_BITS=64 -I$(HADOOP_PREFIX)/src/c++/libhdfs -D_FUSE_DFS_VERSION=\"$(PACKAGE_VERSION)\" -DPROTECTED_PATHS=\"$(PROTECTED_PATHS)\" -I$(FUSE_HOME)/include
LDFLAGS= -L/opt/mapr/hadoop/hadoop-0.20.2/c++/Linux-amd64-64/lib -L/opt/mapr/lib -L$(FUSE_HOME)/lib -Wl,-allow-shlib-undefined
LDLIBS = -lMapRClient -lfuse 

all:    fuse_dfs
clean:
    rm *.o fuse_dfs
```

## Mount with the following command

```bash
LD_LIBRARY_PATH=/opt/mapr/lib ./fuse_dfs -oserver=default -oport=0 <mount point>
```

We use the MapR client library instead of libhdfs, and there's no Java code involved between fuse and the disks.  (Java is probably more of a psychological issue here than a performance issue here.)



