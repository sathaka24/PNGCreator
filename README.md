This is a simple c++ class that can create a png file using the raw pixel data.
In the header file you can see simply constructor do all the things when you create the object.
You just need to set the arguments of raw data containing file path, width, height, and the oup-put image name
Then it create the PNG.

I also add DLL file which contain the createPNG objec and the necessary parts from the zlib.
Huge shoutout to the zlib https://github.com/madler/zlib
