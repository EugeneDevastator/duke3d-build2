//
// Created by omnis on 11/21/2025.
//

#include "monoclip.h"

#include "monodebug.h"
#define EXLOGS 0

// array of head pairs; all mono polys known
mph_t *mph = 0;
int mphnum = 0;
int mphmal = 0;

// pool of heads.
mp_t *mp = 0;
int mpempty, mpmal = 0;

void mono_mph_check(int num) {
    if (num >= mphmal) {
        mphmal <<= 1;
        mph = (mph_t *) realloc(mph, mphmal * sizeof(mph[0]));
    }
}

void mono_initonce() {
    int i;

    mphnum = 0;
    mphmal = 512;
    mph = (mph_t *) realloc(mph, mphmal * sizeof(mph[0]));
    mpempty = 0;
    mpmal = 12048;
    mp = (mp_t *) realloc(mp, mpmal * sizeof(mp[0]));
    for (i = 0; i < mpmal; i++) {
        mp[i].n = i + 1;
        mp[i].p = i - 1;
    }
    mp[mpmal - 1].n = 0;
    mp[0].p = mpmal - 1;
    mono_dbg_init();
}

int mono_ins2d(int i, double nx, double ny) {
    int j, p, n, got;

    got = mpempty;
    p = mp[mpempty].p;
    n = mp[mpempty].n;
    if (p == n) {
        mpempty = mpmal;
        mpmal <<= 1;
        mp = (mp_t *) realloc(mp, mpmal * sizeof(mp[0]));
        for (j = mpempty; j < mpmal; j++) {
            mp[j].n = j + 1;
            mp[j].p = j - 1;
        }
        mp[mpmal - 1].n = mpempty;
        mp[mpempty].p = mpmal - 1;
    } else {
        mp[n].p = p;
        mp[p].n = n;
        mpempty = n;
    }
    #if EXLOGS
    printf("Mono| MP alloc: got=%d, empty=%d, used=%d \n", got, mpempty, mpmal - (mpempty >= 0 ? 1 : 0));
    #endif

    if (i < 0) //Start new loop
    {
        mp[got].p = got;
        mp[got].n = got;
    } else {
        n = mp[i].n;
        mp[got].p = i;
        mp[got].n = n;
        mp[n].p = got;
        mp[i].n = got;
    }
    mp[got].x = nx;
    mp[got].y = ny;
    return (got);
}

int mono_ins(int i, double nx, double ny, double nz) {
    dpoint3d p = {nx,ny,nz};
   // LOOPADD(p)
    i = mono_ins2d(i, nx, ny);
    mp[i].z = nz;
    return (i);
}
int mono_insp(int i, dpoint3d p) {
    i = mono_ins2d(i, p.x, p.y);
    mp[i].z = p.z;
  //  LOOPADD(p)
    return (i);
}
void mono_del(int i) {
    int p, n;
logstep("Mono| Del %d",i);
#if EXLOGS
    printf("Mono| MP dealloc: freeing=%d, old_empty=%d \n", i, mpempty);
#endif
    p = mp[i].p;
    n = mp[i].n;
    mp[n].p = p;
    mp[p].n = n;

    n = mp[mpempty].n;
    mp[i].p = mpempty;
    mp[i].n = n;
    mp[n].p = i;
    mp[mpempty].n = i;
}
void mono_deloop2(int* i) {
    mono_deloop(i[0]);
    mono_deloop(i[1]);
}

void mono_deloop(int i) {
    if (i<0)
        return;
    int j, count = 0;
    // ADD DEBUG - COUNT LOOP SIZE
    j = i;
    do {
        count++;
        if (count >70) {
            printf("Corrupt loop");
        }
        j = mp[j].n;
    } while (j != i);
    if (i < 0) return;
    //while (mp[i].n != i) mono_del(mp[i].n); mono_del(i);

    //logstep("Mono| DelLoop %d (size=%d), old_empty=%d", i, count, mpempty);

    //mpempty <-> {i .. mp[i].p} <-> mp[mpempty].n
    j = mp[i].p; //WARNING:this temp var needed for loops of only 1 element
    mp[j].n = mp[mpempty].n;
    mp[mp[mpempty].n].p = j;
    mp[i].p = mpempty;
    mp[mpempty].n = i;
    // ADD DEBUG AFTER BULK FREE
#if EXLOGS
    printf("Mono| DelLoop freed %d nodes, new_empty=%d \n", count, mpempty);
#endif

}

void mono_centroid_addlin(int i0, int i1, double *cx, double *cy, double *area) {
    double x0, y0, x1, y1;
    x0 = mp[i0].x; y0 = mp[i0].y;
    x1 = mp[i1].x; y1 = mp[i1].y;
    (*cx) += ((x0+x1)*x0 + x1*x1)*(y1-y0);
    (*cy) += ((y0+y1)*y0 + y1*y1)*(x0-x1);
    (*area) += (x0+x1)*(y1-y0);
}

double mono_centroid(int hd0, int hd1, double *retcx, double *retcy) {
    int i;
    double f, cx = 0.0, cy = 0.0, area = 0.0;
    if ((hd0|hd1) < 0) { return(0.0); }
    for(i=mp[hd0].n;i!=hd0;i=mp[i].n) { mono_centroid_addlin(mp[i].p,i,&cx,&cy,&area); } mono_centroid_addlin(mp[hd0].p,mp[hd1].p,&cx,&cy,&area);
    for(i=mp[hd1].n;i!=hd1;i=mp[i].n) { mono_centroid_addlin(i,mp[i].p,&cx,&cy,&area); } mono_centroid_addlin(   hd1   ,   hd0   ,&cx,&cy,&area);
    f = 1.0/(area*3.0); (*retcx) = cx*f; (*retcy) = cy*f; return(area*0.5);
}

double mono_area(int hd0, int hd1) { double fx, fy; return(mono_centroid(hd0,hd1,&fx,&fy)); }

void mono_genfromloop(int *plothead0, int *plothead1, dpoint3d *tp, int n) {
    int i, i0, i1, imin, imax, plothead[2];
    if (n<3) {
        *plothead0=-1;
        *plothead1=-1;
        printf("2 vertices for loop");
        return;
    }
    imin = 0;
    imax = 0;
    for (i = 0; i < n; i++) {
        if (tp[i].x < tp[imin].x) imin = i;
        if (tp[i].x > tp[imax].x) imax = i;
    }
    plothead[0] = -1;
    plothead[1] = -1;
    if (imin != imax) {
        i0 = imin;
        while (1) {
            i = i0 + 1;
            if (i >= n) i = 0;
            if (tp[i0].x < tp[i].x) break;
            i0 = i;
        }
        i1 = imax;
        while (1) {
            i = i1 - 1;
            if (i < 0) i = n - 1;
            if (tp[i1].x > tp[i].x) break;
            i1 = i;
        }
        i = i0;
        while (1) {
            plothead[0] = mono_ins(plothead[0], tp[i].x, tp[i].y, tp[i].z);
            if (i == i1) break;
            i++;
            if (i >= n) i = 0;
        }
        i0 = imin;
        while (1) {
            i = i0 - 1;
            if (i < 0) i = n - 1;
            if (tp[i0].x < tp[i].x) break;
            i0 = i;
        }
        i1 = imax;
        while (1) {
            i = i1 + 1;
            if (i >= n) i = 0;
            if (tp[i1].x > tp[i].x) break;
            i1 = i;
        }
        i = i0;
        while (1) {
            plothead[1] = mono_ins(plothead[1], tp[i].x, tp[i].y, tp[i].z);
            if (i == i1) break;
            i--;
            if (i < 0) i = n - 1;
        }
        plothead[0] = mp[plothead[0]].n;
        plothead[1] = mp[plothead[1]].n;
    }
    //logstep("Mono| Gen from loop, HEADS %d:%d",plothead[0],plothead[1]);
    (*plothead0) = plothead[0];
    (*plothead1) = plothead[1];
}

void mono_intersamexy(double x0, double y0, double x1, double y1, double z0, double z1, double z2, double z3, double *ix,
                 double *iy, double *iz) {
    double f;
    z2 -= z0;
    z3 -= z1;
    if (fabs(z2 - z3) < 1e-8) {
        (*ix) = x0;
        (*iy) = y0;
        (*iz) = z0;
        return;
    } //FIXFIX!
    f = z2 / (z2 - z3);
    (*ix) = (x1 - x0) * f + x0;
    (*iy) = (y1 - y0) * f + y0;
    (*iz) = (z1 - z0) * f + z0;
}
int intersect_traps_mono_points(dpoint3d p0, dpoint3d p1, dpoint3d trap1[4], dpoint3d trap2[4], int *rh0, int *rh1) {
    return intersect_traps_mono(
        p0.x, p0.y, p1.x, p1.y,
        trap1[0].z, trap1[1].z, trap1[2].z, trap1[3].z,
        trap2[0].z, trap2[1].z, trap2[2].z, trap2[3].z,
        rh0, rh1
    );
}
int intersect_traps_mono(double x0, double y0, double x1, double y1, double z0, double z4, double z5, double z1,
                         double z2, double z6, double z7, double z3, int *rh0, int *rh1) {
    double fx, fy, fz;
    int i, j, h0, h1;

    //0123,0213,0231,2013,2031,2301
    if (z0 < z2) {
        if (z1 < z2) i = 0;
        else i = (z1 >= z3) + 1;
    } else {
        if (z3 < z0) i = 5;
        else i = (z3 < z1) + 3;
    }
    if (z4 < z6) {
        if (z5 < z6) j = 0;
        else j = (z5 >= z7) + 1;
    } else {
        if (z7 < z4) j = 5;
        else j = (z7 < z5) + 3;
    }

    h0 = -1;
    h1 = -1;
    if ((i == 0) || (i == 5)) {
        if (i != j) {
            if (i == 5) mono_intersamexy(x0, y0, x1, y1, z0, z4, z3, z7, &fx, &fy, &fz);
            else mono_intersamexy(x0, y0, x1, y1, z1, z5, z2, z6, &fx, &fy, &fz);
            h0 = mono_ins(h0, fx, fy, fz);
            h1 = mono_ins(h1, fx, fy, fz);
        }
    } else {
        if (i > 2) h0 = mono_ins(h0, x0, y0, z0);
        else h0 = mono_ins(h0, x0, y0, z2);
        if (i & 1) h1 = mono_ins(h1, x0, y0, z1);
        else h1 = mono_ins(h1, x0, y0, z3);
    }
    if (i != j) {
        if ((i < 3) != (j < 3)) {
            mono_intersamexy(x0, y0, x1, y1, z0, z4, z2, z6, &fx, &fy, &fz);
            h0 = mono_ins(h0, fx, fy, fz);
        }
        if (((i ^ 1) < 3) != ((j ^ 1) < 3)) {
            mono_intersamexy(x0, y0, x1, y1, z1, z5, z3, z7, &fx, &fy, &fz);
            h1 = mono_ins(h1, fx, fy, fz);
        }
    }
    if ((j == 0) || (j == 5)) {
        if (i != j) {
            if (j == 5) mono_intersamexy(x0, y0, x1, y1, z0, z4, z3, z7, &fx, &fy, &fz);
            else mono_intersamexy(x0, y0, x1, y1, z1, z5, z2, z6, &fx, &fy, &fz);
            h0 = mono_ins(h0, fx, fy, fz);
            h1 = mono_ins(h1, fx, fy, fz);
        }
    } else {
        if (j > 2) h0 = mono_ins(h0, x1, y1, z4);
        else h0 = mono_ins(h0, x1, y1, z6);
        if (j & 1) h1 = mono_ins(h1, x1, y1, z5);
        else h1 = mono_ins(h1, x1, y1, z7);
    }

    if ((h0 | h1) < 0) return (0);
    (*rh0) = mp[h0].n;
    (*rh1) = mp[h1].n;
    return (1);
}

int mono_max(int hd0, int hd1, int maxsid, int mode) {
    double f, g, dx, dy, dx2, dy2, x0[2], y0[2], x1[2], y1[2];
    int i, j, ogot, good, ho, hd[2], ind[2];

    //no overlap
    if ((mp[mp[hd0].p].x <= mp[hd1].x) || (mp[mp[hd1].p].x <= mp[hd0].x)) {
        if (!mode) return (-1);
        i = hd0;
        ho = -1;
        do {
            ho = mono_ins2d(ho, mp[i].x, mp[i].y);
            i = mp[i].n;
        } while (i != hd0);
        return (mp[ho].n);
    }

    hd[0] = hd0;
    hd[1] = hd1;

    for (i = 2 - 1; i >= 0; i--) {
        x0[i] = mp[hd[i]].x;
        y0[i] = mp[hd[i]].y;
        ind[i] = mp[hd[i]].n;
        x1[i] = mp[ind[i]].x;
        y1[i] = mp[ind[i]].y;
    }

    ho = -1;

    i = (x0[1] < x0[0]);
    j = (i ^ 1);
    while (x1[i] <= x0[j]) {
        if (j & mode) { ho = mono_ins2d(ho, x0[0], y0[0]); }
        ind[i] = mp[ind[i]].n;
        if (ind[i] == hd[i]) {
            ind[0] = hd[0];
            goto bad;
        }
        x0[i] = x1[i];
        x1[i] = mp[ind[i]].x;
        y0[i] = y1[i];
        y1[i] = mp[ind[i]].y;
    }

    f = (y1[i] - y0[i]) * (x0[j] - x0[i]);
    dx = x1[i] - x0[i];
    good = ((f - (y0[j] - y0[i]) * dx) * ((double) maxsid) > 0.0);
    if (j & mode) { ho = mono_ins2d(ho, x0[i], y0[i]); }
    if ((j & mode) != good) { ho = mono_ins2d(ho, x0[j], f / dx + y0[i]); }
    if (!good) { ho = mono_ins2d(ho, x0[j], y0[j]); }
    while (1) {
        ogot = (good == i);
        i = (x1[1] < x1[0]);
        j = (i ^ 1);

        dx2 = x0[j] - x1[j];
        dy2 = y0[j] - y1[j];
        f = (x1[j] - x1[i]) * dy2 - (y1[j] - y1[i]) * dx2;
        good = (f * ((double) maxsid) < 0.0);
        if ((good == i) != ogot) {
            dx = x0[i] - x1[i];
            dy = y0[i] - y1[i];
            g = dx * dy2 - dy * dx2;
            if (g) {
                f /= g;
                if ((f >= 0.0) && (f <= 1.0)) ho = mono_ins2d(ho, dx * f + x1[i], dy * f + y1[i]);
            }
        }
        if (good) { ho = mono_ins2d(ho, x1[i], y1[i]); }
        ind[i] = mp[ind[i]].n;
        if (ind[i] == hd[i]) break;
        x0[i] = x1[i];
        x1[i] = mp[ind[i]].x;
        y0[i] = y1[i];
        y1[i] = mp[ind[i]].y;
    }

    if ((i & mode) == good) { ho = mono_ins2d(ho, x1[i], (x1[i] - x0[j]) * dy2 / dx2 + y0[j]); }
    if (i & mode) //Write rest of red verts
    {
    bad:;
        do {
            ho = mono_ins2d(ho, mp[ind[0]].x, mp[ind[0]].y);
            ind[0] = mp[ind[0]].n;
        } while (ind[0] != hd[0]);
    }

    return (mp[ho].n);
}

int mono_clipself(int hd0, int hd1, bdrawctx* b, void (*mono_output)(int h0, int h1,bdrawctx* b)) {
    double f, g, ix, iy, dx, dy, dx2, dy2, x0[2], y0[2], x1[2], y1[2];
    int i, j, k, ogood, good, hd[2], ind[2], ho[2], outnum = 0;

    if ((hd0 < 0) || (hd1 < 0)) return (0);
    hd[0] = hd0;
    hd[1] = hd1;

    for (i = 2 - 1; i >= 0; i--) {
        x0[i] = mp[hd[i]].x;
        y0[i] = mp[hd[i]].y;
        ind[i] = mp[hd[i]].n;
        x1[i] = mp[ind[i]].x;
        y1[i] = mp[ind[i]].y;
        ho[i] = -1;
    }

    i = (x0[1] < x0[0]);
    j = (i ^ 1);
    while (x1[i] <= x0[j]) {
        ind[i] = mp[ind[i]].n;
        if (ind[i] == hd[i]) goto bad;
        x0[i] = x1[i];
        x1[i] = mp[ind[i]].x;
        y0[i] = y1[i];
        y1[i] = mp[ind[i]].y;
    }
    if (x0[i] < x0[j]) {
        f = (y1[i] - y0[i]) * (x0[j] - x0[i]);
        dx = x1[i] - x0[i];
        good = ((f - dx * (y0[j] - y0[i])) * ((double) (i * 2 - 1)) > 0.0);
        if (good) {
            ho[i] = mono_ins2d(ho[i], x0[j], f / dx + y0[i]);
            ho[j] = mono_ins2d(ho[j], x0[j], y0[j]);
        }
    } else {
        good = (y0[0] < y0[1]);
        if (good) { for (k = 2 - 1; k >= 0; k--) { ho[k] = mono_ins2d(ho[k], x0[k], y0[k]); } }
    }
    while (1) {
        i = (x1[1] < x1[0]);
        j = (i ^ 1);
        ogood = good;

        dx2 = x0[j] - x1[j];
        dy2 = y0[j] - y1[j];
        f = (x1[j] - x1[i]) * dy2 - (y1[j] - y1[i]) * dx2;
        good = (f * ((double) (i * 2 - 1)) < 0.0);
        if (good ^ ogood) {
            dx = x0[i] - x1[i];
            dy = y0[i] - y1[i];
            g = dx * dy2 - dy * dx2;
            if (g) {
                f /= g;
                ix = dx * f + x1[i];
                iy = dy * f + y1[i];
                for (k = 2 - 1; k >= 0; k--) { ho[k] = mono_ins2d(ho[k], ix, iy); }
            }
            if (!good) {
                //logstep("Mono| clipself, callback bad HEADS %d:%d",mp[ho[0]].n,mp[ho[1]].n);
                mono_output(mp[ho[0]].n, mp[ho[1]].n,b);
                ho[0] = -1;
                ho[1] = -1;
                outnum++;
            }
        }

        ind[i] = mp[ind[i]].n;
        if (ind[i] == hd[i]) break;
        if (good) { ho[i] = mono_ins2d(ho[i], x1[i], y1[i]); }
        x0[i] = x1[i];
        x1[i] = mp[ind[i]].x;
        y0[i] = y1[i];
        y1[i] = mp[ind[i]].y;
    }
    if (good) {
        ho[i] = mono_ins2d(ho[i], x1[i], y1[i]);
        ho[j] = mono_ins2d(ho[j], x1[i], (x1[i] - x0[j]) * dy2 / dx2 + y0[j]);
    }

    for (k = 2 - 1; k >= 0; k--) if (ho[k] >= 0) { ho[k] = mp[ho[k]].n; }
bad:;
    //logstep("Mono| clipself, callback bad HEADS %d:%d",ho[0],ho[1]);
    mono_output(ho[0], ho[1],b);
    if ((ho[0] >= 0) && (ho[1] >= 0)) outnum++;
    return (outnum);
}

int mono_clipends(int hd, double x0, double x1) {
    int i0, i1;

    if (hd < 0) return (-1);
    if ((mp[hd].x >= x1) || (mp[mp[hd].p].x <= x0)) {
        mono_deloop(hd);
        return (-1);
    }

    while (mp[mp[hd].n].x <= x0) {
        hd = mp[hd].n;
        mono_del(mp[hd].p);
    }
    while (mp[mp[mp[hd].p].p].x >= x1) { mono_del(mp[hd].p); }
    if (mp[hd].x < x0) {
        i0 = hd;
        i1 = mp[i0].n;
        mp[i0].y = (x0 - mp[i0].x) * (mp[i1].y - mp[i0].y) / (mp[i1].x - mp[i0].x) + mp[i0].y;
        mp[i0].x = x0;
    }
    if (mp[mp[hd].p].x > x1) {
        i0 = mp[hd].p;
        i1 = mp[i0].p;
        mp[i0].y = (x1 - mp[i0].x) * (mp[i1].y - mp[i0].y) / (mp[i1].x - mp[i0].x) + mp[i0].y;
        mp[i0].x = x1;
    }

    return (hd);
}

int mono_join(int hd0, int hd1, int hd2, int hd3, int *ho0, int *ho1) {
    int i, j, t, hd[4], iy[4], ho[2];

    hd[0] = hd0;
    hd[1] = hd1;
    hd[2] = hd2;
    hd[3] = hd3;

    if (mp[hd[0]].x > mp[hd[2]].x) {
        for (j = 2 - 1; j >= 0; j--) {
            t = hd[j];
            hd[j] = hd[j + 2];
            hd[j + 2] = t;
        }
    }
    //(mp[mp[hd[1]].p].x != mp[hd[3]].x)) return (0); hd[1] is out of bounds sometimes.
    if ((mp[mp[hd[0]].p].x != mp[hd[2]].x) || (mp[mp[hd[1]].p].x != mp[hd[3]].x)) return (0);
    for (j = 2 - 1; j >= 0; j--) {
        iy[j + 0] = mp[hd[j + 0]].p;
        while ((mp[mp[iy[j + 0]].p].x == mp[iy[j + 0]].x) && (iy[j + 0] != hd[j + 0])) iy[j + 0] = mp[iy[j + 0]].p;
        iy[j + 2] = hd[j + 2];
        while ((mp[mp[iy[j + 2]].n].x == mp[iy[j + 2]].x) && (mp[iy[j + 2]].n != hd[j + 2]))
            iy[j + 2] = mp[iy[j + 2]].n;
    }
    if (max(mp[iy[0]].y, mp[iy[2]].y) >= min(mp[iy[1]].y, mp[iy[3]].y)) return (0);
    for (j = 2 - 1; j >= 0; j--) {
        ho[j] = -1;
        for (i = hd[j + 0]; 1;) {
            ho[j] = mono_ins2d(ho[j], mp[i].x, mp[i].y);
            if (i == iy[j + 0]) break;
            i = mp[i].n;
        }
        for (i = iy[j + 2]; 1;) {
            ho[j] = mono_ins2d(ho[j], mp[i].x, mp[i].y);
            i = mp[i].n;
            if (i == hd[j + 2]) break;
        }
        ho[j] = mp[ho[j]].n;
    }
    //logstep("Mono| join ok, HEADS IN %d:%d %d:%d -> %d:%d",hd0,hd1,hd2,hd3, ho[0],ho[1]);
    (*ho0) = ho[0];
    (*ho1) = ho[1];
    return (1);
}

void mono_bool(int hr0, int hr1, int hw0, int hw1, int boolop, bdrawctx* b, void (*mono_output)(int h0, int h1,bdrawctx* b)) {
    int hd0, hd1;
    if (g_captureframe) {
        char buf[64];
        sprintf(buf, "BEFORE_%s_hr", (boolop==MONO_BOOL_AND)?"AND":(boolop==MONO_BOOL_SUB)?"SUB":"SUBREV");
        mono_dbg_capture_pair(hr0, hr1, buf, boolop);
        sprintf(buf, "BEFORE_%s_hw", (boolop==MONO_BOOL_AND)?"AND":(boolop==MONO_BOOL_SUB)?"SUB":"SUBREV");
        mono_dbg_capture_pair(hw0, hw1, buf, boolop);
    }
    //logstep("Mono|> BOOL, op = %d HEADS IN %d:%d %d:%d",boolop,hr0,hr1,hw0,hw1);
    if (boolop == MONO_BOOL_AND) {
        //{ //Debug!
        //int i;
        //printf("---- gcnt=%d\n",gcnt);
        //printf("hr0\n"); i = hr0; do { printf("\t%22.16f %22.16f\n",mp[i].x,mp[i].y); i = mp[i].n; } while (i != hr0);
        //printf("hr1\n"); i = hr1; do { printf("\t%22.16f %22.16f\n",mp[i].x,mp[i].y); i = mp[i].n; } while (i != hr1);
        //printf("hw0\n"); i = hw0; do { printf("\t%22.16f %22.16f\n",mp[i].x,mp[i].y); i = mp[i].n; } while (i != hw0);
        //printf("hw1\n"); i = hw1; do { printf("\t%22.16f %22.16f\n",mp[i].x,mp[i].y); i = mp[i].n; } while (i != hw1);
        //}

        hd0 = mono_max(hr0, hw0, +1, 0);
        hd1 = mono_max(hr1, hw1, -1, 0);

        mono_clipself(hd0, hd1, b, mono_output);
        mono_deloop(hd1);
        mono_deloop(hd0);
    } else {
        boolop = (boolop == MONO_BOOL_SUB);
        hd0 = mono_max(hr1, hw0, -1, boolop ^ 1);

        mono_clipself(hr0, hd0, b, mono_output);
        mono_deloop(hd0);
        hd0 = mono_max(hr0, hw1, +1, boolop);

        mono_clipself(hd0, hr1, b, mono_output);
        mono_deloop(hd0);
    }
    //logstep("Mono|< BOOLEND");
}
void strip_init(triangle_strip_t *strip) {
   // strip->indices = NULL;
    strip->count = 0;
    strip->capacity = 0;
}

void strip_free(triangle_strip_t *strip) {
    if (strip->indices) {
        free(strip->indices);
        strip->indices = NULL;
    }
    strip->count = 0;
    strip->capacity = 0;
}

void strip_add(triangle_strip_t *strip, int index) {
    if (strip->count >= strip->capacity) {
        strip->capacity = max(strip->capacity << 1, 16);
        strip->indices = (int *)realloc(strip->indices, strip->capacity * sizeof(int));
    }
    strip->indices[strip->count++] = index;
}
int mono_generate_eyepol(int hd0, int hd1, point3d **out_verts1,  point3d **out_verts2, int *out_count1, int *out_count2) {
    if ((hd0 | hd1) < 0) {
        *out_verts1 = NULL;
        *out_verts2 = NULL;
        *out_count1 = 0;
        *out_count2 = 0;
        return 0;
    }

    point3d *verts1 = (point3d *)malloc((64) * sizeof(point3d));
    point3d *verts2 = (point3d *)malloc((64) * sizeof(point3d));
    int v1cnt = 0;
    // Add chain0 forward
    int i = hd0;
    do {
        verts1[v1cnt].x = (float)mp[i].x;
        verts1[v1cnt].y = (float)mp[i].y;
        verts1[v1cnt].z = (float)mp[i].z;
        v1cnt++;
        i = mp[i].n;
    } while (i != hd0);
    mono_deloop(hd0);

    // Collect chain1 indices
    i = hd1;
    int v2cnt = 0;
    do {
        i = mp[i].p;
        verts2[v2cnt].x = (float)mp[i].x;
        verts2[v2cnt].y = (float)mp[i].y;
        verts2[v2cnt].z = (float)mp[i].z;
        v2cnt++;
    } while (i != hd1);


    mono_deloop(hd0);
  //  free(chain1);

    *out_count1 = v1cnt;
    *out_count2 = v2cnt;
    *out_verts1 = verts1;
    *out_verts2 = verts2;
    return (v1cnt+v2cnt >= 3) ? 1 : 0;
}
// ============= Mono Polygons management ==============
int mph_appendloop(int *outh1, int *outh2, dpoint3d *tp, int n, int newtag) {
    *outh1=-1;
    *outh2=-1;
    mono_mph_check(mphnum);
    mono_genfromloop(&mph[mphnum].head[0], &mph[mphnum].head[0], tp, n);

    if ((mph[mphnum].head[0] | mph[mphnum].head[1]) < 0)
    { mono_deloop(mph[mphnum].head[0]); mono_deloop(mph[mphnum].head[1]); return 0; }

    mph[mphnum].tag = newtag;
    mphnum++;
    *outh1 = mph[mphnum].head[0] ;
    *outh2 = mph[mphnum].head[1] ;
    return mphnum;
}

int mph_remove(int delid) {
    if (delid < 0 || delid > mphnum)
        return 0;
    mono_deloop(mph[delid].head[1]);
    mono_deloop(mph[delid].head[0]);
    mph[delid] = mph[mphnum-1];
    mphnum--;
    return mphnum;
}

int mph_append(int h1, int h2, int tag) {
    if ((h1 | h2)<0)
        return 0;
    mono_mph_check(mphnum);
    mph[mphnum].head[0] = h1;
    mph[mphnum].head[1] = h2;
    mph[mphnum].tag = tag;
    mphnum++;
    return mphnum;
}


int mpcheck(int h1, int h2) {
    if ((h1|h2)<0) {
        return 0;
    }
    return 1;
}

int mphremoveontag(int tag) {
    for (int i = mphnum - 1; i >= 0; i--) {
        if (mph[i].tag == tag)
            mph_remove(i);
    }
}

int mphremoveaboveincl(int tag_including) {
    for (int i = mphnum - 1; i >= 0; i--) {
        if (mph[i].tag >= tag_including)
            mph_remove(i);
    }
}

void monocopy(int h1, int h2, int *hout1, int *hout2) {
    *hout1 = -1;
    *hout2 = -1;

    if ((h1 | h2) < 0) {
        return;
    }

    // Copy first head chain
    if (h1 >= 0) {
        int i = h1;
        do {
            *hout1 = mono_ins(*hout1, mp[i].x, mp[i].y, mp[i].z);
            i = mp[i].n;
        } while (i != h1);
        *hout1 = mp[*hout1].n;
    }

    // Copy second head chain
    if (h2 >= 0) {
        int i = h2;
        do {
            *hout2 = mono_ins(*hout2, mp[i].x, mp[i].y, mp[i].z);
            i = mp[i].n;
        } while (i != h2);
        *hout2 = mp[*hout2].n;
    }
}






