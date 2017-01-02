Title: Android WebKit Rendering Pipeline and Instrumentation
Date: 2013-07-08
Category: Technology
Tags: Android

The Android WebKit rendering pipeline is summarized in [this document]({attach}webkit-draw1.pdf).  The final section provides a patch to the android source code that will log all the rendering activity.  A sample browsing session is traced with this code, with all image-rendering activities logged and offline processed and rendered with a standalone installation of Skia.  Each time WebKit updates the screen, an frame is produced from the log, and concatenating these frames with ffmpeg makes [this video]({attach}trace1.mpg).
In this way, I'm able to log all images that are actually drawn on the screen during a browsing session.

I wrote this code to study opportunities for bandwidth saving -- an image in a webpage won't have to be actually downloaded if it is not shown at all, and only a low-resolution version of the image, given multiple resolutions are available, is needed if only a zoomed thumbnail of the image is shown.
