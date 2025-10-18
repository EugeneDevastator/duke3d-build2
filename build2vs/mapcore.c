//This version also handles u&v. Note: Input should still be simple wall quad
#include "mapcore.h"
tile_t *gtile;
long gnumtiles, gmaltiles, gtilehashead[1024];

long wallclippol (kgln_t *pol, kgln_t *npol)
{
	double f, dz0, dz1;

	dz0 = pol[3].z-pol[0].z; dz1 = pol[2].z-pol[1].z;
	if (dz0 > 0.0) //do not include null case for rendering
	{
		npol[0] = pol[0];
		if (dz1 > 0.0) //do not include null case for rendering
		{
			npol[1] = pol[1];
			npol[2] = pol[2];
			npol[3] = pol[3];
			npol[0].n = npol[1].n = npol[2].n = 1; npol[3].n = -3;
			return(4);
		}
		else
		{
			f = dz0/(dz0-dz1);
			npol[1].x = (pol[1].x-pol[0].x)*f + pol[0].x;
			npol[1].y = (pol[1].y-pol[0].y)*f + pol[0].y;
			npol[1].z = (pol[1].z-pol[0].z)*f + pol[0].z;
			npol[1].u = (pol[1].u-pol[0].u)*f + pol[0].u;
			npol[1].v = (pol[1].v-pol[0].v)*f + pol[0].v;
			npol[2] = pol[3];
			npol[0].n = npol[1].n = 1; npol[2].n = -2;
			return(3);
		}
	}
	if (dz1 <= 0.0) return(0); //do not include null case for rendering
	f = dz0/(dz0-dz1);
	npol[0].x = (pol[1].x-pol[0].x)*f + pol[0].x;
	npol[0].y = (pol[1].y-pol[0].y)*f + pol[0].y;
	npol[0].z = (pol[1].z-pol[0].z)*f + pol[0].z;
	npol[0].u = (pol[1].u-pol[0].u)*f + pol[0].u;
	npol[0].v = (pol[1].v-pol[0].v)*f + pol[0].v;
	npol[1] = pol[1];
	npol[2] = pol[2];
	npol[0].n = npol[1].n = 1; npol[2].n = -2;
	return(3);
}


//Find shortest path between two 3D line segments
//Input: 2 line segments: (a0)-(a1), (b0)-(b1)
//Returns: distance^2 between closest points
double roundcylminpath2 (double a0x, double a0y, double a1x, double a1y,
								 double b0x, double b0y, double b1x, double b1y)
{
	dpoint3d da, db, ab;
	double k0, k1, k2, k3, k4, d, t, u;

	da.x = a1x-a0x; db.x = b1x-b0x; ab.x = b0x-a0x;
	da.y = a1y-a0y; db.y = b1y-b0y; ab.y = b0y-a0y;
	k0 = da.x*da.x + da.y*da.y;
	k1 = db.x*db.x + db.y*db.y;
	k2 = da.x*db.x + da.y*db.y;
	k3 = da.x*ab.x + da.y*ab.y;
	k4 = db.x*ab.x + db.y*ab.y;

	if (k0 == 0)
	{
		if (k1 == 0) { t = u = 0; }
		else { t = 0; u = -k4/k1; u = min(max(u,0),1); }
	}
	else if (k1 == 0) { u = 0; t = k3/k0; t = min(max(t,0),1); }
	else
	{
		d = k0*k1 - k2*k2;
		t = k1*k3 - k2*k4;
		u = k2*k3 - k0*k4;
		d = 1.0/d; t *= d; u *= d;
		if ((fabs(t-.5) > .5) || (fabs(u-.5) > .5))
		{
			u = min(max(u,0),1);
			t = (k2*u + k3)/k0; t = min(max(t,0),1);
			u = (k2*t - k4)/k1; u = min(max(u,0),1);
		}
	}
	ab.x += db.x*u - da.x*t;
	ab.y += db.y*u - da.y*t;
	return(ab.x*ab.x + ab.y*ab.y);
}