#ifndef VOC_AUDIO_H
#define VOC_AUDIO_H

// play a VOC file with raylib UI (blocking until user closes window).
// Returns 0 on success, non-zero on error.
int voc_play_with_raylib(const char *vocPath);

// convert VOC to WAV on disk. If voc is decodable internally, writes WAV directly.
// For compressed VOCs tries to use ffmpeg (must be in PATH). Returns 0 on success.
int voc_to_wav(const char *vocPath, const char *outWav);

#endif // VOC_AUDIO_H
