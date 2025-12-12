// monodebug.h
#ifndef MONODEBUG_H
#define MONODEBUG_H

#include "monoclip.h"
#define LOOPADD(p) loops[loopnum]=(p); loopuse[loopnum]=true; loopnum++;
#define LOOPEND loopuse[loopnum]=false;loopnum++;
typedef struct {
    double x, y, z;
} mono_dbg_point_t;

typedef struct {
    mono_dbg_point_t *points;
    int count;
    int chain_id;  // 0 or 1 for hd0/hd1
} mono_dbg_chain_t;

typedef struct {
    mono_dbg_chain_t *chains;
    int chain_count;
    int operation;  // MONO_BOOL_AND, etc.
    char label[64];
} mono_dbg_snapshot_t;

typedef struct {
    mono_dbg_snapshot_t *snapshots;
    int snapshot_count;
    int capacity;
} mono_dbg_capture_t;

extern mono_dbg_capture_t g_mono_dbg;
extern int g_captureframe;

void mono_dbg_init(void);
void mono_dbg_free(void);
void mono_dbg_clear(void);
void mono_dbg_capture_chain(int hd, int chain_id, const char *label, int operation);
void mono_dbg_capture_pair(int hd0, int hd1, const char *label, int operation);

extern dpoint3d loops[70000];
extern bool loopuse[70000];
extern int loopnum;
#endif