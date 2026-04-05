#include "voc_decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helpers to read little-endian
static uint16_t readLE16(FILE *f) {
    int a = fgetc(f);
    int b = fgetc(f);
    if (a == EOF || b == EOF) return 0;
    return (uint16_t)( (uint8_t)a | ((uint16_t)(uint8_t)b << 8) );
}

static uint32_t readLE24(FILE *f) {
    int b1 = fgetc(f);
    int b2 = fgetc(f);
    int b3 = fgetc(f);
    if (b1 == EOF || b2 == EOF || b3 == EOF) return 0;
    return (uint32_t)((uint8_t)b1 | ((uint32_t)(uint8_t)b2 << 8) | ((uint32_t)(uint8_t)b3 << 16));
}

static uint32_t readLE32(FILE *f) {
    uint32_t b1 = (uint8_t)fgetc(f);
    uint32_t b2 = (uint8_t)fgetc(f);
    uint32_t b3 = (uint8_t)fgetc(f);
    uint32_t b4 = (uint8_t)fgetc(f);
    return b1 | (b2<<8) | (b3<<16) | (b4<<24);
}

VocPCM voc_decode_file(const char *filename) {
    VocPCM pcm;
    memset(&pcm, 0, sizeof(pcm));
    pcm.channels = 1;
    pcm.bitsPerSample = 8;
    pcm.sampleRate = 22050;
    pcm.compressed = 0;

    FILE *f = fopen(filename, "rb");
    if (!f) return pcm;

    // read header
    char magic[20] = {0};
    if (fread(magic, 1, 19, f) != 19) { fclose(f); return pcm; }
    magic[19] = 0;
    if (strcmp(magic, "Creative Voice File") != 0) {
        fclose(f);
        return pcm;
    }

    // skip bytes up to first block (header length given at offset 0x16 usually)
    // Spec: offset 0x16 contains data offset (word) to first block. Simpler: seek to 0x1A
    fseek(f, 0x1A, SEEK_SET);

    unsigned char *buffer = NULL;
    size_t totalData = 0;

    while (!feof(f)) {
        int blockType = fgetc(f);
        if (blockType == EOF) break;

        uint32_t blockSize = readLE24(f);
        if (blockSize == 0) break;

        if (blockType == 0) {
            // terminator
            break;
        } else if (blockType == 1) {
            // sound data block
            int sampleRateByte = fgetc(f);
            int pack = fgetc(f); // packing method (0=PCM unsigned 8-bit, 1=ADPCM-4, 2=???)

            if (sampleRateByte == EOF || pack == EOF) { break; }

            // VOC uses rate = 1000000 / (256 - sampleRateByte)
            int sampleRate = 1000000 / (256 - sampleRateByte);
            pcm.sampleRate = sampleRate;

            // data size = blockSize - 2 (sampleRateByte + pack)
            uint32_t dataSize = blockSize > 2 ? blockSize - 2 : 0;

            if (pack == 0) {
                // PCM unsigned 8-bit stored; convert to signed 16-bit little-endian (suitable for WAV)
                unsigned char *tmp = malloc(dataSize);
                if (!tmp) { free(buffer); fclose(f); return pcm; }
                if (fread(tmp, 1, dataSize, f) != dataSize) {
                    free(tmp); break;
                }
                // convert to 16-bit signed little-endian
                size_t newBytes = dataSize * 2;
                unsigned char *newBuf = realloc(buffer, totalData + newBytes);
                if (!newBuf) { free(buffer); free(tmp); fclose(f); return pcm; }
                buffer = newBuf;
                // unsigned 8-bit to signed 16-bit: (u8 - 128) << 8
                for (size_t i = 0; i < dataSize; ++i) {
                    int16_t v = (int16_t)(((int)tmp[i] - 128) << 8);
                    buffer[totalData + i*2 + 0] = (uint8_t)(v & 0xFF);
                    buffer[totalData + i*2 + 1] = (uint8_t)((v >> 8) & 0xFF);
                }
                totalData += newBytes;
                free(tmp);
                pcm.bitsPerSample = 16;
                pcm.channels = 1;
                pcm.compressed = 0;
            } else {
                // compressed (ADPCM) or other packing — we will not attempt risky decode here.
                // Seek past the block and set compressed flag for external tool handling.
                fseek(f, dataSize, SEEK_CUR);
                pcm.compressed = 1;
                // We still continue scanning to maybe find later PCM blocks (rare); but we return compressed=1
            }
        } else if (blockType == 2) {
            // continuation of sound data — treated similarly to block 1 headerless
            // In many VOCs block type 2 is a continuation: first byte of the block may be packing method in older formats.
            // We'll try to read first two bytes to decide; if we can't, skip.
            long pos = ftell(f);
            if (pos < 0) break;
            // For safety just skip block and mark compressed, because it's risky to guess
            fseek(f, blockSize, SEEK_CUR);
            pcm.compressed = 1;
        } else {
            // other block types: skip
            fseek(f, blockSize, SEEK_CUR);
        }
    }

    fclose(f);

    if (totalData > 0) {
        pcm.data = buffer;
        pcm.dataSize = totalData;
        pcm.bitsPerSample = 16; // we converted to 16-bit
        pcm.channels = 1;
        pcm.compressed = 0;
    } else {
        // no decoded PCM found
        if (buffer) free(buffer);
        pcm.data = NULL;
        pcm.dataSize = 0;
        // pcm.compressed likely 1 if we encountered compressed blocks
    }

    return pcm;
}

void FreeVocPCM(VocPCM *pcm) {
    if (!pcm) return;
    if (pcm->data) free(pcm->data);
    pcm->data = NULL;
    pcm->dataSize = 0;
}
