//
// Created by omnis on 11/4/2025.
//

#ifndef GAME_GAME_TEXT_H
#define GAME_GAME_TEXT_H
#define patchstatusbar(x1,y1,x2,y2)                                        \
{                                                                          \
rotatesprite(0,(200-34)<<16,65536L,0,BOTTOMSTATUSBAR,4,0,10+16+64+128, \
scale(x1,xdim,320),scale(y1,ydim,200),                             \
scale(x2,xdim,320)-1,scale(y2,ydim,200)-1);                        \
}
#include "duke3d.h"


void displayfragbar();

int gametext(int x,int y,char *t,char s,short dabits);

int gametextpal(int x,int y,char *t,char s,char p);

int gametextpart(int x,int y,char *t,char s,short p);

int minitext(int x,int y,char *t,char p,char sb);

int minitextshade(int x,int y,char *t,char s,char p,char sb);

void weaponnum(short ind,long x,long y,long num1, long num2,char ha);

void weaponnum999(char ind,long x,long y,long num1, long num2,char ha);

#endif //GAME_GAME_TEXT_H
