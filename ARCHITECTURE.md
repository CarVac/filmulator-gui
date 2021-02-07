# Architecture

This document is an description of the architecture of Filmulator.

## Overview

Filmulator is written in C++ and QML. All of the image processing and database stuff happens in C++, and much of the user interface stuff happens in QML.

QML interfaces with C++ through properties and methods of QObjects that are hooked up to it in `main.cpp`.

Some of these QObjects deal with the database, providing tables of information that are used to fill the Queue, the Organize view, and the date histogram in the Organize tab.

Another key one, the ParameterManager, handles communicating processing parameters between the user interface, the database, and the image processing pipeline.

The images make their way to the user interface via the FilmImageProvider, which is a special type of QObject called a QQuickImageProvider.

## Code Map

This section talks about important directories and files therein in the code structure.

### `core`

This contains image processing code, plus image I/O.

The key part of this is the ImagePipeline, which actually performs all of the steps in the processing pipeline. It connects to a ParameterManager to grab the latest parameters and to check whether it should cancel the current computation. It also connects to an Interface, which lets it expose histogram data it generates during processing.

Mostly everything else in the directory can be figured out by following through the processing pipeline.

### `database`

This contains objects that write to and read from the SQL database so that information can be presented to the UI.

It also has stuff for accessing ancillary databases, like camconst.json for camera parameters or lensfun for lens correction parameters.

Most of the Models are relatively self-explanatory, but ImportModel does not present info to the UI unlike the others; instead it's mainly about handling the process of importing files. Originally it was intended to provide a view of recently imported files, but it never ended up happening.

### `ui`

This contains code that primarily deals with handling the UI.

The most important, and most complex by far, are the ParameterManager and FilmImageProvider.

The ParameterManager has a tough three-way connection between the UI, the database, and the image processing. More on this later.

The FilmImageProvider actually contains the image pipelines and runs them in a background thread (automatic, part of being a QQuickImageProvider) so that the UI doesn't block while computing. More on this later.

### `qml/filmulator-gui`

This contains the QML code that actually declares the UI and most of its behavior.

Note that there's some degree of boilerplate on every Filmulator-specific component; everything needs to be scaled according to `uiscale`, and tooltip requests must be passed from child to parent all the way up to `main.qml`.

By far the most complexity exists in `Edit.qml` which handles the behavior of the image display, and any tools such as cropping or leveling that interact directly with the image... this has complicated arithmetic to handle making sure that everything scales properly, the image stays centered in the viewport, and that zooming about the mouse occurs properly.

Components that get reused go in the `gui_components` subdirectory.

## The Flow of Info from Slider to Processing

When the user moves a slider, here's what happens (assuming we're using the quick preview)

* The slider's value changes, so it sets the property of the ParameterManager to its value (or whatever function of the value for nonlinear controls).
* The ParameterManager updates the validity. If the current processing is not invalidated, computation continues.
* Additionally, the clone paramManager for the high-res pipeline updates and has its validity set. The high-res pipeline always cancels computation so as to let the quick preview render faster.
* If the current processing is invalidated, any current computation is canceled, and the FilmImageProvider returns an empty image.
* Additionally, the ParameterManager emits the `updateImage` signal, which tells `Edit.qml` to request a new image. The `topImage` is the one that submits the request by having its `source` changed.
* `Edit.qml` requests the quick preview, which then begins computation.
* When an image pipeline completes a pipeline stage and requests the parameters for the next stage, validity of the computation is updated in case computation gets restarted.
* When the quick preview completes computation, the image is returned to `Edit.qml`, where the image gets immediately displayed as two `Image`s in the same location.
* Then, the `Image` on top, `topImage`, requests the full-resolution image, disappearing (this is why the bottom one exists) and waits for the full-res image to load.
* The full-res pipeline runs, and returns the full-size image. It steals the demosaic data (and in the future, NR results) from the low-res pipeline to speed things up.
* When the `topImage` gets the new result, the `bottomImage` also requests it, displays it immediately (it's cached), and then tells the FilmImageProvider to update the thumbnail.

## The Flow of Info When Selecting an Image

New images are selected from the queue. Here's how it works when preloading is happening.

* The queue either knows (from double-clicking) or looks up from the QueueModel (from pressing right or left) what the new image index is.
* Then it finds whether this is to the left or the right of the current image on the queue, and looks up that image ID, for preloading.
* Then it calls the `prepareShuffle` method of FilmImageProvider to inform it what the next image is.
* Finally (last part in the queue) the queue tells the main ParameterManager to select the next image with `selectImage`.
* When an image gets selected, the ParameterManager loads the parameters from the database, and then via `paramChangeWrapper` tells the QML to request a new image.
* The first thing that the QML does is actually load the JPEG thumbnail from the disk. Then it requests the quick image from the FilmImageProvider.
* When the image gets requested, the FilmImageProvider begins by shuffling data between its internal pipelines so as to maintain valid data, using `shufflePipelines`.
* The current quick pipeline's image data and parameters get swapped out to the "previous" image pipeline and parameters.
* Then, if the newly selected image either matches the former "next" image or the former "previous" image, that image data is loaded into the quick pipeline.
* The next pipeline's data gets invalidated, and the parametermanager for it gets the new image ID.
* After shuffling, the current preview ImagePipeline runs. If it was loaded from cache, it returns almost immediately thanks to the image data being fully valid. If not, it runs normally and returns.
* Now, when the QML requests the full size image, the next pipeline runs at preview resolution. As long as the user switches images after the next pipeline runs, the preview resolution will be immediately available. However, this doesn't return anything; it just keeps the data.
* After the next pipeline runs, the full size current pipeline runs and returns the full res image.

## Adding New Parameters

The key complexity in working with Filmulator has to do with handling parameters, which touch every part of the program, from the database, to the user interface, to image processing.

### Database

Parameters need to be stored in the database. When creating a new parameter, add it to `database/dbsetup.cpp`; you need to create a column in `ProcessingTable`, you *may* need to create a column in `ProfileTable`, and you need to rev up the database schema and perform modifications on old rows. This is VERY CRITICAL and you need to test both that new databases get initialized correctly and that existing data doesn't get clobbered by the schema update.

You also need to deal with how the parameters are accesssed, but that comes next...

### ParameterManager

Parameters need to be exposed to the UI via being a `Q_PROPERTY` in the ParameterManager. You need to create both the parameter and a default (that the UI doesn't write back to). You need to:

* Initialize the parameter at the creation of the object.
* Add the parameter to the appropriate pipeline stage struct.
* Assign the parameter to the struct in the appropriate `claim[pipeline stage]Params` method.
* Create the setter, ensuring that the validity is handled correctly and all the locking happens properly. If this is a binary value operated by a switch and not a slider, you need to call `writeback` to ensure it gets written to the database as soon as it's changed.
* Make sure it gets written back in `writeToDB`. This gets called when the user stops interacting with the control so that the database only gets written to once per user interaction instead of at every slider change. Ensure that the numbers align with those in `dbsetup`.
* Initialize the param in `selectImage` if it has unusual behavior, such as lens corrections which are kinda complicated. Also emit signals telling the UI that the parameters have changed.
* Initialize the default in `loadDefaults` except when this is handled in `selectImage`.
* Initialize the actual value from the database in `loadParams` and update validity accordingly.
* Make `cloneParams` copy the value from another ParameterManager and update validity accordingly.
* If a parameter is only available for some types of files, make sure it has an availability `Q_PROPERTY` and make sure `updateAvailability` manages that properly.

### ImagePipeline

This is the easy part: simply grab the parameter from the appropriate struct and use it.
