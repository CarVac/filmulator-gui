filmulator-gui
==============

A Qt Quick GUI adaptation of Filmulator --- a film emulator with all of the positives and none of the negatives.

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

This is pre-pre-pre-alpha; all it does currently is display a framework that will be filled in; in the Filmulator tab you can display what Filmulator outputs from a raw file pointed to in the box at the bottom.

More is to come soon, hopefully.
