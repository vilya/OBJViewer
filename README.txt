Introduction
============
This is a very basic viewer for Wavefront .obj files.

It supports .obj and .mtl files at the moment, but the support is incomplete at
the moment. The intent is to flesh out this support over time, as well as
adding features to the viewer (which is pretty bare-bones for now too).


License
=======
This code is licensed under the GPL v3. A full copy of the license terms can be
found in the accompanying LICENSE.txt file, or on the Free Software
Foundation's website at http://www.gnu.org/licenses/gpl.html


Pre-requisites
==============
- MacOS X or Linux (tested on MacOS X 10.6).
- Glut


Compiling
=========
It's very simple:

    make dirs   # Only if you're building for the first time.
    make

That's all! Afterwards, the executable will be in the bin/ folder. You only
need to make dirs once: it creates the intermediate and output directories used
by the build. So long as they're there, you can skip it.


Running
=======
If you are in the project root folder, type

    ./bin/objviewer -h

to get information about how to run the app.


Credits
=======
This app was written by Vilya Harvey (vilya.harvey@gmail.com).

