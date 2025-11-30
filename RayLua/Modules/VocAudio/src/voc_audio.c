#include "voc_audio.h"
#include "voc_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <raylib.h>

#if defined(_WIN32)
#include <windows.h>
#endif

//--------------------------------------------------------------
// Helper: write WAV from PCM16 mono/ster
//--------------------------------------------------------------
static int write_wav_from_pcm16(const char *outPath,
                                const void *data,
                                size_t bytes,
                                int sampleRate,
                                int channels)
{
    FILE *f = fopen(outPath, "wb");
    if (!f) return -1;

    uint32_t byteRate = sampleRate * channels * 2;  // 16-bit = 2 bytes
    uint16_t blockAlign = channels * 2;
    uint32_t subchunk2Size = (uint32_t)bytes;

    // RIFF
    fwrite("RIFF", 1, 4, f);
    uint32_t chunkSize = 36 + subchunk2Size;
    fwrite(&chunkSize, 4, 1, f);
    fwrite("WAVE", 1, 4, f);

    // fmt
    fwrite("fmt ", 1, 4, f);
    uint32_t fmtSize = 16;
    fwrite(&fmtSize, 4, 1, f);
    uint16_t audioFormat = 1;
    fwrite(&audioFormat, 2, 1, f);

    uint16_t ch = (uint16_t)channels;
    fwrite(&ch, 2, 1, f);

    uint32_t sr = (uint32_t)sampleRate;
    fwrite(&sr, 4, 1, f);

    uint32_t br = byteRate;
    fwrite(&br, 4, 1, f);

    uint16_t ba = blockAlign;
    fwrite(&ba, 2, 1, f);

    uint16_t bps = 16;
    fwrite(&bps, 2, 1, f);

    // data
    fwrite("data", 1, 4, f);
    fwrite(&subchunk2Size, 4, 1, f);
    fwrite(data, 1, subchunk2Size, f);

    fclose(f);
    return 0;
}

//--------------------------------------------------------------
// FFmpeg fallback
//--------------------------------------------------------------
static int ffmpeg_convert_voc_to_wav(const char *inPath, const char *outPath)
{
#if defined(_WIN32)
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "ffmpeg -y -hide_banner -loglevel error -i \"%s\" \"%s\"",
        inPath, outPath);
#else
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "ffmpeg -y -hide_banner -loglevel error -i '%s' '%s'",
        inPath, outPath);
#endif
    return system(cmd);
}

//--------------------------------------------------------------
// Convert VOC -> WAV
//--------------------------------------------------------------
int voc_to_wav(const char *vocPath, const char *outWav)
{
    VocPCM pcm = voc_decode_file(vocPath);

    // compressed VOC — use ffmpeg
    if (pcm.compressed) {
        FreeVocPCM(&pcm);
        return ffmpeg_convert_voc_to_wav(vocPath, outWav);
    }

    // PCM available
    if (pcm.data && pcm.dataSize > 0) {
        int r = write_wav_from_pcm16(outWav,
                                     pcm.data,
                                     pcm.dataSize,
                                     pcm.sampleRate,
                                     pcm.channels);
        FreeVocPCM(&pcm);
        return r;
    }

    FreeVocPCM(&pcm);
    return -2;
}

//--------------------------------------------------------------
// Play VOC using raylib (window stays open)
//--------------------------------------------------------------
int voc_play_with_raylib(const char *vocPath)
{
    char tmpWav[512];
    snprintf(tmpWav, sizeof(tmpWav),
        "tmp_voc_%ld.wav", (long)time(NULL));

    if (voc_to_wav(vocPath, tmpWav) != 0) {
        fprintf(stderr, "Failed to convert VOC to WAV.\n");
        return -1;
    }

    InitWindow(600, 150, "VOC Player");
    InitAudioDevice();

    Wave wave = LoadWave(tmpWav);

    if (wave.frameCount == 0 || wave.sampleRate == 0) {
        fprintf(stderr, "LoadWave(): failed.\n");
        CloseAudioDevice();
        CloseWindow();
        remove(tmpWav);
        return -2;
    }

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    if (sound.frameCount == 0) {
        fprintf(stderr, "LoadSoundFromWave(): failed.\n");
    }

    PlaySound(sound);

    bool paused = false;
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("VOC Player", 10, 10, 20, DARKGRAY);
        DrawText("SPACE: pause/resume | S: stop | Q: quit | W: write WAV",
                 10, 40, 12, DARKGRAY);

        DrawText(TextFormat("File: %s", vocPath), 10, 70, 12, DARKGRAY);

        if (IsKeyPressed(KEY_SPACE)) {
            if (paused) { ResumeSound(sound); paused = false; }
            else { PauseSound(sound); paused = true; }
        }
        if (IsKeyPressed(KEY_S)) StopSound(sound);
        if (IsKeyPressed(KEY_Q)) break;

        if (IsKeyPressed(KEY_W)) {
            char outW[512];
            snprintf(outW, sizeof(outW), "%s_out.wav", vocPath);
            int r = voc_to_wav(vocPath, outW);
            if (r == 0)
                DrawText("Saved!", 10, 100, 12, DARKGRAY);
            else
                DrawText("Failed!", 10, 100, 12, RED);
        }

        EndDrawing();
    }

    UnloadSound(sound);
    CloseAudioDevice();
    CloseWindow();

    remove(tmpWav);
    return 0;
}
