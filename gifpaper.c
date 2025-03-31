#include "gifpaper.h"

int display_as_gif(char *gifpath, long framerate)
{
    Frame *head = load_images_to_list(gifpath);
    if (head == NULL) {
        printf("Error: the gif was not readable.\n");
        return -1;
    }

    struct timespec start, end, diff;
    struct timespec w_frame, w_actual;
    w_frame.tv_sec = 0;
    w_frame.tv_nsec = 999999999 / framerate; // 1 second divided by frame rate

    while (True) {
        check_power_conditions();
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

        set_background(head);
        head = head->next;

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        diff = time_diff(start, end);
        if (diff.tv_sec > 0 || diff.tv_nsec >= w_frame.tv_nsec) {
            printf("Timing failure! Expect a choppy frame...\n");
        } else {
            w_actual.tv_sec = 0;
            w_actual.tv_nsec = w_frame.tv_nsec - diff.tv_nsec;
            nanosleep(&w_actual, NULL);
        }
    }
}

// display_as_slideshow lives in slideshow.c

static struct option long_options[] = {
    {"crop", required_argument, NULL, 'c'},
    {"replicate", no_argument, NULL, 'r'},
    {"extend", no_argument, NULL, 'e'},
    {"help", no_argument, NULL, 'h'},
    {"power-save", no_argument, NULL, 'p'},
    {"memory-load", required_argument, NULL, 'l'},
    {NULL, 0, NULL, 0}};

const char *help_string =
    "Gifpaper: a tool for drawing gifs to the X root window (i.e., the wallpaper).\n\
Syntax: gifpaper [options] wallpaper.gif \n\
\n\
Options: \n\
-f FRAMERATE              Set the framerate of the gif. \n\
-s SLIDESHOW_RATE         Slideshow mode. Must provide a directory with gifs. \n\
-h, --help                Show this help menu. \n\
    --crop 'x0 y0 x1 y1'  Crop gif to the dimensions speficied by the coordinates. \n\
    --power-save          Only run the gif if the battery is charging. \n\
    --memory-load LOAD    Dictate the ratio (from 0.0 to 1.0) of frames that should be fully cached, versus partially cached. \n\
\n\
Multihead Options : \n\
    --extend              Extend the gif, scaled, across all monitors. \n\
    --replicate           Replicate the gif across each monitor. \n\
\n\
Please report bugs to <remykaldawy@gmail.com>.\n";

// Globals to indicate crop mode and crop parameters.
int crop_mode = 0; // todo: rename to use mode syntax
int crop_params[4] = {0};
// Global to indicate battery saving mode.
int battery_saver = 0; // todo: rename to use mode syntax
// Global to indicate multihead display mode.
int display_mode = 0;
// Global to indicate hybrid frame caching mode.
int hybrid_frame_mode = 0;
float hybrid_frame_rate = 1.0;

int main(int argc, char **argv)
{
    long framerate = 12;

    int slideshow_mode = 0;
    int sliderate = 180;

    int opt;
    char *endptr;

    while ((opt = getopt_long(argc, argv, "s:f:hc:", long_options, NULL)) !=
           -1) {
        switch (opt) {
        case 'f':
            framerate = strtol(optarg, &endptr, 10);
            if (*optarg == '\0' || *endptr != '\0') {
                printf("Error: framerate argument not an integer.\n");
                return -1;
            }
            if (framerate <= 0 || framerate > 60) {
                printf("Error: Framerate must be between 1Hz and 60Hz.\n");
                return -1;
            }
            break;
        case 's':
            slideshow_mode = 1;
            sliderate = strtol(optarg, &endptr, 10);
            if (*optarg == '\0' || *endptr != '\0') {
                printf(
                    "Error: the slideshow rate argument is not an integer.\n");
                return -1;
            }
            if (sliderate <= 30) {
                printf("Warning: fast slideshow rates may incur performance "
                       "costs and "
                       "choppiness.\n");
            }
            break;
        case 'c':
            crop_mode = 1;
            char *num_str = strtok(optarg, " ");
            int i;
            for (i = 0; i < 4 && num_str != NULL; i++) {
                crop_params[i] = strtol(num_str, &endptr, 10);
                if (*num_str == '\0' || *endptr != '\0') {
                    printf(
                        "Error: crop option needs four integer parameters.\n");
                    return -1;
                }
                num_str = strtok(NULL, " ");
            }
            if (i < 4) {
                printf("Usage: --crop 'x0 y0 x1 y1'.");
                return -1;
            }
            break;
        case 'r':
            display_mode = DISPLAY_MODE_REPLICATE;
            break;
        case 'e':
            display_mode = DISPLAY_MODE_EXTEND;
            break;
        case 'h':
            printf("%s", help_string);
            return 0;
        case 'p':
            if (detect_charging() < 0) {
                printf("Warning: cannot use battery saving mode.\n");
            } else {
                battery_saver = 1;
            }
            break;
        case 'l':
            hybrid_frame_mode = 1;
            hybrid_frame_rate = strtof(optarg, NULL);
            if (hybrid_frame_rate > 1.0 || hybrid_frame_rate < 0.0) {
                printf("Error: memory load must be between 0.0 and 1.0.\n");
                return -1;
            }
            break;
        default:
            printf("Error: invalid option at '%s'\n", argv[optind]);
            return -1;
        }
    }

    if (optind >= argc) {
        printf("Error: no gif or directory specified.\n");
        return -1;
    }
    char *gifpath = argv[optind];

    init_x();
    init_xinerama();

    if (slideshow_mode) {
        display_as_slideshow(gifpath, framerate, sliderate);
    } else {
        display_as_gif(gifpath, framerate);
    }
}
