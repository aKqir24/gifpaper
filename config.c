#include "gifpaper.h"

/* A set of deprecated functions when imlib and ffmpeg were used to prepare the
 * gifs. This will eventually be converted into a config file handler.
 */

/*int break_gif_into_images(char *filename) {
  char path[200];
  memset(path, 0x00, 200);
  char *subpath[3] = {".config", "gifpaper", ".frames"};

  // build the path of the gif frames
  char *home = getenv("HOME");
  if (!home) {
    printf("Error: HOME directory does not exist!");
    return -1;
  }
  snprintf(path, 200, "%s", home);

  for (int i = 0; i < 3; i++) {
    snprintf(path + strlen(path), 200, "/%s", subpath[i]);

    struct stat st;
    if (stat(path, &st) == -1) {
      mkdir(path, 0700);
    }
  }
  snprintf(path + strlen(path), 200, "/%s", "frame%05d.png");

  // make sure the directory is clean
  clear_image_dir();

  char *argv[5] = {"ffmpeg", "-i", filename, path, NULL};

  pid_t child_pid = fork();
  int child_status;
  pid_t tpid = 0;

  if (child_pid == 0) {
    execvp(argv[0], argv);
    exit(0);
  } else {
    do {
      tpid = wait(&child_status);
    } while (tpid != child_pid);
  }
  return 0;
}

int clear_image_dir(void) {
  char basepath[200];
  memset(basepath, 0x00, 200);
  DIR *dirp;
  struct dirent *entry;

  char *home = getenv("HOME");
  if (!home) {
    printf("Error: HOME directory does not exist!");
    return -1;
  }
  snprintf(basepath, 200, "%s/%s", home, ".config/gifpaper/.frames");

  dirp = opendir(basepath);
  while ((entry = readdir(dirp)) != NULL) {
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
      char fullpath[200];
      snprintf(fullpath, 200, "%s/%s", basepath, entry->d_name);
      unlink(fullpath);
    }
  }
  closedir(dirp);

  return 0;
}

char *generate_filename(char *prefix, int idx) {
  char buf[200];
  memset(buf, 0x00, 200);
  snprintf(buf, 200, "%s/frame%05d.png", prefix, idx);
  return strndup(buf, 200);
}*/
