Introduction
============
This is a 3D viewer for Wavefront .obj files.

It supports a reasonable subset of the .obj (geometry) and .mtl (material) file
formats. Support for the rest depends on my finding sample models which makes
use of those parts.


License
=======
This code is licensed under the GPL v3. A full copy of the license terms can be
found in the accompanying LICENSE.txt file, or on the Free Software
Foundation's website at http://www.gnu.org/licenses/gpl.html


Pre-requisites
==============
- MacOS X or Linux (tested on MacOS X 10.6).
- Glut
- ImageLib -- see http://github.com/vilya/ImageLib
- A graphics card which supports OpenGL 2.1 or above. 


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

