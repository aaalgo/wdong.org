Title: JPEG vs JPEG 2000 VS WebP
Date: 2012-04-12
Modified: 2017-01-01
Category: Technology
Tags: Image

Evaluation protocol:

A small number of web images are collected from the VPN trace.  These images are first converted to lossless formats (png and pnm).  The programs jpeg, jasper and webp are then used to compress the images with various quality numbers and then decompress to a lossless format.  The program imgcmp is used to compare the original image and the decompressed images.  Compression ratio is the size of the compressed file divided by thatof the lossless png file.

A few error bars are also shown for jpeg with quality=30 as reference.  This configuration is with noticable yet tolerable noise.  (These bars fall into two groups: photos and cartoons.  The baseline PNG is good at compressing cartoons, so jpeg has a relatively low compression ratio and high error for this group.)

The two performance measures lead to the same conclusion:<span style="color: #ff0000;">  webp wins when compression ratio &lt; 0.12, and jpeg 2000 wins when compression ratio &gt; 0.12.  The performance of jpeg is always comparable to the worse of the other two.</span></h5>
&nbsp;

[ui]({attach}:codec-rmse.png)
[ui]({attach}:codec-psnr.png)
[ui]({attach}:montage.jpg)

Image Format Distribution:
- 1146 image/jpeg
- 194 image/gif
- 188 image/png</pre>

