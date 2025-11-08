//
// Created by omnis on 11/8/2025.
//
#include "physics.h"

#include "mapcore.h"

void collmove (dpoint3d *p, int *cursect, dpoint3d *v, double cr, long doslide, mapstate_t* map)
{
	dpoint3d nv, n0, n1, n2;
	double f;
	long i;

	f = sqrt(v->x*v->x + v->y*v->y + v->z*v->z) + cr;
	build2.clipmaxcr = findmaxcr(p,*cursect,f,&nv);
	if (build2.clipmaxcr >= f) { p->x += v->x; p->y += v->y; p->z += v->z; build2.cliphitnum = -1; return; }
	cr = min(cr,build2.clipmaxcr);

	nv = (*v);

	build2.cliphitnum = 0; cr -= 1e-7;
	if (!sphtrace(p,&nv,&build2.cliphit[0],cursect,cr,map))
	{
		build2.cliphitnum = 1;
		if (doslide)
		{
				//Make slide vector (nv) parallel to surface hit (normal: build2.cliphit[0]-p)
			n0.x = build2.cliphit[0].x-p->x; n0.y = build2.cliphit[0].y-p->y; n0.z = build2.cliphit[0].z-p->z;
			f = (nv.x*n0.x + nv.y*n0.y + nv.z*n0.z) / (cr*cr);
			nv.x -= n0.x*f; nv.y -= n0.y*f; nv.z -= n0.z*f;

			cr -= 1e-7;
			if (!sphtrace(p,&nv,&build2.cliphit[1],cursect,cr,map))
			{
				build2.cliphitnum = 2;

					//Make slide vector (nv) parallel to both surfaces hit (normals: build2.cliphit[0]-p, build2.cliphit[1]-p)
				n1.x = build2.cliphit[1].x-p->x; n1.y = build2.cliphit[1].y-p->y; n1.z = build2.cliphit[1].z-p->z;
				n2.x = n0.z*n1.y - n0.y*n1.z;
				n2.y = n0.x*n1.z - n0.z*n1.x;
				n2.z = n0.y*n1.x - n0.x*n1.y;
				f = (nv.x*n2.x + nv.y*n2.y + nv.z*n2.z) / (n2.x*n2.x + n2.y*n2.y + n2.z*n2.z);
				nv.x = n2.x*f; nv.y = n2.y*f; nv.z = n2.z*f;

				cr -= 1e-7;
				if (!sphtrace(p,&nv,&build2.cliphit[2],cursect,cr,map)) build2.cliphitnum = 3;
			}
		}
	}
}

void collmove (point3d *p, int *cursect, point3d *v, double cr, long doslide, mapstate_t* map)
{
	dpoint3d np, nv;

	np.x = (double)p->x; nv.x = v->x;
	np.y = (double)p->y; nv.y = v->y;
	np.z = (double)p->z; nv.z = v->z;
	collmove(&np,cursect,&nv,cr,doslide);
	p->x = (float)np.x;
	p->y = (float)np.y;
	p->z = (float)np.z;
	if (build2.cliphitnum < 2) return;

		//FIX:This evil hack makes sure double->float quantization does not mess
		//    w/3rd stage of sliding collision
	if ((nv.x < 0.0) && ((double)p->x > np.x)) (*(long *)&p->x)--;
	if ((nv.x > 0.0) && ((double)p->x < np.x)) (*(long *)&p->x)++;
	if ((nv.y < 0.0) && ((double)p->y > np.y)) (*(long *)&p->y)--;
	if ((nv.y > 0.0) && ((double)p->y < np.y)) (*(long *)&p->y)++;
	if ((nv.z < 0.0) && ((double)p->z > np.z)) (*(long *)&p->z)--;
	if ((nv.z > 0.0) && ((double)p->z < np.z)) (*(long *)&p->z)++;
}

double findmaxcr (dpoint3d *p0, int cursect, double mindist, dpoint3d *hit, mapstate_t* map)
{
	dpoint3d np, nhit, pol[4], npol[4];
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	sect_t *sec;
	wall_t *wal, *wal2;
	spri_t *spr;
	double d, f, g, dist2, mindist2, mindist2andmaxfat;
	long *gotsect;
	int i, j, k, s, w, nw, bs, bw, vn, s0, s1, cf0, cf1, *secfif, secfifw, secfifr, hitit;

	if ((unsigned)cursect >= (unsigned)map->numsects) return(mindist);

	i = (((map->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(i);
	memset(gotsect,0,i);
	gotsect[cursect>>5] |= (1<<cursect);

	secfif = (int *)_alloca(map->numsects*sizeof(secfif[0]));
	secfif[0] = cursect; secfifr = 0; secfifw = 1;

	hitit = 0;
	mindist2 = mindist*mindist;
	mindist2andmaxfat = (mindist+build2.fattestsprite)*(mindist+build2.fattestsprite);
	hit->x = hit->y = hit->z = -17.0;
	sec = map->sect;
	//for(s=gst->numsects-1;s>=0;s--) //Brute force for now.. portals later :/
	while (secfifr < secfifw)
	{
		s = secfif[secfifr]; secfifr++;

		//dist2 = 0.0; //FIX: bounded box not implemented yet!
		//d = max(max((sec[s].minx) - p0->x,p0->x - (sec[s].maxx)),0.0); dist2 += d*d;
		//d = max(max((sec[s].miny) - p0->y,p0->y - (sec[s].maxy)),0.0); dist2 += d*d;
		//if (dist2 >= mindist2) continue;

		wal = sec[s].wall; j = 0;
		for(i=2-1;i>=0;i--)
		{
			if ((p0->z > getslopez(&sec[s],i,p0->x,p0->y)) == i) { j |= (i+1); continue; }

				// point: <wal[0].x,wal[0].y,sec[s].z[i]>
				//normal: <sec[s].grad[i].x,sec[s].grad[i].y,1>
			f = (p0->x-wal[0].x)*sec[s].grad[i].x + (p0->y-wal[0].y)*sec[s].grad[i].y + (p0->z-sec[s].z[i])*1.0;
			g = ((double)sec[s].grad[i].x)*sec[s].grad[i].x + ((double)sec[s].grad[i].y)*sec[s].grad[i].y + 1.0*1.0;
			f /= g;
			np.x = p0->x - sec[s].grad[i].x*f;
			np.y = p0->y - sec[s].grad[i].y*f;
			np.z = p0->z -              1.0*f;
			if (!insidesect(np.x,np.y,sec[s].wall,sec[s].n)) continue;
			f *= f*g; if (f < mindist2) { mindist2 = f; (*hit) = np; hitit = 1; }
		}
		if (j == 3) continue; //Behind ceiling or floor
		for(w=0;w<sec[s].n;w++)
		{
			nw = wal[w].n+w;
			vn = getwalls_imp(s,w,verts,MAXVERTS,map);
			if (wal[w].surf.flags&1) vn = 0; //Blocking wall
			pol[0].x = wal[ w].x; pol[0].y = wal[ w].y;
			pol[1].x = wal[nw].x; pol[1].y = wal[nw].y;
			pol[2].x = wal[nw].x; pol[2].y = wal[nw].y;
			pol[3].x = wal[ w].x; pol[3].y = wal[ w].y;
			for(k=0;k<=vn;k++) //Warning: do not reverse for loop!
			{
				if (k >  0) { s0 = verts[k-1].s; cf0 = 1; } else { s0 = s; cf0 = 0; }
				if (k < vn) { s1 = verts[k  ].s; cf1 = 0; } else { s1 = s; cf1 = 1; }

				pol[0].z = getslopez(&sec[s0],cf0,wal[ w].x,wal[ w].y);
				pol[1].z = getslopez(&sec[s0],cf0,wal[nw].x,wal[nw].y);

				if ((k) && (!(gotsect[s0>>5]&(1<<s0))))
				{
						//FIX:Should only test sectors > mindist2 for sprites...
					if (ptpolydist2(p0,pol,4,&nhit) < mindist2andmaxfat)
						{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
				}

				pol[2].z = getslopez(&sec[s1],cf1,wal[nw].x,wal[nw].y);
				pol[3].z = getslopez(&sec[s1],cf1,wal[ w].x,wal[ w].y);

				i = wallclip(pol,npol);
				if (i) { d = ptpolydist2(p0,npol,i,&nhit); if (d < mindist2) { mindist2 = d; (*hit) = nhit; hitit = 1; } }
			}
		}

		spr = map->spri;
		for(w=sec[s].headspri;w>=0;w=spr[w].sectn)
		{
			//if (spr[w].owner >= 0) continue;
			if (!(spr[w].flags&1)) continue;
			if (spr[w].fat > 0)
			{
				np.x = p0->x-spr[w].p.x;
				np.y = p0->y-spr[w].p.y;
				np.z = p0->z-spr[w].p.z;
				d = sqrt(np.x*np.x + np.y*np.y + np.z*np.z);
				f = d-spr[w].fat;
				if ((f <= 0.0) || (f*f >= mindist2)) continue;
				mindist2 = f*f;
				d = spr[w].fat/d;
				hit->x = spr[w].p.x + np.x*d;
				hit->y = spr[w].p.y + np.y*d;
				hit->z = spr[w].p.z + np.z*d;
				hitit = 1;
			}
			else //Flat polygon (wall/floor sprite)
			{
				for(i=4-1;i>=0;i--)
				{
					if ((i+1)&2) f = 1; else f = -1;
					if ((i  )&2) g = 1; else g = -1;
					pol[i].x = spr[w].p.x + spr[w].r.x*f + spr[w].d.x*g;
					pol[i].y = spr[w].p.y + spr[w].r.y*f + spr[w].d.y*g;
					pol[i].z = spr[w].p.z + spr[w].r.z*f + spr[w].d.z*g;
				}
				d = ptpolydist2(p0,pol,4,&nhit); if (d < mindist2) { mindist2 = d; (*hit) = nhit; hitit = 1; }
			}
		}
	}
	if (!hitit) return(mindist); //Minor optimization; this guarantees same value returned if nothing hit
	return(sqrt(mindist2));
}


//Find maximum clip radius (distance to closest point of any visible polygon)

	//Note: pol doesn't support loops as dpoint3d's!
	//(flags&1): collide both sides of plane
double sphpolydist (dpoint3d *p0, dpoint3d *v0, double cr, dpoint3d *pol, int n, int flags, dpoint3d *hit)
{
	dpoint3d np, fd, e, ff, fg;
	double f, g, t, u, v, nx, ny, nz, mint, Za, Zb, Zc, x0, y0, x1, y1;
	int i, j, k, maxnormaxis;

	mint = 1.0;

#if 0
		//Saving this block in case I choose to optimize wall planes by processing all segments simultaneously
	dpoint3d pol[4], npol[4];
	double wx, wy, wz;
	sect_t *sec;
	wall_t *wal;
	int nw;
	sec = gst->sect; wal = sec[s].wall; nw = wal[w].n+w;

	pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].z = getslopez(&sec[s0],cf0,pol[0].x,pol[0].y);
	pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].z = getslopez(&sec[s0],cf0,pol[1].x,pol[1].y);
	pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].z = getslopez(&sec[s1],cf1,pol[2].x,pol[2].y);
	pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].z = getslopez(&sec[s1],cf1,pol[3].x,pol[3].y);
	n = wallclip(pol,npol); if (!n) return(1.0);

		//Raytrace to planes
	nx = wal[w].y-wal[nw].y; ny = wal[nw].x-wal[w].x; //plane's normal (nz = 0)
	t = v0->x*nx + v0->y*ny;
	if (t < 0.0)
	{
		f = 1.0/sqrt(nx*nx + ny*ny); nx *= f; ny *= f; t *= f; //FIX:optimize
		wx = wal[w].x; wy = wal[w].y; wz = 0.0; //point on plane
		t = (cr - ((p0->x-wx)*nx + (p0->y-wy)*ny)) / t;
		np.x = v0->x*t + p0->x - nx*cr;
		np.y = v0->y*t + p0->y - ny*cr;
		np.z = v0->z*t + p0->z;
		if (fabs(wal[nw].x-wal[w].x) >= fabs(wal[nw].y-wal[w].y))
			  { f = np.x-wal[w].x; g = wal[nw].x-wal[w].x; }
		else { f = np.y-wal[w].y; g = wal[nw].y-wal[w].y; }
		if (g < 0) { f = -f; g = -g; }
		if ((t >= 0) && (t < mint) &&
			 (f >= 0.0) && (f <= g) &&
			 (np.z >= getslopez(&sec[s0],cf0,np.x,np.y)) &&
			 (np.z <= getslopez(&sec[s1],cf1,np.x,np.y)))
			{ mint = t; (*hit) = np; }
	}
#else
	nx = ny = nz = 0.0;
	for(i=n-2;i>0;i--)
	{
		nx += (pol[i].y-pol[0].y)*(pol[i+1].z-pol[0].z) - (pol[i].z-pol[0].z)*(pol[i+1].y-pol[0].y);
		ny += (pol[i].z-pol[0].z)*(pol[i+1].x-pol[0].x) - (pol[i].x-pol[0].x)*(pol[i+1].z-pol[0].z);
		nz += (pol[i].x-pol[0].x)*(pol[i+1].y-pol[0].y) - (pol[i].y-pol[0].y)*(pol[i+1].x-pol[0].x);
	}
	f = nx*nx + ny*ny + nz*nz;
	if (f > 0.0) //Plane must have area
	{
		f = -cr/sqrt(f); nx *= f; ny *= f; nz *= f;

		t = v0->x*nx + v0->y*ny + v0->z*nz;
		if ((flags&1) && (t > 0.0)) { t = -t; nx = -nx; ny = -ny; nz = -nz; }
		if (t < 0.0) //Vector must be towards plane
		{
				//(v0->x*t + p0->x - (pol[0].x + nx))*nx
				//(v0->y*t + p0->y - (pol[0].y + ny))*ny
				//(v0->z*t + p0->z - (pol[0].z + nz))*nz = 0
			t = ((pol[0].x+nx-p0->x)*nx +
				  (pol[0].y+ny-p0->y)*ny +
				  (pol[0].z+nz-p0->z)*nz) / t;
			np.x = v0->x*t + p0->x - nx;
			np.y = v0->y*t + p0->y - ny;
			np.z = v0->z*t + p0->z - nz;

			if ((fabs(nx) > fabs(ny)) && (fabs(nx) > fabs(nz))) maxnormaxis = 0;
			else if (fabs(ny) > fabs(nz)) maxnormaxis = 1; else maxnormaxis = 2;

			for(i=n-1,j=k=0;j<n;i=j,j++)
			{
				if (maxnormaxis > 0) { x0 = pol[i].x - np.x; x1 = pol[j].x - np.x; }
									 else { x0 = pol[i].y - np.y; x1 = pol[j].y - np.y; }
				if (maxnormaxis > 1) { y0 = pol[i].y - np.y; y1 = pol[j].y - np.y; }
									 else { y0 = pol[i].z - np.z; y1 = pol[j].z - np.z; }
				if (y0*y1 < 0.0)
				{
					if (x0*x1 >= 0.0) { if (x0 < 0.0) k++; }
					else if ((x0*y1 - x1*y0)*y1 < 0.0) k++;
				}
			}
			if ((k&1) && (t > 0) && (t < mint)) { mint = t; (*hit) = np; }
		}
	}
#endif

	for(i=0;i<n;i++)
	{
		j = i+1; if (j >= n) j = 0;
			//Raytrace to edges (cylinders)
			//ix = t*v0->x+p0->x  (ix,iy,iz)
			//iy = t*v0->y+p0->y     /�
			//iz = t*v0->z+p0->z   c` cr
			//                   /`   �
			//           p->v[i]���a���������p->v[j]
			//
			//a = ((ix-p->v[i].x)*d.x + (iy-p->v[i].y)*dy + (iz-p->v[i].z)*dz) / sqrt(d.x*d.x + dy*dy + dz*dz)
			//c = sqrt((ix-p->v[i].x)^2 + (iy-p->v[i].y)^2 + (iz-p->v[i].z)^2)
			//a*a + cr*cr = c*c
			//0 <= t < mint
			//
			//((t*v0->x+ex)*d.x+(t*v0->y+e.y)*d.y+(t*v0->z+e.z)*d.z)^2-((t*v0->x+e.x)^2+(t*v0->y+e.y)^2+(t*v0->z+e.z)^2-cr^2)*v
		fd.x = pol[j].x-pol[i].x; e.x = p0->x-pol[i].x;
		fd.y = pol[j].y-pol[i].y; e.y = p0->y-pol[i].y;
		fd.z = pol[j].z-pol[i].z; e.z = p0->z-pol[i].z;
		ff.x = fd.x*fd.x; ff.y = fd.y*fd.y; ff.z = fd.z*fd.z; v = ff.x+ff.y+ff.z;
		ff.x -= v; fg.z = fd.x*fd.y;
		ff.y -= v; fg.y = fd.x*fd.z;
		ff.z -= v; fg.x = fd.y*fd.z;
		Za = ff.x*v0->x*v0->x + ff.y*v0->y*v0->y + ff.z*v0->z*v0->z + (fg.z* v0->x*v0->y          + fg.y* v0->x*v0->z          + fg.x* v0->y*v0->z         )*2;
		Zb = ff.x*e.x*v0->x   + ff.y*e.y*v0->y   + ff.z*e.z*v0->z   + (fg.z*(e.x*v0->y+e.y*v0->x) + fg.y*(e.x*v0->z+e.z*v0->x) + fg.x*(e.y*v0->z+e.z*v0->y))  ;
		Zc = ff.x*e.x*e.x     + ff.y*e.y*e.y     + ff.z*e.z*e.z     + (fg.z* e.x*e.y              + fg.y* e.x*e.z              + fg.x* e.y*e.z             )*2;
		Zc += cr*cr*v;
		u = Zb*Zb - Za*Zc; if ((u < 0.0) || (Za == 0.0)) continue;
	 //t = (sqrt(u)-Zb) / Za;  //Classic quadratic equation (Zb premultiplied by .5)
		t = -Zc / (sqrt(u)+Zb); //Alternate quadratic equation (Zb premultiplied by .5)
		if ((t < 0.0) || (t >= mint)) continue;
		u = (t*v0->x+e.x)*fd.x + (t*v0->y+e.y)*fd.y + (t*v0->z+e.z)*fd.z; if ((u < 0) || (u >= v)) continue;
		mint = t; u /= v;
		hit->x = fd.x*u + pol[i].x;
		hit->y = fd.y*u + pol[i].y;
		hit->z = fd.z*u + pol[i].z;
	}

		//Raytrace to vertices (sphere)
		//ix = t*v0->x + p0->x
		//iy = t*v0->y + p0->y
		//iz = t*v0->z + p0->z
		//(px-ix)^2 + (py-iy)^2 + (pz-iz)^2 = cr*cr
		//0 <= t < mint
		//
		//ex = p0->x-px;
		//ey = p0->y-py;
		//ez = p0->z-pz;
		//(t*v0->x + ex)^2 +
		//(t*v0->y + ey)^2 +
		//(t*v0->z + ez)^2 = cr*cr
		//
		//t*t*v0->x^2 + t*v0->x*ex*2 + ex*ex
		//t*t*v0->y^2 + t*v0->y*ey*2 + ey*ey
		//t*t*v0->z^2 + t*v0->z*ez*2 + ez*ez - cr*cr = 0
	for(i=0;i<n;i++)
	{
		e.x = p0->x-pol[i].x; e.y = p0->y-pol[i].y; e.z = p0->z-pol[i].z;
		Za = v0->x*v0->x + v0->y*v0->y + v0->z*v0->z; //FIX:optimize
		Zb = v0->x*e.x + v0->y*e.y + v0->z*e.z;
		Zc = e.x*e.x + e.y*e.y + e.z*e.z - cr*cr;
		u = Zb*Zb - Za*Zc; if ((u < 0.0) || (Za == 0.0)) continue;
	 //t = -(sqrt(u)+Zb) / Za; //Classic quadratic equation (Zb premultiplied by .5)
		t = Zc / (sqrt(u)-Zb); //Alternate quadratic equation (Zb premultiplied by .5)
		if ((t < 0.0) || (t >= mint)) continue;
		mint = t; (*hit) = pol[i];
	}

	return(mint);
}

double sphtracewall (dpoint3d *p0, dpoint3d *v0, double cr, int s, int w, int s0, int cf0, int s1, int cf1, dpoint3d *hit, mapstate_t* map)
{
	sect_t *sec;
	wall_t *wal;
	dpoint3d pol[4], npol[8];
	int n, nw;

	sec = map->sect; wal = sec[s].wall; nw = wal[w].n+w;
	pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].z = getslopez(&sec[s0],cf0,wal[ w].x,wal[ w].y);
	pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].z = getslopez(&sec[s0],cf0,wal[nw].x,wal[nw].y);
	pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].z = getslopez(&sec[s1],cf1,wal[nw].x,wal[nw].y);
	pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].z = getslopez(&sec[s1],cf1,wal[ w].x,wal[ w].y);
	n = wallclip(pol,npol); if (!n) return(1.0);
	return(sphpolydist(p0,v0,cr,npol,n,0,hit));
}

double sphtracerec (dpoint3d *p0, dpoint3d *v0, dpoint3d *hit, int *cursect, double cr, mapstate_t* map)
{
	dpoint3d np, nhit, pol[4];
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	sect_t *sec;
	wall_t *wal, *wal2;
	spri_t *spr;
	double d, f, g, t, u, v, wx, wy, wz, nx, ny, nz, mint, Za, Zb, Zc;
	long *gotsect;
	int i, j, k, s, w, nw, bs, bw, vn, s0, s1, cf0, cf1, *secfif, secfifw, secfifr;

	if ((unsigned)(*cursect) >= (unsigned)map->numsects) return(1.0);

	i = (((map->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(i);
	memset(gotsect,0,i);
	gotsect[(*cursect)>>5] |= (1<<(*cursect));

	secfif = (int *)_alloca(map->numsects*sizeof(secfif[0]));
	secfif[0] = (*cursect); secfifr = 0; secfifw = 1;

	mint = 1.0; hit->x = hit->y = hit->z = -17.0;

	sec = map->sect;
	while (secfifr < secfifw)
	{
		s = secfif[secfifr]; secfifr++;

			//Raytrace to planes
			//ix = t*v0->x + p0->x
			//iy = t*v0->y + p0->y
			//iz = t*v0->z + p0->z
			//(ix-wx)*nx + (iy-wy)*ny + (iz-wz)*nz = cr
			//
			//t = (cr - ((p0->x-wx)*nx + (p0->y-wy)*ny + (p0->z-wz)*nz)) / (v0->x*nx + v0->y*ny + v0->z*nz)
		wal = sec[s].wall;
		for(i=2-1;i>=0;i--) //Collide ceilings/floors
		{
			nx = sec[s].grad[i].x; ny = sec[s].grad[i].y; nz = 1.0; //plane's normal
			if (i) { nx = -nx; ny = -ny; nz = -nz; }
			t = v0->x*nx + v0->y*ny + v0->z*nz; if (t >= 0.0) continue;
			f = 1.0/sqrt(nx*nx + ny*ny + 1.0); nx *= f; ny *= f; nz *= f; t *= f; //FIX:optimize
			wx = wal[0].x; wy = wal[0].y; wz = sec[s].z[i]; //point on plane
			f = (p0->x-wx)*nx + (p0->y-wy)*ny + (p0->z-wz)*nz; if (f < 0) continue;
			t = (cr-f)/t;
			np.x = v0->x*t + p0->x - nx*cr;
			np.y = v0->y*t + p0->y - ny*cr;
			np.z = v0->z*t + p0->z - nz*cr;
			if (!insidesect(np.x,np.y,sec[s].wall,sec[s].n)) continue;
			if ((t >= 0) && (t < mint))
			{
				mint = t; (*hit) = np;
				build2.clipsect[build2.cliphitnum] = s;
				build2.clipwall[build2.cliphitnum] = i-2;
			}
		}

		for(w=0;w<sec[s].n;w++)
		{
			nw = wal[w].n+w;
			vn = getwalls_imp(s,w,verts,MAXVERTS, map);
			if (wal[w].surf.flags&1) vn = 0; //Blocking wall

			s0 = s; cf0 = 0;
			for(k=0;k<=vn;k++) //Warning: do not reverse for loop direction!
			{
				if (k)
				{
					s0 = s1; cf0 = 1;
					if (!(gotsect[s0>>5]&(1<<s0)))
					{
						d = distpoint2line2(p0->x,p0->y,wal[w].x,wal[w].y,wal[nw].x,wal[nw].y);
						if ((d < cr*cr) || (sphtracewall(p0,v0,cr,s,w,s1,cf1,s0,cf0,&nhit, map) < mint))
							{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
						else
						{
								//FIX:Should only test these sectors for sprites...
							d = roundcylminpath2(p0->x,p0->y,p0->x+v0->x,p0->y+v0->y,wal[w].x,wal[w].y,wal[nw].x,wal[nw].y);
							if (d < (build2.fattestsprite+cr)*(build2.fattestsprite+cr))
								{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
						}
					}
				}
				if (k < vn) { s1 = verts[k].s; cf1 = 0; } else { s1 = s; cf1 = 1; }
				d = sphtracewall(p0,v0,cr,s,w,s0,cf0,s1,cf1,&nhit,map);
				if (d < mint)
				{
					mint = d; (*hit) = nhit;
					build2.clipsect[build2.cliphitnum] = s;
					build2.clipwall[build2.cliphitnum] = w;
				}
			}
		}

		spr = map->spri;
		for(w=sec[s].headspri;w>=0;w=spr[w].sectn)
		{
			//if (spr[w].owner >= 0) continue;
			if (!(spr[w].flags&1)) continue;

			if (spr[w].fat > 0.f)
			{
					//Raytrace to sphere
				np.x = p0->x-spr[w].p.x;
				np.y = p0->y-spr[w].p.y;
				np.z = p0->z-spr[w].p.z;
				Za = v0->x*v0->x + v0->y*v0->y + v0->z*v0->z; //FIX:optimize
				Zb = v0->x*np.x + v0->y*np.y + v0->z*np.z;
				d = np.x*np.x + np.y*np.y + np.z*np.z;
				Zc = d - (spr[w].fat+cr)*(spr[w].fat+cr);
				u = Zb*Zb - Za*Zc; if ((u < 0.0) || (Za == 0.0)) continue;
			 //t = -(sqrt(u)+Zb) / Za; //Classic quadratic equation (Zb premultiplied by .5)
				t = Zc / (sqrt(u)-Zb); //Alternate quadratic equation (Zb premultiplied by .5)
				if ((t < 0.0) || (t >= mint)) continue;
				mint = t;
				if (d != 0) d = spr[w].fat/sqrt(d);
				hit->x = spr[w].p.x + np.x*d;
				hit->y = spr[w].p.y + np.y*d;
				hit->z = spr[w].p.z + np.z*d;
				build2.clipsect[build2.cliphitnum] = s;
				build2.clipwall[build2.cliphitnum] = w+0x40000000;
			}
			else //Flat polygon (wall/floor sprite)
			{
				for(i=4-1;i>=0;i--)
				{
					if ((i+1)&2) f = 1; else f = -1;
					if ((i  )&2) g = 1; else g = -1;
					pol[i].x = spr[w].p.x + spr[w].r.x*f + spr[w].d.x*g;
					pol[i].y = spr[w].p.y + spr[w].r.y*f + spr[w].d.y*g;
					pol[i].z = spr[w].p.z + spr[w].r.z*f + spr[w].d.z*g;
				}
				j = 0;
				if (!(spr[w].flags&64)) j |= 1; //select 1/2-sided collision
				else if (spr[w].flags&4) { np = pol[0]; pol[0] = pol[2]; pol[2] = np; } //Mirrored: use other side
				t = sphpolydist(p0,v0,cr,pol,4,j,&nhit);
				if ((t < 0.0) || (t >= mint)) continue;
				mint = t;
				(*hit) = nhit;
				build2.clipsect[build2.cliphitnum] = s;
				build2.clipwall[build2.cliphitnum] = w+0x40000000;
			}
		}
	}
	return(mint);
}

	//Returns: 1 if no obstacles, 0 if hit something
long sphtrace (dpoint3d *p0,  //start pt
							 dpoint3d *v0,  //move vector
							 dpoint3d *hit, //pt causing collision
							 int *cursect,
							 double cr, mapstate_t* gst)
{
	dpoint3d dp;
	double mint;

	if ((v0->x == 0.0) && (v0->y == 0.0) && (v0->z == 0.0)) return(1);

	mint = sphtracerec(p0,v0,hit,cursect,cr,gst);
	dp.x = v0->x*mint; p0->x += dp.x; v0->x -= dp.x;
	dp.y = v0->y*mint; p0->y += dp.y; v0->y -= dp.y;
	dp.z = v0->z*mint; p0->z += dp.z; v0->z -= dp.z;
	return(mint == 1.0);
}

