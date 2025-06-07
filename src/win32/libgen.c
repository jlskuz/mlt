#include "libgen.h"

char* basename(char* path) {
    char* base = strrchr(path, '/');
    return base ? base + 1 : path;
}

char* dirname(char* path) {
    char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        return ".";
    }
    if (last_slash == path) {
        return "/";
    }
    *last_slash = '\0';
    return path;

    if (path && strlen(path)) {
        char *dirsep = strrchr(path, '/');
        // Handle back slash on Windows.
        if (!dirsep)
            dirsep = strrchr(path, '\\');
        // Handle trailing slash.
        if (dirsep == &path[strlen(path) - 1]) {
            dirsep[0] = '\0';
            dirsep = strrchr(path, '/');
            if (!dirsep)
                dirsep = strrchr(path, '\\');
        }
        // Truncate string at last directory separator.
        if (dirsep)
            dirsep[0] = '\0';
    }
    return path;
}
