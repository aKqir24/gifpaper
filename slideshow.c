#include "gifpaper.h"

// signal primitives
static pthread_mutex_t timer_lock;
static int timer_signal = 0;

void *timer_thread(void *args)
{
    struct timespec w = *(struct timespec *)args;
    nanosleep(&w, NULL);
    pthread_mutex_lock(&timer_lock);
    timer_signal = 1;
    pthread_mutex_unlock(&timer_lock);
}

struct timespec generate_load_projection(struct timespec start,
                                         struct timespec load_start,
                                         struct timespec end)
{
    struct timespec load_diff, diff, temp;

    load_diff = time_diff(load_start, end);
    diff = time_diff(start, end);
    temp.tv_sec = load_diff.tv_sec;
    temp.tv_nsec = (long)(0.2 * load_diff.tv_nsec);

    return time_combine(time_combine(diff, load_diff), temp);
}

int display_as_slideshow(char *dirpath, long framerate, long sliderate)
{
    // Initialize the timer thread, that alerts when gifs must swap.
    pthread_mutex_init(&timer_lock, NULL);

    // Find the paths to all gifs in the slideshow.
    SlideshowEntry *gif = load_slideshow_paths(dirpath);
    if (!gif) {
        printf("Error: gif directory is empty.\n");
        return -1;
    } else if (gif->next == gif) {
        display_as_gif(gif->path, framerate);
        return 0;
    }

    Frame *c = NULL;   // The gif which is actively being displayed.
    Frame *n = NULL;   // The next gif to be displayed, which we are preparing.
    Frame *n_head;     // The head frame of the next gif to be displayed.
    Frame *n_p = NULL; // The prior frame that was prepared for the next gif.
    gd_GIF *n_hdl = NULL; // Handle to the gif object of the next gif.
    int n_idx;            // Index of the frame in the next gif.
    Frame *p = NULL;      // The last gif that was displayed in the slideshow.

    // Prepare the first gif to be displayed upfront.
    SlideshowEntry *gif_head = gif;
    while (1) {
        c = load_images_to_list(gif->path);
        gif = gif->next;
        if (c)
            break;
        else
            printf("Warning: gif at %s was not readable.\n", gif->path);
        if (!c && (gif == gif_head)) {
            printf("Error: No files in the directory were readable gifs.\n");
            return -1;
        }
    }

    struct timespec w_frame, w_actual;
    w_frame.tv_sec = 0;
    w_frame.tv_nsec = 999999999 / framerate; // 1 second divided by frame rate

    struct timespec start, end, diff;
    struct timespec proj, load_start;

    int frames_processed, file_count;
    frames_processed = 0;
    file_count = 0; // to shut up the warning

    struct timespec w_slideshow;
    w_slideshow.tv_sec = sliderate;
    w_slideshow.tv_nsec = 0;

    uint8_t hf_pattern[100] = {0};
    int hf_psize = 0;
    if (hybrid_frame_mode)
        hf_psize = generate_frame_pattern(hf_pattern, hybrid_frame_rate);

    pthread_t tid;
    pthread_create(&tid, NULL, timer_thread, &w_slideshow);

    while (True) {
        pthread_mutex_lock(&timer_lock);
        if (timer_signal) {
            // Finish queueing next gif if not yet completed in time.
            if (frames_processed < file_count)
                printf("Delaying play to finish queueing next gif...\n");
            while (gd_get_frame(n_hdl) > 0) {

                // Determine how the frame should be stored.
                if (hybrid_frame_mode) {
                    if (hf_pattern[n_idx % hf_psize]) {
                        n->type = PIXMAP_FRAME;
                    } else {
                        n->type = BUFFER_FRAME;
                    }
                } else {
                    n->type = PIXMAP_FRAME;
                }

                append_image_to_list(n_hdl, n);

                n->next = (Frame *)malloc(sizeof(Frame));
                n->prev = n_p;
                n_p = n;
                n = n->next;
                n_idx += 1;
            }

            if (n_p) {
                free(n);
                n = n_p;
            }
            n->next = n_head;
            n_head->prev = n;

            // Swap out the gifs.
            p = c;
            c = n_head;
            n = NULL;
            gif = gif->next;

            // Reset the timer.
            timer_signal = 0;
            pthread_create(&tid, NULL, timer_thread, &w_slideshow);
        }
        pthread_mutex_unlock(&timer_lock);

        // Set the background to the next frame of the
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        check_power_conditions();
        _set_background(c, p);
        if (p) {
            clean_gif_frames(p);
            p = NULL;
        }
        c = c->next;

        if (!n) {
            if (n_hdl)
                gd_close_gif(n_hdl);
            n_hdl = NULL;
            while (!n_hdl) {
                n_hdl = gd_open_gif(gif->path);
                if (!n_hdl)
                    printf("Warning: gif at %s was not readable.\n", gif->path);
                gif = gif->next;
            }

            n = (Frame *)malloc(sizeof(Frame));
            n_head = n;
            n_idx = 0;

            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        } else {
            // Write frames to the circular list
            while (gd_get_frame(n_hdl) > 0) {
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &load_start);

                // Determine how the frame should be stored.
                if (hybrid_frame_mode) {
                    if (hf_pattern[n_idx % hf_psize]) {
                        n->type = PIXMAP_FRAME;
                    } else {
                        n->type = BUFFER_FRAME;
                    }
                } else {
                    n->type = PIXMAP_FRAME;
                }

                append_image_to_list(n_hdl, n);

                n->next = (Frame *)malloc(sizeof(Frame));
                n->prev = n_p;
                n_p = n;
                n = n->next;
                n_idx += 1;

                // Make a projection to see if we have enough time for another
                // frame.
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                proj = generate_load_projection(start, load_start, end);
                if (proj.tv_sec > 0 || proj.tv_nsec >= w_frame.tv_nsec) {
                    break;
                }
            }
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        }

        diff = time_diff(start, end);

        // Wait out the remaining time until the next frame is needed.
        if (diff.tv_sec > 0 || diff.tv_nsec >= w_frame.tv_nsec) {
            printf("Timing failure! Expect a choppy frame...\n");
            continue;
        } else {
            w_actual.tv_sec = 0;
            w_actual.tv_nsec = w_frame.tv_nsec - diff.tv_nsec;
        }
        nanosleep(&w_actual, NULL);
    }
}
