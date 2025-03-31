## About

**Gifpaper** is a lightweight utility to draw gif frames to the X root window (i.e. *GIF wallpapers*).

Some of Gifpaper's features include:

* drawing gifs onto the X root window
* playing a slideshow of gifs stored in a single directory
* cropping the gif before it gets displayed
* multihead support which replicates the gif on each monitor
* multihead support which scales and extends the gif over all monitors
* power saving mode which halts the gif if the battery is discharging
* an option to only partially cache some frames to save memory

Here is an example of gifpaper on my personal machine:

![Example-wallpaper](https://i.imgur.com/DDPtlci.gif)

### Why gifpaper?

Gifpaper is designed to minimize the performance cost of loading each frame to the root window. This
is primarily done by cacheing each frame of the gif as a Pixmap, so that at runtime the pixmap need only
be drawn to the background. The draw operation itself has also been optimized to use as few Xlib calls
as possible. Gifpaper is a **lightweight**, **performant**, and **customizable** utlility to create
GIF wallpapers.

## Installation

Clone with `git clone` and install with `make install`. It's as simple as that.

## Usage

Basic usage for gifpaper is `gifpaper path/to/gif`. Several other options can be set as well:

### Slideshow Mode

This allows several gifs to be played in a slideshow. The next gif is queued while the current gif is playing.
Slideshow mode is set using `gifpaper -s SLIDESHOW_RATE path/to/directory`, where SLIDESHOW RATE specifies the
amount of time that each gif should play before the next gif is queued.

### Cropping

Gifpaper allows you to crop the gif before it is set as the wallpaper. This can be useful when trying to extend a
single gif over multiple monitors. This option is set with `--crop 'x0 y0 x1 y1'`.

### Multihead

Gifpaper fully supports multihead with Xinerama. Currently, gifs can either be replicated onto each monitor or
extended over all monitors (while maintaining the original gif's scale). The mode is set with `--replicate` and
`--extend`, respectively.

### Power Saving Mode

Regardless of optimization, continuously drawing gif frames in X will use CPU cycles and drain battery. If this
is a concern, the `--power-save` option will halt the gif if the computer's battery is discharging.

### Memory Load

Pixmaps are quite large, since they hold the scaled-up gif frame that will be directly applied to the root window.
The `--memory-load` option can be used to avoid issues where gifs take up obnoxious amounts of memory by chosing
a percentage of frames to be only partially cached. The memory load ratio can be set between 0.0 and 1.0, where 0.0
forces all frames to only be partially cached, while 1.0 allows all frames to be fully cached into Pixmaps. Gifpaper
evenly distributes partially cached-frames over the entire gif to minimize noticable framerate drops.

## Troubleshooting

1. Running gifpaper with compton

Gifpaper couples really well with compton, since the compositor allows for real transparency for things like
terminal windows. However, when running X through an NVIDIA discrete graphics card, compton's default backend
has a bug which causes X to leak GPU memory. To fix this, you must run compton with the xrender backend, i.e.
with `compton --backend xrender ...`.

2. Gifpaper uses too much memory!

Pixmaps will inevitably become large. This may lead to performance issues on your system, especially if running X
on a graphics card. Use the --memory-load option to control the percentage of gif frames that get fully cached, as
opposed to an intermediate cache state where only part of the preparation operation still needs to be applied.

## Future updates

* a config file to store options for specific gifs
* an option to store frames as quad-tree diffs (for maximum memory optimization)
* optional antialiasing for the scale function

*Thank you!*
