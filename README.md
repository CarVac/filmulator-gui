filmulator-gui
==============

A Qt Quick GUI adaptation of Filmulator --- a film emulator with all of the positives and none of the negatives.

Filmulator accepts raw files from cameras and simulates the development of film as if exposed to the same light as the camera. For various reasons, this inherently brings about several benefits:

* Large bright regions become darker, compressing the output dynamic range
* Small bright regions make their surroundings darker, enhancing local contrast
* In bright regions, saturation is enhanced, helping retain colors in skies and sunsets
* In extremely saturated regions, the brightness is attenuated, helping retain detail e.g. in flowers.

The program's design ideology is to have the best tool for any job, and only that one. The tradeoff here is a slight decrease in flexibility, but with a greatly simplified and streamlined user interface.

# Building Filmulator

This program depends on:

libtiff
libgomp
libexiv2
libjpeg
libraw

Some libraw package maintainers don't include the GPL demosaic packs, so we highly encourage you to compile it yourself.

It also requires Qt 5.3: open the .pro file from Qt Creator and select Build in order to run it. You may have to initialize the build configurations upon first loading the project; I suggest you add the -j# flag to the Make build parameters to speed compilation.

A note: Use a standalone git client to clone the repository initially, and then you can use Qt Creator's built-in git tools.

# Using Filmulator

For now, head straight for the Filmulate tab. Click the 'Open' button at the bottom, adjust the sliders, and when you're ready, click one of the other two buttons at the bottom to output an image.

Tools of note:
* In-pipeline mini histograms: They display the distribution of the pixel brightnesses right before and right after the filmulation algorithm. Use the first one to adjust exposure compensation and highlight recovery, and use the second one to adjust the output clipping.
* Highlight Recovery: If nothing is clipped, leave highlight recovery at 0. If there is clipping, skip right to 3. If there are clipped skin tones, 9 tends to work well, but it's really slow. 1 and 2 are useless, while 4-8 are weighted blends of 3 and 9.
* Film Area: Here's where it gets good. The program defaults to an approximate equivalent of 36x24 film. If you reduce the film size, the algorithm accentuates brightness in larger regions. If you increase the film size, the whole image becomes slightly flatter (reduced dynamic range), but it emphasizes smaller details.
* Drama: This is effectively the strength of the 'filminess'. Higher values reduce the dynamic range of the output; you'll see the "post-filmulator" histogram shrink towards the left. With the default film area, feel free to turn this all the way up to 100 if you need to bright down the highlights, but with large-format equivalent film area it starts to get too flat-looking past 50.

# Status

If told to make a version number for it right now, I'd put it as 0.3 Alpha.

Currently, the photo editor is mostly complete, although noise reduction and sharpening are currently missing. The work queue isn't implemented yet. Importing mostly works (I use it to grab files off of my cards), but doesn't yet generate thumbnails. The database format is likely change in the future, breaking old versions' saved settings.

But in the meantime, feel free to play around. Report any bugs or suggestions you may have!
