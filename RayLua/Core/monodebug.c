// monodebug.c
#include "monodebug.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

mono_dbg_capture_t g_mono_dbg = {0};
int g_captureframe = 0;
dpoint3d loops[70000]={};
dpoint3d loopsm[70000]={};
dpoint3d loopscam[70000]={};
bool loopuse[70000]={};
int loopnum=0;
int captureframe=0;
 transform lastcamtr = {};
 transform lastcamtr2 = {};
int opercurr=0;
signed int operstopn=-1;

void logstep(const char *fmt, ...) {
    if (!captureframe)
        return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void mono_dbg_init(void) {
    g_mono_dbg.capacity = 64;
    g_mono_dbg.snapshots = (mono_dbg_snapshot_t*)malloc(g_mono_dbg.capacity * sizeof(mono_dbg_snapshot_t));
    g_mono_dbg.snapshot_count = 0;
    loopnum=0;
}

void mono_dbg_free(void) {
    int i, j;
    for (i = 0; i < g_mono_dbg.snapshot_count; i++) {
        for (j = 0; j < g_mono_dbg.snapshots[i].chain_count; j++) {
            free(g_mono_dbg.snapshots[i].chains[j].points);
        }
        free(g_mono_dbg.snapshots[i].chains);
    }
    free(g_mono_dbg.snapshots);
    g_mono_dbg.snapshots = NULL;
    g_mono_dbg.snapshot_count = 0;
    g_mono_dbg.capacity = 0;
}

void mono_dbg_clear(void) {
    mono_dbg_free();
    mono_dbg_init();
}

void mono_dbg_capture_chain(int hd, int chain_id, const char *label, int operation) {
    int i, count;
    mono_dbg_snapshot_t *snap;
    mono_dbg_chain_t *chain;

    if (!g_captureframe || hd < 0) return;

    if (g_mono_dbg.snapshot_count >= g_mono_dbg.capacity) {
        g_mono_dbg.capacity *= 2;
        g_mono_dbg.snapshots = (mono_dbg_snapshot_t*)realloc(g_mono_dbg.snapshots,
                                                              g_mono_dbg.capacity * sizeof(mono_dbg_snapshot_t));
    }

    snap = &g_mono_dbg.snapshots[g_mono_dbg.snapshot_count++];
    snap->operation = operation;
    strncpy(snap->label, label, 63);
    snap->label[63] = 0;
    snap->chain_count = 1;
    snap->chains = (mono_dbg_chain_t*)malloc(sizeof(mono_dbg_chain_t));

    chain = &snap->chains[0];
    chain->chain_id = chain_id;

    // Count vertices
    count = 1;
    i = mp[hd].n;
    while (i != hd) {
        count++;
        i = mp[i].n;
    }

    chain->count = count;
    chain->points = (mono_dbg_point_t*)malloc(count * sizeof(mono_dbg_point_t));

    // Copy vertices
    count = 0;
    i = hd;
    do {
        chain->points[count].x = mp[i].x;
        chain->points[count].y = mp[i].y;
        chain->points[count].z = mp[i].z;
        count++;
        i = mp[i].n;
    } while (i != hd);
}

void mono_dbg_capture_pair(int hd0, int hd1, const char *label, int operation) {
    int i, count;
    mono_dbg_snapshot_t *snap;
    mono_dbg_chain_t *chain;

    if (!g_captureframe) return;
    if ((hd0 | hd1) < 0) return;

    if (g_mono_dbg.snapshot_count >= g_mono_dbg.capacity) {
        g_mono_dbg.capacity *= 2;
        g_mono_dbg.snapshots = (mono_dbg_snapshot_t*)realloc(g_mono_dbg.snapshots,
                                                              g_mono_dbg.capacity * sizeof(mono_dbg_snapshot_t));
    }

    snap = &g_mono_dbg.snapshots[g_mono_dbg.snapshot_count++];
    snap->operation = operation;
    strncpy(snap->label, label, 63);
    snap->label[63] = 0;
    snap->chain_count = 2;
    snap->chains = (mono_dbg_chain_t*)malloc(2 * sizeof(mono_dbg_chain_t));

    // Capture chain 0
    chain = &snap->chains[0];
    chain->chain_id = 0;
    count = 1;
    i = mp[hd0].n;
    while (i != hd0) {
        count++;
        i = mp[i].n;
    }
    chain->count = count;
    chain->points = (mono_dbg_point_t*)malloc(count * sizeof(mono_dbg_point_t));
    count = 0;
    i = hd0;
    do {
        chain->points[count].x = mp[i].x;
        chain->points[count].y = mp[i].y;
        chain->points[count].z = mp[i].z;
        count++;
        i = mp[i].n;
    } while (i != hd0);

    // Capture chain 1
    chain = &snap->chains[1];
    chain->chain_id = 1;
    count = 1;
    i = mp[hd1].p;
    while (i != hd1) {
        i = mp[i].p;
        count++;
    }
    chain->count = count;
    chain->points = (mono_dbg_point_t*)malloc(count * sizeof(mono_dbg_point_t));
    count = 0;
    i = hd1;
    do {
        i = mp[i].p;
        chain->points[count].x = mp[i].x;
        chain->points[count].y = mp[i].y;
        chain->points[count].z = mp[i].z;
        count++;

    } while (i != hd1);
}

void mono_dbg_capture_mph(int id, const char *label) {
    int i;
    int rethead[2];
    rethead[0] = mph[id].head[0];
    rethead[1] = mph[id].head[1];
    LOOPEND
    for(int h=0;h<2;h++)
    {
        i = rethead[h];
        do
        {
            if (h)
                i = mp[i].p;
        //   LOOPADD(mp[i].pos);
            if (!h) i = mp[i].n;
        } while (i != rethead[h]);
      //  mono_deloop(rethead[h]);
    }
}
