#ifndef VOC_DECODER_H
#define VOC_DECODER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    unsigned char *data;     // PCM data (signed 16-bit little-endian after internal conversion) OR raw 8-bit PCM if kept
    size_t dataSize;         // bytes in data
    int sampleRate;          // e.g. 22050
    int bitsPerSample;       // 8 or 16
    int channels;            // 1 normally
    int compressed;          // 0 if decoded into PCM already; 1 if VOC is compressed and data==NULL (caller should convert via ffmpeg)
} VocPCM;

// decode VOC if it's plain PCM (pack==0). For compressed VOCs sets compressed=1 and returns minimal header info.
// Caller must call FreeVocPCM(&pcm) when finished.
VocPCM voc_decode_file(const char *filename);

// free resources
void FreeVocPCM(VocPCM *pcm);

#endif // VOC_DECODER_H
