/* wallpaper.c

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "gifpaper.h"

Display *disp = NULL;
Visual *vis = NULL;
Screen *scr = NULL;
Colormap cm;
int depth;
XContext xid_context = 0;
Window root = 0;

void init_x(void)
{
    // XInitThreads(); // must always be the first call
    disp = XOpenDisplay(NULL);
    if (!disp)
        return;
    XSetCloseDownMode(disp, RetainPermanent);
    vis = DefaultVisual(disp, DefaultScreen(disp));
    depth = DefaultDepth(disp, DefaultScreen(disp));
    cm = DefaultColormap(disp, DefaultScreen(disp));
    root = RootWindow(disp, DefaultScreen(disp));
    scr = ScreenOfDisplay(disp, DefaultScreen(disp));
    xid_context = XUniqueContext();

    return;
}

#ifdef HAVE_LIBXINERAMA
XineramaScreenInfo *xinerama_screens = NULL;
int xinerama_screen = 0;
int num_xinerama_screens = 0;
#endif /* HAVE_LIBXINERAMA */

#ifdef HAVE_LIBXINERAMA
void init_xinerama(void)
{
    if (XineramaIsActive(disp)) {
        int major, minor, px, py, i;

        Window dw;
        int di;
        unsigned int du;

        XineramaQueryVersion(disp, &major, &minor);
        xinerama_screens = XineramaQueryScreens(disp, &num_xinerama_screens);

        xinerama_screen = 0;
        XQueryPointer(disp, root, &dw, &dw, &px, &py, &di, &di, &du);
        for (i = 0; i < num_xinerama_screens; i++) {
            if (XY_IN_RECT(px, py, xinerama_screens[i].x_org,
                           xinerama_screens[i].y_org, xinerama_screens[i].width,
                           xinerama_screens[i].height)) {
                xinerama_screen = i;
                break;
            }
        }
    }
}
#endif /* HAVE_LIBXINERAMA */

Buffmap generate_bmap(uint8_t *buffer, int srcW, int srcH)
{
    Buffmap ret;
    ret.buf = buffer;
    ret.w = srcW;
    ret.h = srcH;

    return ret;
}

Pixmap generate_pmap(uint8_t *buffer, int srcW, int srcH)
{
    switch (display_mode) {
    case DISPLAY_MODE_REPLICATE:
        return generate_pmap_replicate(buffer, srcW, srcH);
    case DISPLAY_MODE_EXTEND:
        return generate_pmap_extend(buffer, srcW, srcH);
    default:
        return generate_pmap_replicate(buffer, srcW, srcH);
    }
}

Pixmap generate_pmap_replicate(uint8_t *buffer, int srcW, int srcH)
{
    Pixmap pmap;
    pmap = XCreatePixmap(disp, root, scr->width, scr->height, depth);

    uint8_t *scaled = (uint8_t *)malloc(scr->width * scr->height * 4);

#ifdef HAVE_LIBXINERAMA
    for (int i = 0; i < num_xinerama_screens; i++) {
        scaled = scale_to_screen(scaled, buffer, srcW, 0, 0, srcW, srcH, i);
    }
#else
    scaled = scale_to_screen(scaled, buffer, srcW, 0, 0, srcW, srcH, 0);
#endif /* HAVE_LIBXINERAMA */

    _generate_pmap(pmap, scaled, 0, 0, scr->width, scr->height);

    return pmap;
}

Pixmap generate_pmap_extend(uint8_t *buffer, int srcW, int srcH)
{
    Pixmap pmap;
    pmap = XCreatePixmap(disp, root, scr->width, scr->height, depth);

    int im_w, im_h;
    im_w = srcW;
    im_h = srcH;
    double scale = scr->width > scr->height
                       ? ((double)im_w) / ((double)scr->width)
                       : ((double)im_h) / ((double)scr->height);

    uint8_t *scaled = (uint8_t *)malloc(scr->width * scr->height * 4);

    for (int i = 0; i < num_xinerama_screens; i++) {
        int diff_x, diff_y;
        diff_x = im_w - ((double)scr->width * scale);
        diff_y = im_h - ((double)scr->height * scale);

        int sub_x, sub_y, sub_w, sub_h = 0;
        sub_x = ((double)xinerama_screens[i].x_org * scale) + diff_x;
        sub_y = ((double)xinerama_screens[i].y_org * scale) + diff_y;
        sub_w = ((double)xinerama_screens[i].width * scale);
        sub_h = ((double)xinerama_screens[i].height * scale);

        uint8_t *cropped;
        cropped = crop(buffer, srcW, srcH, sub_x, sub_y, sub_w, sub_h);

        scaled = scale_to_screen(scaled, cropped, sub_w, 0, 0, sub_w, sub_h, i);
        free(cropped);
    }

    _generate_pmap(pmap, scaled, 0, 0, scr->width, scr->height);

    free(scaled);

    return pmap;
}

Pixmap _generate_pmap(Pixmap pmap, uint8_t *buffer, int x, int y, int w, int h)
{
    GC gc = XCreateGC(disp, root, 0, 0);
    XImage *img = XCreateImage(disp, CopyFromParent, depth, ZPixmap, 0,
                               (char *)buffer, w, h, 32, 0);
    XPutImage(disp, pmap, gc, img, 0, 0, x, y, w, h);
    XFreeGC(disp, gc);

    return pmap;
}

void clear_pmap(Pixmap pmap)
{
    XFreePixmap(disp, pmap);
}

Pixmap get_pixmap(Frame *frame)
{
    switch (frame->type) {
    case PIXMAP_FRAME:
        return frame->pmap;
    case BUFFER_FRAME:
        if (frame->bmap.active) {
            return frame->bmap.pmap;
        } else {
            return -1;
        }
    default:
        return frame->pmap;
    }
}

int set_background(Frame *frame)
{
    return _set_background(frame, NULL);
}

int _set_background(Frame *frame, Frame *frame_last)
{
    int ret;
    Pixmap pmap;

    switch (frame->type) {
    case PIXMAP_FRAME:
        ret = draw_pmap_to_background(frame, frame_last, frame->pmap);
        break;
    case BUFFER_FRAME:
        pmap = generate_pmap(frame->bmap.buf, frame->bmap.w, frame->bmap.h);
        ret = draw_pmap_to_background(frame, frame_last, pmap);
        frame->bmap.active = 1;
        frame->bmap.pmap = pmap;
        break;
    default:
        ret = draw_pmap_to_background(frame, frame_last, frame->pmap);
        break;
    }

    Frame *p = frame->prev;
    if (!p)
        return ret;

    switch (p->type) {
    case PIXMAP_FRAME:
        break;
    case BUFFER_FRAME:
        if (p->bmap.active && frame != p) {
            p->bmap.active = 0;
            XFreePixmap(disp, p->bmap.pmap);
        }
        break;
    default:
        break;
    }

    return ret;
}

int draw_pmap_to_background(Frame *frame, Frame *prev, Pixmap pmap)
{
    Frame *c = frame;

    Atom prop_root, prop_esetroot, type;
    int format;
    unsigned long length, after;
    unsigned char *data_root = NULL, *data_esetroot = NULL;

    prop_root = XInternAtom(disp, "_XROOTPMAP_ID", True);
    prop_esetroot = XInternAtom(disp, "ESETROOT_PMAP_ID", True);

    if (prop_root != None && prop_esetroot != None) {
        XGetWindowProperty(disp, root, prop_root, 0L, 1L, False,
                           AnyPropertyType, &type, &format, &length, &after,
                           &data_root);
        if (type == XA_PIXMAP) {
            XGetWindowProperty(disp, root, prop_esetroot, 0L, 1L, False,
                               AnyPropertyType, &type, &format, &length, &after,
                               &data_esetroot);
            if (data_root && data_esetroot) {
                if (type == XA_PIXMAP &&
                    *((Pixmap *)data_root) == *((Pixmap *)data_esetroot)) {
                    // Looks like someone owns the root window. Let's see who it
                    // is! printf("Checking to see who owns the root window...
                    // \n");

                    Pixmap target_pmap = *((Pixmap *)data_root);
                    int kill = 1;
                    while (1) {
                        if (target_pmap == get_pixmap(c)) {
                            kill = 0;
                            break;
                        }
                        c = c->next;
                        if (c == NULL || c == frame) {
                            break;
                        }
                    }

                    if (prev != NULL) {
                        c = prev;
                        while (1) {
                            if (target_pmap == get_pixmap(c)) {
                                kill = 0;
                                break;
                            }
                            c = c->next;
                            if (c == NULL || c == frame) {
                                break;
                            }
                        }
                    }

                    if (kill) {
                        XKillClient(disp, target_pmap);
                    }
                }
            }
        }
    }

    if (data_root)
        XFree(data_root);
    if (data_esetroot)
        XFree(data_esetroot);

    /* This will locate the property, creating it if it doesn't exist */
    prop_root = XInternAtom(disp, "_XROOTPMAP_ID", False);
    prop_esetroot = XInternAtom(disp, "ESETROOT_PMAP_ID", False);

    if (prop_root == None || prop_esetroot == None) {
        fprintf(stderr, "error: Creation of display pixmap properties failed.");
        return 1;
    }

    XChangeProperty(disp, root, prop_root, XA_PIXMAP, 32, PropModeReplace,
                    (unsigned char *)&pmap, 1);
    XChangeProperty(disp, root, prop_esetroot, XA_PIXMAP, 32, PropModeReplace,
                    (unsigned char *)&pmap, 1);

    XSetWindowBackgroundPixmap(disp, root, pmap);
    XClearWindow(disp, root);
    XFlush(disp);

    return 0;
}
