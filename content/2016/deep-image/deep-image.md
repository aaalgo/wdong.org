Title: Image Storage For Deep Learning, Raw or JPG?
Date: 2015-11-07
Tags: Deep Learning, Storage, Image
Category: Technology

Caffe requires the user to preload training images into a database, and the
images are stored as raw pixels. The following calculation shows that this is
not a very good idea.

Assume that images are pre-scaled to 256×256, so each raw image costs 256x256x3
= 192KB storage. On the other hand, 255x255x3 JPEG images compressed with
default parameters cost about 48KB, about 1/4 the storage of raw pixels.
Benchmark shows that a 4-core 2600K can decode jpeg images of this size at a
rate of 6500/s using all cores, or 1350/s with one core. If we assume only one
core can be allocated for image decoding, the processing power translates to
63MB/s input throughput of JPEG data, and 253MB/s output throughput of raw
pixels. The sequential read throughput of a traditional HDD is about 100MB,
which is above the input throughput of one core. So an economical design would
be to store the images compressed with jpeg on a tradional HDD, and decode the
image with a dedicated CPU core. The throughput of Caffe, according to the
website, is about 4ms/image for learning and 1ms/image for predicting on a K40
GPU. So the throughput of the above configuration can well saturate the GPU
power even for predicting. The whole system is nice and balanced, and the main
stream HDD provides about 3TB of storage. This also leaves some room for future
growth of GPU power and training image size (HDD/SSD grows in capacity rather
than throughput).

Of course, this all relies on being able to achieve 63MB/s throughput from the
disk, and achieving this on a HDD requires sequential I/O. With images stored
in a database, it requires a very fast SSD to achieve such throughput. That’s
why I developed the PicPoc image storage for deep learning.(Benchmarking show
that sequential read with LMDB DOES achieve raw hardware throughput, whether
HDD or SSD. The storage overhead of LMDB is also reasonably low, around 3% as I
measured with the ILSVRC 2012 dataset.)

Here are some performance numbers I’ve been achieving with preliminary
experiments.

Importing fall 2011 version of ImageNet (14million images stored on 21935 tar
files, totalling about 1.2TB) into PicPoc took about 10 hours. The output is
400GB. The input on one HDD and output on another. CPU usage is 213.6%.
Considering reading 1.2TB from HDD takes about 3.5 hours and CPU usage is about
50%, there’s a possibility to double the loading throughput. But that’s a one
shot business so I’ll say it’s good enough for now. The ILSVRC 2012 training
data, when imported, costs 28GB storage, as apposed to 173GB imported to LMDB
as raw pixels as described in Caffe’s documentation. (One doesn’t have to use
raw pixels with LMDB. The Caffe Datum can be used to store encoded image, and
OpenCV pretty much support all popular image codecs).

On reading with decoding, the system is able to sustain 120MB/s throughput on a
traditional 1TB HDD. I’ve also created a [Caffe fork with PicPoc backend](https://github.com/aaalgo/caffe-picpoc).


