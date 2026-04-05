filmulator-gui
==============

A Qt Quick GUI adaptation of Filmulator --- a film emulator with all of the positives and none of the negatives.

Filmulator accepts raw files from cameras and simulates the development of film as if exposed to the same light as the camera's sensor. For various reasons, this inherently brings about several benefits:

* Large bright regions become darker, compressing the output dynamic range.
* Small bright regions make their surroundings darker, enhancing local contrast.
* In bright regions, saturation is enhanced, helping retain color in blue skies, brighter skin tones, and sunsets.
* In extremely saturated regions, the brightness is attenuated, helping retain detail e.g. in flowers.

The program's design ideology is to have the best tool for any job, and only that one tool. The tradeoff here is a slight decrease in flexibility, but gaining a greatly simplified and streamlined user interface.

![Filmulate overview](http://i.imgur.com/hXIHUkd.png)

# Building Filmulator

## Building with vcpkg (Recommended)

**vcpkg** is the recommended way to build Filmulator. It automatically manages all dependencies across Windows, macOS, and Linux.

### Prerequisites

1. Install vcpkg:
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh  # or bootstrap-vcpkg.bat on Windows
```

2. Install system dependencies (for building some packages in vcpkg):
- **macOS**: `brew install pkg-config ninja libomp autoconf autoconf-archive automake libtool`
- **Linux**: `sudo apt install pkg-config ninja-build`

3. Set the `VCPKG_ROOT` environment variable:
```bash
export VCPKG_ROOT=/path/to/vcpkg  # Add to your shell profile
```

### Build Steps

```bash
cd filmulator-gui/filmulator-gui
cmake --preset relwithdebinfo
cmake --build build/relwithdebinfo -j$(nproc)
```

The first build will take time as vcpkg downloads and compiles dependencies. Subsequent builds use cached packages.

### Available Presets

- `debug` - Debug build with symbols
- `release` - Optimized release build  
- `relwithdebinfo` - Release with debug info (recommended for development)

---

## Legacy Build Methods

### Dependencies (Manual Installation)

If not using vcpkg, you need these libraries installed:
```
libtiff, libgomp, libexiv2, libjpeg, libraw, librtprocess, liblensfun, libcurl, libarchive
```
And Qt 5.15 or newer.

We highly encourage you to compile libraw yourself to ensure you have support for recent cameras.

### Building with CMake (Linux)

Create a build directory and navigate to inside that directory.

Run `cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/usr ..` If you are using a binary release of Qt from them, use -DCMAKE\_PREFIX\_PATH=\[path to the qt dir and version and arch\] as an argument.

Then run `make` and run `make install` as root.

### Building on macOS (Legacy)

You'll need to know the locations of a couple things in order to build this. They're not hard to find, just use Finder's search functionality to find them. Wherever they are, we need the real dynamic libraries (.dylib), not the symbolic links that point to them. That's important! If you find a symbolic link, follow it to get the real dynamic library.  We need:

libomp.dylib: it should be somewhere like /opt/local/lib/libomp.dylib. If you installed from homebrew, that's probably where it is. Replace wherever it is into `-DOpenMP_libomp_LIBRARY=` and `-fopenmp` below.

libarchive.dylib: If you installed from homebrew, it probably needs to be /usr/local/Cellar/libarchive/3.4.3/include like below. Put this path into `-DLibArchive_INCLUDE_DIR`.

librtprocess: This needs to point towards the .dylib file for librtprocess. If you installed librtprocess from source, it's probably in /opt/local/lib/librtprocess.0.0.1.dylib like below. Wherever it is, put it into `-Dlibrtprocess_dylib`.

Qt: If you installed this from homebrew, it's probably at /usr/local/Cellar/qt/5.13.1/. Wherever it is, put it in the `export QT=` command below.


Once you have all those figured out, the following commands, edited according to your locations detailed above, should build the Filmulator application on macOS. 

1. `cd ~/filmulator-gui/filmulator-gui`
2. `mkdir build && cd build`
3. `export QT=/usr/local/Cellar/qt/5.13.1`
4. `cmake -DCMAKE_BUILD_TYPE="RELEASE" -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_CXX_FLAGS=-I/opt/local/include -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp /opt/local/lib/libomp/libomp.dylib -I/opt/local/include" -DOpenMP_CXX_LIB_NAMES="libomp" -DOpenMP_libomp_LIBRARY=/opt/local/lib/libomp/libomp.dylib -DCMAKE_INSTALL_PREFIX=/opt/local -DCMAKE_SHARED_LINKER_FLAGS=-L/opt/local/lib -DCMAKE_PREFIX_PATH=$(echo $QT/lib/cmake/* | sed -Ee 's$ $;$g') -G "Unix Makefiles" -DCMAKE_VERBOSE_MAKEFILE=1 -DLibArchive_INCLUDE_DIR=/usr/local/Cellar/libarchive/3.4.3/include -Dlibrtprocess_dylib=/opt/local/lib/librtprocess.0.0.1.dylib ..`
5. `make -j8 install`

# Using Filmulator

First, import your photos. You can select directories to import, or individual files. You can also select whether to leave the photos where they are, or to copy them into a directory structure based upon the date captured, in both the Destination Directory and the Backup Directory. These can be sorted correctly by setting the Camera UTC Offset to the timezone you set the camera's clock in, and the Local UTC Offset to the timezone where the photo was captured. Select the source directory (your card, or a directory containing your photos), and pressing the `Import` button will begin pulling the photos into the database and writing out the source directory.

If you choose to enqueue imported photos before you import, you can skip the `Organize` tab, but you should check it out anyway.

Next, go to the `Organize` tab, where you can select photos to enqueue. The calendar and the date histogram let you select photos from a given day, while the timezone slider lets you adjust how to 'slice' the day: it always displays from midnight to midnight.

To enqueue photos, double-click on them in the grid view in the `Organize` tab. To remove them from the queue, right-click on the queue entry and select `Remove from queue`. From either the `Organize` tab or from the queue, you can rate images from zero through five, and you can filter the organize tab to show only photos above a certain rating.

At this point, go to the `Filmulate` tab, which lets you process your photos. Double-click on a photo in the queue to load it through the Filmulation process, and set the sliders to your taste.

The tools are arranged in pipeline order: the processing they control occurs from top to bottom. Thus, we advise you mostly work top-down through the tools.

Tools and features of interest:
* In-pipeline mini histograms: They display the distribution of the pixel brightnesses right before and right after the filmulation algorithm. Use the first one to adjust exposure compensation and highlight recovery, and use the second one to adjust the output clipping.
* Highlight Recovery: If nothing is clipped, leave highlight recovery at 0. If there is clipping, skip right to 3. If there are clipped skin tones, 9 tends to work well, but it's really slow. 1 and 2 are useless, while 4-8 are weighted blends of 3 and 9.
* Film Area: Here's where it gets good. The program defaults to an approximate equivalent of 36x24 film. If you reduce the film size, the algorithm accentuates brightness in larger regions. If you increase the film size, the whole image becomes slightly flatter (reduced dynamic range), but it emphasizes smaller details.
* Drama: This is effectively the strength of the 'filminess'. Higher values reduce the dynamic range of the output; you'll see the "post-filmulator" histogram shrink towards the left. With the default film area, feel free to turn this all the way up to 100 if you need to bright down the highlights, but with large-format equivalent film area it starts to get too flat-looking past 50.

If you want the UI to appear larger on a high-pixel density display, use the User Interface Scale slider in the `Settings` tab to set a desired scale, save the setting, and then restart the program. While it cannot automatically read your display pixel density, this setting enables otherwise full HiDPI support.

# Testing

## E2E Tests

The project uses Python-based E2E tests for the RAW pipeline. We use [**uv**](https://github.com/astral-sh/uv) to manage Python dependencies and the virtual environment.

### Prerequisites

Install `uv`:
```bash
curl -LsSf https://astral.ml/uv/install.sh | sh
```

### Running Tests

To run the RAW pipeline E2E test:
```bash
uv run python tests/e2e/test_raw_pipeline.py
```

To preserve the output JPEG for analysis (e.g., if a test fails):
```bash
uv run python tests/e2e/test_raw_pipeline.py --keep-output
```

### Analyzing Differences

If a test fails, you can use the diagnostic notebook to inspect pixel-wise differences:
```bash
uv run jupyter notebook tests/e2e/analyze_diff.ipynb
```


# Status

If told to make a version number for it right now, I'd put it as 0.9.0.

The editing functionality is nearly complete, missing only noise reduction. The library functionality may still be expanded and massaged, however.

But in the meantime, feel free to play around. Report any new bugs or suggestions you may have either on the [subreddit](https://www.reddit.com/r/Filmulator/) or on [the pixls.us forum](https://discuss.pixls.us/c/software/filmulator)!
