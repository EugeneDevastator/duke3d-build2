#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/voc_audio.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  %s play <file.voc>\n", argv[0]);
        printf("  %s waver <file.voc> <out.wav>\n", argv[0]);
        return 1;
    }

    const char *cmd = argv[1];
    if (strcmp(cmd, "play") == 0) {
        const char *path = argv[2];
        return voc_play_with_raylib(path);
    } else if (strcmp(cmd, "waver") == 0) {
        if (argc < 4) {
            printf("Usage: %s waver <file.voc> <out.wav>\n", argv[0]);
            return 2;
        }
        const char *in = argv[2];
        const char *out = argv[3];
        int r = voc_to_wav(in, out);
        if (r == 0) printf("WAV written: %s\n", out);
        else printf("Conversion failed (code %d)\n", r);
        return r;
    } else {
        printf("Unknown command '%s'\n", cmd);
        return 3;
    }
}
