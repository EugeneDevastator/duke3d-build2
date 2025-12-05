//
// Created by omnis on 11/25/2025.
//

#include "scenerender.h"

#include <stdbool.h>

#include "shadowtest2.h"
#include "mapcore.h"
static bool useLights = false;

void setLightOption(bool isenabled) {
	useLights=isenabled;
}
void drawframe (cam_t *cc, player_transform *gdps, mapstate_t *gst) // draws scene
{
	cam_t cam;
	dpoint3d dp, dr, dd, df;
	long i, j, k, l, m, flashlight1st;

//	if (!moveindex) gst = &sst; else gst = &pst;
	//gdps = &gst->p[viewindex];
//	memcpy(&cps,&gst->p[viewindex],sizeof(playerstruct_t)); gdps = &cps; //FIX

//	if (((!useLights) || (gdps->cursect < 0)) || (gdps->compact2d > 0.0))
//	for(i=0,j=cc->z.f;i<cc->c.y;i++,j+=cc->z.p) memset8((void *)j,0x7f7f7f7f,cc->c.x<<2);

	dp.x = 0.0; dp.y = 0.0; dp.z = 0.0;
	dr.x = 1.0; dr.y = 0.0; dr.z = 0.0;
	dd.x = 0.0; dd.y = 1.0; dd.z = 0.0;
	df.x = 0.0; df.y = 0.0; df.z = 1.0;
	//redundant?
//	drawpoly_setup(                           (tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,gdps->ghx,gdps->ghy,gdps->ghz);
//	drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,gdps->ghx,gdps->ghy,gdps->ghz);
//	 drawkv6_setup(&drawkv6_frame,            (tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,gdps->ghx,gdps->ghy,gdps->ghz);

	cam.h.x = gdps->ghx; cam.h.y = gdps->ghy; cam.h.z = gdps->ghz;
	//cam.h.x = gst->p[moveindex].ghx; //Needs many internal hacks! :/
	//cam.h.y = gst->p[moveindex].ghy;
	//cam.h.z = gst->p[moveindex].ghx * gdps->ghz/gdps->ghx;
	cam.c = cc->c; cam.z = cc->z;

	if ((!useLights) || (gdps->cursect < 0))
	{
		cam.r.x = 1.f; cam.r.y = 0.f; cam.r.z = 0.f;
		cam.d.x = 0.f; cam.d.y = 1.f; cam.d.z = 0.f;
		cam.f.x = 0.f; cam.f.y = 0.f; cam.f.z = 1.f;
		cam.p.x = 0.f; cam.p.y = 0.f; cam.p.z = 0.f;
	//	drawkv6_numlights = -1;
	//	drawview(&cam,gdps,0);
	}
	else
	{
		cam.r.x = gdps->irig.x; cam.r.y = gdps->irig.y; cam.r.z = gdps->irig.z;
		cam.d.x = gdps->idow.x; cam.d.y = gdps->idow.y; cam.d.z = gdps->idow.z;
		cam.f.x = gdps->ifor.x; cam.f.y = gdps->ifor.y; cam.f.z = gdps->ifor.z;
		cam.p.x = gdps->ipos.x; cam.p.y = gdps->ipos.y; cam.p.z = gdps->ipos.z;

		if (true)//gdps->compact2d == 0.0)
		{   // Main render scope
			shadowtest2_useshadows = 1;//b2opts.shadows;
			shadowtest2_numlights = 0;
			for(i=gst->light_sprinum-1;i>=0;i--)
			{
				if (((unsigned)gst->light_spri[i] < (unsigned)gst->malspris) && (gst->spri[gst->light_spri[i]].sect >= 0) && (shadowtest2_numlights < MAXLIGHTS))
				{
					shadowtest2_light[shadowtest2_numlights].sect   = gst->spri[gst->light_spri[i]].sect;
					shadowtest2_light[shadowtest2_numlights].p      = gst->spri[gst->light_spri[i]].p;
					k = ((gst->spri[gst->light_spri[i]].flags>>17)&7);
					if (!k) { shadowtest2_light[shadowtest2_numlights].spotwid = -1.0; }
					else
					{
						m = ((gst->spri[gst->light_spri[i]].flags>>20)&1023); if (!m) continue;
						shadowtest2_light[shadowtest2_numlights].spotwid = cos(m*PI/1024.0); //FIX:use lut
						switch(k)
						{
							case 1: case 2: shadowtest2_light[shadowtest2_numlights].f = gst->spri[gst->light_spri[i]].d; break;
							case 3: case 4: shadowtest2_light[shadowtest2_numlights].f = gst->spri[gst->light_spri[i]].f; break;
							case 5: case 6: shadowtest2_light[shadowtest2_numlights].f = gst->spri[gst->light_spri[i]].r; break;
						}
						if (!(k&1)) { shadowtest2_light[shadowtest2_numlights].f.x *= -1; shadowtest2_light[shadowtest2_numlights].f.y *= -1; shadowtest2_light[shadowtest2_numlights].f.z *= -1; }
					}
					shadowtest2_light[shadowtest2_numlights].rgb[0] = gst->spri[gst->light_spri[i]].bsc/8192.f; //gsc/8192   gst->spri[gst->light_spri[i]].fat;
					shadowtest2_light[shadowtest2_numlights].rgb[1] = gst->spri[gst->light_spri[i]].gsc/8192.f;
					shadowtest2_light[shadowtest2_numlights].rgb[2] = gst->spri[gst->light_spri[i]].rsc/8192.f;
					shadowtest2_light[shadowtest2_numlights].flags  = 1;
					shadowtest2_numlights++;
				}
			}

			flashlight1st = shadowtest2_numlights;
			i=0;
			{
				bool flashl = 1;
				if ((flashl) && (shadowtest2_numlights < MAXLIGHTS))
				{
					shadowtest2_light[shadowtest2_numlights].sect   = gdps->cursect;
					shadowtest2_light[shadowtest2_numlights].p      = gdps->ipos;
					shadowtest2_light[shadowtest2_numlights].f      = gdps->ifor;
					shadowtest2_light[shadowtest2_numlights].rgb[0] = 0.5f;
					shadowtest2_light[shadowtest2_numlights].rgb[1] = 0.5f;
					shadowtest2_light[shadowtest2_numlights].rgb[2] = 0.5f;
					shadowtest2_light[shadowtest2_numlights].spotwid= -1.0;
					shadowtest2_light[shadowtest2_numlights].flags  = (i != 0);
					shadowtest2_numlights++;
				}
			}

		//	drawkv6_numlights = shadowtest2_numlights;
		//	for(i=shadowtest2_numlights-1;i>=0;i--)
		//	{
		//		drawkv6_light[i].sect      = shadowtest2_light[i].sect;
		//		drawkv6_light[i].p         = shadowtest2_light[i].p;
		//		drawkv6_light[i].rgb[0]    = shadowtest2_light[i].rgb[0];
		//		drawkv6_light[i].rgb[1]    = shadowtest2_light[i].rgb[1];
		//		drawkv6_light[i].rgb[2]    = shadowtest2_light[i].rgb[2];
		//		drawkv6_light[i].useshadow =(shadowtest2_light[i].flags&1);
		//		xformpos(&drawkv6_light[i].p.x,&drawkv6_light[i].p.y,&drawkv6_light[i].p.z);
		//	}
		//	drawkv6_ambrgb[0] = shadowtest2_ambrgb[0];
		//	drawkv6_ambrgb[1] = shadowtest2_ambrgb[1];
		//	drawkv6_ambrgb[2] = shadowtest2_ambrgb[2];

			shadowtest2_rendmode = 2;
			draw_hsr_polymost(&cam,gst,gdps,gdps->cursect);
// and here we probably can stop and see eyepoln.

			shadowtest2_rendmode = 4;
			if (shadowtest2_updatelighting) //FIXFIX
			{
				cam_t ncam; ncam = cam;
				shadowtest2_updatelighting = 0; //FIXFIX
				shadowtest2_ligpolreset(-1);
				for(glignum=0;glignum<shadowtest2_numlights;glignum++)
				{
					ncam.p = shadowtest2_light[glignum].p;
					draw_hsr_polymost(&ncam,gst,gdps,shadowtest2_light[glignum].sect);
				}
			}
			else
			{
				cam_t ncam; ncam = cam;
				for(glignum=flashlight1st;glignum<shadowtest2_numlights;glignum++)
				{
					shadowtest2_ligpolreset(glignum);
					ncam.p = shadowtest2_light[glignum].p;
					draw_hsr_polymost(&ncam,gst,gdps,shadowtest2_light[glignum].sect);
				}
			}

			shadowtest2_setcam(&cam);
			// no need for it for now.
			//htrun(drawpollig,0,eyepoln,shadowtest2_numcpu);
			//drawsprites();
		//	drawview(&cam,gdps,1); // draws sprites, huh

		//	drawkv6_numlights = -1;
		}
		else
		{
	//		drawview(&cam,gdps,0);
		}
	}

		//FIX: debug only
	//if (((unsigned)gdps->cursect < (unsigned)gst->numsects) && (gdps->editmode == 3) && (gdps->showdebug))
	//{
	//	dpoint3d dp, clos;
	//	dp.x = (double)gdps->ipos.x;
	//	dp.y = (double)gdps->ipos.y;
	//	dp.z = (double)gdps->ipos.z;
	//	print6x8((tiltyp *)&cam.c,cam.c.x-350*fontscale,48,0xffffff,-1,"maxcr:%.9f, chn=%d",findmaxcr(&dp,gdps->cursect,1e16,&clos),build2.cliphitnum);
	//	if (build2.cliphitnum < 0) drawsph(&cam,clos.x,clos.y,clos.z,.02+rand()/3276800.0,0x808080);
	//}

		//User function must write these:
	cc->p = gdps->ipos; cc->r = gdps->irig; cc->d = gdps->idow; cc->f = gdps->ifor;
	cc->h.x = gdps->ghx; cc->h.y = gdps->ghy; cc->h.z = gdps->ghz;
}
