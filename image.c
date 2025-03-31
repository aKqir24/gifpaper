#include "gifpaper.h"

/**
 * A set of functions for transforming gif frames and loading them into a
 * linked list of X11 Pixmaps. Mainly used for preparing the gif to be a
 * wallpaper.
 */

/**
 * Scales a gif frame to the size of the screen. Returns a freshly malloc'd
 * buffer with the pixels, in BRGA. Note that gifdec provides gifs in RGB.
 *
 * todo: implement antialiasing
 */

uint8_t *scale_to_screen(unsigned char *dst, unsigned char *src, int srcWidth,
                         int srcX, int srcY, int srcW, int srcH, int i)
{
#ifdef HAVE_LIBXINERAMA
    scale(dst, scr->width, xinerama_screens[i].x_org, xinerama_screens[i].y_org,
          xinerama_screens[i].width, xinerama_screens[i].height, src, srcWidth,
          srcX, srcY, srcW, srcH);
#else
    scale(dst, scr->width, 0, 0, scr->width, scr->height, src, srcWidth, srcX,
          srcY, srcW, srcH);
#endif /* HAVE_LIBXINERAMA */

    return dst;
}

/**
 * Scale a gif frame to a new specified size. X11 accepts buffered data in BGRA
 * format, while gifdec provides frames in RGB; thus, the scaling operation also
 * translates the frame data.
 *
 * todo: implement antialiasing
 */

void scale(unsigned char *dst, int dstWidth, int dstX, int dstY, int dstW,
           int dstH, unsigned char *src, int srcWidth, int srcX, int srcY,
           int srcW, int srcH)
{
    for (int y = 0; y < dstH; y++) {
        for (int x = 0; x < dstW; x++) {
            int indexDst = ((dstY + y) * dstWidth + (dstX + x)) * 4;
            int indexSrc = ((srcY + (y * srcH / dstH)) * srcWidth +
                            (srcX + (x * srcW / dstW))) *
                           3;
            dst[indexDst + 2] = src[indexSrc + 0];
            dst[indexDst + 1] = src[indexSrc + 1];
            dst[indexDst + 0] = src[indexSrc + 2];
        }
    }
}

/**
 * Crop a gif frame to a new specified size. Note that this function expects gif
 * frames in RGB format; thus, crop operations must take place before the final
 * scaling operations.
 */

uint8_t *crop(unsigned char *src, int srcW, int srcH, int subX, int subY,
              int subW, int subH)
{
    uint8_t *dst = (uint8_t *)malloc(subW * subH * 3);
    int dst_idx = 0;

    int src_idx = subY * (srcW * 3) + (subX * 3);
    for (int i = subY; i < subY + subH; i++) {
        memcpy(&dst[dst_idx], &src[src_idx], subW * 3);

        src_idx += srcW * 3;
        dst_idx += subW * 3;
    }

    return dst;
}

/**
 * Counts the number of frames in the gif, by cycling through gifdec's linked
 * list.
 */

int count_frames_in_gif(char *gifpath)
{
    int file_count;
    gd_GIF *gif = gd_open_gif(gifpath);
    for (file_count = 0; gd_get_frame(gif); file_count++) {
    }
    gd_close_gif(gif);
    return file_count;
}

void clear_frame(Frame *c)
{
    switch (c->type) {
    case PIXMAP_FRAME:
        clear_pmap(c->pmap);
        break;
    case BUFFER_FRAME:
        free(c->bmap.buf);
        if (c->bmap.active)
            clear_pmap(c->bmap.pmap);
        break;
    default:
        clear_pmap(c->pmap);
        break;
    }
}

void clean_gif_frames(Frame *head)
{
    Frame *c = head->next;
    Frame *temp;
    while (c != head) {
        temp = c->next;
        clear_frame(c);
        free(c);
        c = temp;
    }
    clear_frame(head);
    free(head);
}

/**
 * Grabs all file paths in a directory, and loads them into a linked list.
 * Used for gifpaper's slideshow mode.
 */

SlideshowEntry *load_slideshow_paths(char *gifpath)
{
    SlideshowEntry *p = NULL;
    SlideshowEntry *c = (SlideshowEntry *)malloc(sizeof(SlideshowEntry));
    SlideshowEntry *head = c;

    DIR *dirp;
    struct dirent *entry;

    dirp = opendir(gifpath);

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            snprintf(c->path, 200, "%s/%s", gifpath, entry->d_name);
            c->next = (SlideshowEntry *)malloc(sizeof(SlideshowEntry));
            p = c;
            c = c->next;
        }
    }
    free(c);

    if (!p)
        return NULL;
    p->next = head;

    return head;
}

Frame *append_image_to_list(gd_GIF *gif, Frame *c)
{
    uint8_t *buffer = (uint8_t *)malloc(gif->width * gif->height * 4);
    gd_render_frame(gif, buffer);
    // todo: handle cropping, once a config file exists
    switch (c->type) {
    case PIXMAP_FRAME:
        c->pmap = generate_pmap(buffer, gif->width, gif->height);
        free(buffer);
        break;
    case BUFFER_FRAME:
        c->bmap = generate_bmap(buffer, gif->width, gif->height);
        break;
    default:
        c->pmap = generate_pmap(buffer, gif->width, gif->height);
        free(buffer);
        break;
    }

    return c;
}

Frame *load_images_to_list(char *gifpath)
{
    Frame *p = NULL;
    Frame *c = (Frame *)malloc(sizeof(Frame));
    Frame *head = c;
    // Stub out our first entry's neighbors, since it needs to get drawn.
    c->prev = NULL;
    c->next = NULL;

    int w, h;

    // Prepare the hybrid frame variables.
    uint8_t hf_pattern[100] = {0};
    int hf_psize = 0;
    if (hybrid_frame_mode)
        hf_psize = generate_frame_pattern(hf_pattern, hybrid_frame_rate);

    gd_GIF *gif = gd_open_gif(gifpath);
    if (!gif)
        return NULL;

    for (int i = 0; gd_get_frame(gif); i++) {
        uint8_t *buffer = (uint8_t *)malloc(gif->width * gif->height * 3);
        gd_render_frame(gif, buffer);
        w = gif->width;
        h = gif->height;

        // determine how the frame should be stored
        if (hybrid_frame_mode) {
            if (hf_pattern[i % hf_psize]) {
                c->type = PIXMAP_FRAME;
            } else {
                c->type = BUFFER_FRAME;
            }
        } else {
            c->type = PIXMAP_FRAME;
        }

        // Perform the crop operation, if necessary.
        if (crop_mode) {
            uint8_t *cropped;
            cropped = crop(buffer, gif->width, gif->height, crop_params[0],
                           crop_params[1], crop_params[2], crop_params[3]);
            w = crop_params[2];
            h = crop_params[3];

            free(buffer);
            buffer = cropped;
        }

        // Store the frame's data, either as a pixmap or a buffer.
        switch (c->type) {
        case PIXMAP_FRAME:
            c->pmap = generate_pmap(buffer, w, h);
            free(buffer);
            break;
        case BUFFER_FRAME:
            c->bmap = generate_bmap(buffer, w, h);
            break;
        default:
            c->pmap = generate_pmap(buffer, w, h);
            free(buffer);
            break;
        }

        if (i == 0) { // the first frame
            set_background(c);
        }
        c->next = (Frame *)malloc(sizeof(Frame));
        c->prev = p;
        p = c;
        c = c->next;
    }

    if (p != NULL) {
        free(c);
        c = p;
    }
    c->next = head;
    head->prev = c;
    gd_close_gif(gif);

    return head;
}
