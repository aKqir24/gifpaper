#include "gifpaper.h"

struct timespec time_diff(struct timespec start, struct timespec end) {
  struct timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp;
}

struct timespec time_combine(struct timespec a, struct timespec b) {
  struct timespec temp;
  temp.tv_sec = a.tv_sec + b.tv_sec;
  if (a.tv_nsec + b.tv_nsec > 999999999) {
    temp.tv_sec += 1;
  }
  temp.tv_nsec = (a.tv_nsec + b.tv_nsec) % 1000000000;
  return temp;
}

int gcf(int a, int b) {
  int gcf;
  for (int i = 1; i <= a && i <= b; ++i) {
    if ((a % i) == 0 && (b % i) == 0)
      gcf = i;
  }

  return gcf;
}
