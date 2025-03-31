## Gifpaper

 A fork to ![rkaldawy's](https://gitlab.com/rkaldawy/gifpaper/-/tree/master) lightweight utility to draw gif frames to the X root window (i.e. *GIF wallpapers*) that addresses the compilation failure.

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

## Installation

````shell
  git clonehttps://github.com/aKqir24/gifpaper.git
  make install
````
## Usage

Basic usage for gifpaper is `gifpaper path/to/gif`. Several other options can be set as well:

For information about this program go to the official gitlab repo [here](https://gitlab.com/rkaldawy/gifpaper/-/tree/master).
