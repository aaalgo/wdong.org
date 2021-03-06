Title: Receptive Field Size of Convolutional Network
Date: 2017-01-19
Modified: 2017-01-19
Category: Technology
Tags: Deep Learning, Image

The receptive field of a cell in a convolutional neural network (CNN)
is the underlying image region that affects the computation
of this cell.  For a CNN to effectively
recognize an object, the receptive field size of the
narrowest layer of the network must be large enough to 
capture the characterisic structures of that object.
Receptive field being too big can be problematic, too.
In addition to costing excessive computation, the
neural network might be forced to learn structures
that surround, but are not intrinsically attached to or associate with
the object.

[This program](https://github.com/aaalgo/tfgraph) automatically
computes the receptive field size of the narrowest layer of
a TensorFlow model.  This can be used as a guide to modifying a
standard network architecture to work with a simple dataset.


