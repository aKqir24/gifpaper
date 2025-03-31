#include "gifpaper.h"

int generate_frame_pattern(uint8_t *buf, float f_rate)
{
    int rate = (int)(f_rate * 100.0);

    if (!rate) {
        buf[0] = 0;
        return 1;
    }

    int buf_size = (int)(100 / gcf(rate, 100));
    int bmap_frame_count = (int)(rate / gcf(rate, 100));

    _generate_frame_pattern(buf, buf_size, bmap_frame_count);

    return buf_size;
}

void _generate_frame_pattern(uint8_t *buf, int buf_size, int count)
{
    if (!count)
        return;

    if (count % 2) {
        buf[buf_size / 2] = 1;
        count -= 1;
    }
    _generate_frame_pattern(buf, buf_size / 2, count / 2);
    _generate_frame_pattern(buf + buf_size / 2 + 1, buf_size - buf_size / 2 - 1,
                            count / 2);
}

// q-tree code will live here.
