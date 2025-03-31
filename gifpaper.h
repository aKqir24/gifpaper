#ifndef WALLPAPER_H
#define WALLPAPER_H

#define _GNU_SOURCE

#include <X11/Intrinsic.h> /* Xlib, Xutil, Xresource, Xfuncproto */
#include <X11/Xatom.h>
#include <X11/Xfuncproto.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#ifdef HAVE_LIBXINERAMA
#include <X11/X.h>
#include <X11/extensions/Xinerama.h>
#endif /* HAVE_LIBXINERAMA */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "gifdec.h"

#define XY_IN_RECT(x, y, rx, ry, rw, rh)                                       \
  (((x) >= (rx)) && ((y) >= (ry)) && ((x) < ((rx) + (rw))) &&                  \
   ((y) < ((ry) + (rh))))

typedef struct Buffmap {
  uint8_t *buf;
  int w;
  int h;

  // Used to maintain a reference when on display
  uint8_t active;
  Pixmap pmap;

} Buffmap;

#define PIXMAP_FRAME 0 // best runtime performance
#define BUFFER_FRAME 1 // average memory and runtime performance
#define QTREE_FRAME 2  // best memory usage, poor runtime performance

typedef struct Frame {
  int type;
  union {
    Buffmap bmap;
    Pixmap pmap;
  };
  struct Frame *next;
  struct Frame *prev;
} Frame;

typedef struct SlideshowEntry {
  char path[200];
  struct SlideshowEntry *next;
} SlideshowEntry;

extern Window ipc_win;
extern Atom ipc_atom;

extern Display *disp;
extern Visual *vis;
extern Screen *scr;
extern Colormap cm;
extern int depth;
extern XContext xid_context;
extern Window root;

extern XineramaScreenInfo *xinerama_screens;
extern int xinerama_screen;
extern int num_xinerama_screens;

// Globals to indicate crop mode and crop parameters.
extern int crop_mode;
extern int crop_params[];
// Global to indicate battery saving mode.
extern int battery_saver;
// Global to indicate multihead display mode.
extern int display_mode;
// Global to indicate hybrid frame caching mode.
extern int hybrid_frame_mode;
extern float hybrid_frame_rate;

// Define new display modes as needed here.
#define DISPLAY_MODE_REPLICATE 1
#define DISPLAY_MODE_EXTEND 2

Frame *append_image_to_list(gd_GIF *gif, Frame *c);
Frame *load_images_to_list(char *gifpath);

// Slideshow mode functions.
SlideshowEntry *load_slideshow_paths(char *gifpath);
void *slideshow_gif_thread(void *args);

int display_as_gif(char *gifpath, long framerate);
int display_as_slideshow(char *dirpath, long framerate, long sliderate);
void clear_frame(Frame *c);
void clean_gif_frames(Frame *head);

// Power functions.
int check_power_conditions();
int detect_charging();

// Image manipulation functions.
uint8_t *scale_to_screen(unsigned char *dst, unsigned char *src, int srcWidth,
                         int srcX, int srcY, int srcW, int srcH, int i);
void scale(unsigned char *dst, int dstWidth, int dstX, int dstY, int dstW,
           int dstH, unsigned char *src, int srcWidth, int srcX, int srcY,
           int srcW, int srcH);
uint8_t *crop(unsigned char *src, int srcW, int srcH, int subX, int subY,
              int subW, int subH);
int count_frames_in_gif(char *gifpath);

// Memory mode functions.
int generate_frame_pattern(uint8_t *buf, float f_rate);
void _generate_frame_pattern(uint8_t *buf, int buf_size, int count);

// Utility functions.
struct timespec time_diff(struct timespec start, struct timespec end);
struct timespec time_combine(struct timespec a, struct timespec b);
int gcf(int a, int b);

_XFUNCPROTOBEGIN

extern void init_x(void);
extern void init_xinerama(void);

Buffmap generate_bmap(uint8_t *buffer, int srcW, int srcH);

extern Pixmap generate_pmap(uint8_t *buffer, int srcW, int srcH);
extern Pixmap generate_pmap_replicate(uint8_t *buffer, int srcW, int srcH);
extern Pixmap generate_pmap_extend(uint8_t *buffer, int srcW, int srcH);

Pixmap _generate_pmap(Pixmap pmap, uint8_t *buffer, int x, int y, int w, int h);
void clear_pmap(Pixmap pmap);

extern int set_background(Frame *frame);
extern int _set_background(Frame *frame, Frame *prev);
extern int draw_pmap_to_background(Frame *frame, Frame *prev, Pixmap pmap);

_XFUNCPROTOEND

#endif
