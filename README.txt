Introduction
============
This is a simple OpneGL viewer for Wavefront .obj files.


License
=======
This code is licensed under the GPL v3. A full copy of the license terms can be
found in the accompanying LICENSE.txt file, or on the Free Software
Foundation's website at http://www.gnu.org/licenses/gpl.html


Pre-requisites
==============
- MacOS X or Linux (tested on MacOS X 10.5.8 and Fedora 11 respectively).
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

