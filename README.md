filmulator-gui
==============

A Qt Quick GUI adaptation of Filmulator --- a film emulator with all of the positives and none of the negatives.

Filmulator accepts raw files from cameras and simulates the development of film exposed to the same light as the camera. For various reasons, this inherently brings about several benefits:

* Large bright regions become darker, compressing the output dynamic range
* Small bright regions make their surroundings darker, enhancing local contrast
* In bright regions, saturation is enhanced, helping retain colors in skies and sunsets
* In extremely saturated regions, the brightness is attenuated, helping retain detail in flowers.

The program's design ideology is to have the best tool for any job, and only that one. The tradeoff here is a slight decrease in flexibility, but with a greatly simplified and streamlined user interface.

# Building Filmulator

This program depends on:

libtiff
libgomp
libexiv2
libjpeg
libraw

Some libraw package maintainers don't include the GPL demosaic packs, so we highly encourage you to compile it yourself.

It also requires Qt 5.2: open the .pro file from Qt Creator and select Build in order to run it.


# Status

If told to make a version number for it right now, it'd be 0.3 Alpha.

Currently, the photo editor is mostly complete, although noise reduction and sharpening are currently missing. The queue isn't implemented yet (you can directly open raw images though with a button). Importing mostly works (I use it to grab files off of my cards), but doesn't yet generate thumbnails. The database format might change in the future, breaking old versions' libraries.

But in the meantime, feel free to play around. Report any bugs or suggestions you may have!
