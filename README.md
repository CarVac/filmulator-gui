filmulator-gui
==============

A Qt Quick GUI adaptation of Filmulator --- a film emulator with all of the positives and none of the negatives.

Filmulator accepts raw files from cameras and simulates the development of film as if exposed to the same light as the camera's sensor. For various reasons, this inherently brings about several benefits:

* Large bright regions become darker, compressing the output dynamic range.
* Small bright regions make their surroundings darker, enhancing local contrast.
* In bright regions, saturation is enhanced, helping retain color in blue skies, brighter skin tones, and sunsets.
* In extremely saturated regions, the brightness is attenuated, helping retain detail e.g. in flowers.

The program's design ideology is to have the best tool for any job, and only that one tool. The tradeoff here is a slight decrease in flexibility, but gaining a greatly simplified and streamlined user interface.

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

First, import your photos. The import process by default sorts the photos into directories based upon the date captured, in both the Destination Directory and the Backup Directory. These can be sorted correctly by setting the Camera UTC Offset to the timezone you set the camera's clock in, and the Local UTC Offset to the timezone where the photo was captured. Select the source directory (your card, or a directory containing your photos), and pressing the Import button will begin pulling the photos into the database and writing out the source directory.

Next, go to the Organize tab, where you can select photos to enqueue. The calendar lets you select photos from a given day, while the Timezone slider lets you adjust how to 'slice' the day: it always displays from midnight to midnight.

To enqueue photos, double-click on them in the grid view in the organize tab. To remove them from the queue, right-click on the queue entry and select 'Remove from queue'.

At this point, we get to the Filmulate tab, which lets you process your photos. Double-click on a photo to load it through the filmulation process, and set the sliders to your taste.

The tools are arranged in pipeline order: the processing they control occurs from top to bottom. Thus, we advise you mostly work top-down through the tools.

Tools and features of interest:
* In-pipeline mini histograms: They display the distribution of the pixel brightnesses right before and right after the filmulation algorithm. Use the first one to adjust exposure compensation and highlight recovery, and use the second one to adjust the output clipping.
* Highlight Recovery: If nothing is clipped, leave highlight recovery at 0. If there is clipping, skip right to 3. If there are clipped skin tones, 9 tends to work well, but it's really slow. 1 and 2 are useless, while 4-8 are weighted blends of 3 and 9.
* Film Area: Here's where it gets good. The program defaults to an approximate equivalent of 36x24 film. If you reduce the film size, the algorithm accentuates brightness in larger regions. If you increase the film size, the whole image becomes slightly flatter (reduced dynamic range), but it emphasizes smaller details.
* Drama: This is effectively the strength of the 'filminess'. Higher values reduce the dynamic range of the output; you'll see the "post-filmulator" histogram shrink towards the left. With the default film area, feel free to turn this all the way up to 100 if you need to bright down the highlights, but with large-format equivalent film area it starts to get too flat-looking past 50.

# Status

If told to make a version number for it right now, I'd put it as 0.5 Alpha.

Currently, the photo editor is mostly complete, although noise reduction and sharpening are currently missing. Both the Import and Organize tabs need some UI massaging, as does the queue. Finally, the Output tab hasn't even been started yet.

There are a few known bugs. The most important is that occasionally, a new image will not load completely. In this case, nudge a slider in order to reload the image.

But in the meantime, feel free to play around. Report any new bugs or suggestions you may have!
