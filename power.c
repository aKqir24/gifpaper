#include "gifpaper.h"

/**
 * Checks if the machine is currently charging. Returns 0 if discharging, 1 if
 * charging, and -1 on error.
 */

int detect_charging()
{
    FILE *fp = popen("acpi", "r");

    char _resp[100] = {0};
    fscanf(fp, "%s", _resp);
    fscanf(fp, "%s", _resp);

    char resp[100] = {0};
    fscanf(fp, "%s", resp);
    pclose(fp);

    if (!strlen(resp)) {
        return -1;
    } else if (!strcmp(resp, "Discharging,")) {
        return 0;
    } else {
        return 1;
    }
}

int check_power_conditions()
{
    struct timespec w;
    w.tv_sec = 1;
    w.tv_nsec = 0;

    while (!((battery_saver && detect_charging()) || !battery_saver)) {
        nanosleep(&w, NULL);
    }
}
