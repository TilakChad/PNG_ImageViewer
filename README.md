# PNG_ImageViewer
Image Viewer for PNG image format (Not for every png image)<br>
Possible future implementations for : <br>
Grayscale - 1-bit image<br>
TrueColor - 8-bit image<br>
IndexedColor - 8-bit image<br>
TrueColor with alpha channeling : 8-bit image<br>
<br>
The aim is to build a working image viewer with above supported png images using Qt or OpenGL.
<br>
<br>
/src contains deflate.c (decompressor) and image.c (filter reverser).
<br>
image.c returns stream of data and used by Qt to render image.<br>
image.c and deflate.c are converted to their respective object files. So the line, <br>
OBJECTS += deflate.o image.o <br>
is mandatory to be included while building Qt project.<br>
C is called from C++ with the help of extern "C" block and linked with object files generated by gcc. <br>

<br>
Use : ./PNG_file_viewer image_name.png
Note : No support for zoom or any other features... <br>{\  /}
<br>                                                 (-_-)
<br>

# independent_src
This contains image viewer that should compile fine on any OPENGL-supported OS(platform). Need to have Glad (can be generated easily from https://glad.dav1d.de/) and glfw3 library though. 'src' and 'indpendent_src' have same except the defualt image renderer. Src use Qt as image renderer while latter uses OpenGL.
<br>
<br>
Dependency for linux : sudo apt install libglfw3-dev
<br>
glad generated from above link
<br>
Any C compiler supporting C98 standard or above
<br>
Compile instruction : 
<br>
gcc image_viewer.c deflate.c image.c ./glad/src/glad.c -lglfw -ldl -o output
<br>


