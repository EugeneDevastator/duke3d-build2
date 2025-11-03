//
// Created by omnis on 11/2/2025.
//
#include "engine.h"

#define patchstatusbar(x1,y1,x2,y2)                                        \
{                                                                          \
rotatesprite(0,(200-34)<<16,65536L,0,BOTTOMSTATUSBAR,4,0,10+16+64+128, \
scale(x1,xdim,320),scale(y1,ydim,200),                             \
scale(x2,xdim,320)-1,scale(y2,ydim,200)-1);                        \
}

void displayinventory(player_struct *p)
{
    short n, j, xoff, y;

    j = xoff = 0;

    n = (p->jetpack_amount > 0)<<3; if(n&8) j++;
    n |= ( p->scuba_amount > 0 )<<5; if(n&32) j++;
    n |= (p->steroids_amount > 0)<<1; if(n&2) j++;
    n |= ( p->holoduke_amount > 0)<<2; if(n&4) j++;
    n |= (p->firstaid_amount > 0); if(n&1) j++;
    n |= (p->heat_amount > 0)<<4; if(n&16) j++;
    n |= (p->boot_amount > 0)<<6; if(n&64) j++;

    xoff = 160-(j*11);

    j = 0;

    if(ud.screen_size > 4)
        y = 154;
    else y = 172;

    if(ud.screen_size == 4)
    {
        if(ud.multimode > 1)
            xoff += 56;
        else xoff += 65;
    }

    while( j <= 9 )
    {
        if( n&(1<<j) )
        {
            switch( n&(1<<j) )
            {
                case   1:
                rotatesprite(xoff<<16,y<<16,65536L,0,FIRSTAID_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   2:
                rotatesprite((xoff+1)<<16,y<<16,65536L,0,STEROIDS_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   4:
                rotatesprite((xoff+2)<<16,y<<16,65536L,0,HOLODUKE_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   8:
                rotatesprite(xoff<<16,y<<16,65536L,0,JETPACK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  16:
                rotatesprite(xoff<<16,y<<16,65536L,0,HEAT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  32:
                rotatesprite(xoff<<16,y<<16,65536L,0,AIRTANK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case 64:
                rotatesprite(xoff<<16,(y-1)<<16,65536L,0,BOOT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
            }

            xoff += 22;

            if(p->inven_icon == j+1)
                rotatesprite((xoff-2)<<16,(y+19)<<16,65536L,1024,ARROW,-32,0,2+16,windowx1,windowy1,windowx2,windowy2);
        }

        j++;
    }
}



void displayfragbar(void)
{
    short i, j;

    j = 0;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if(i > j) j = i;

    rotatesprite(0,0,65600L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 4) rotatesprite(319,(8)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 8) rotatesprite(319,(16)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 12) rotatesprite(319,(24)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        minitext(21+(73*(i&3)),2+((i&28)<<1),&ud.user_name[i][0],sprite[ps[i].i].pal,2+8+16+128);
        sprintf(tempbuf,"%d",ps[i].frag-ps[i].fraggedself);
        minitext(17+50+(73*(i&3)),2+((i&28)<<1),tempbuf,sprite[ps[i].i].pal,2+8+16+128);
    }
}

int gametext(int x,int y,char *t,char s,short dabits)
{
    short ac,newx;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac];
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,dabits,0,0,xdim-1,ydim-1);

        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac];

        t++;
    }

    return (x);
}

int gametextpal(int x,int y,char *t,char s,char p)
{
    short ac,newx;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac];
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,2+8+16,0,0,xdim-1,ydim-1);
        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac];

        t++;
    }

    return (x);
}

int gametextpart(int x,int y,char *t,char s,short p)
{
    short ac,newx, cnt;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;
    cnt = 0;

    if(centre)
    {
        while(*t)
        {
            if(cnt == p) break;

            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            newx += tilesizx[ac];
            t++;
            cnt++;

        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    cnt = 0;
    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

        if(cnt == p)
        {
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,1,2+8+16,0,0,xdim-1,ydim-1);
            break;
        }
        else
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,2+8+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac];

        t++;
        cnt++;
    }

    return (x);
}

int minitext(int x,int y,char *t,char p,char sb)
{
    short ac;

    while(*t)
    {
        *t = toupper(*t);
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + MINIFONT;

        rotatesprite(x<<16,y<<16,65536L,0,ac,0,p,sb,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

int minitextshade(int x,int y,char *t,char s,char p,char sb)
{
    short ac;

    while(*t)
    {
        *t = toupper(*t);
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + MINIFONT;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,sb,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}
void weaponnum(short ind,long x,long y,long num1, long num2,char ha)
{
    char dabuf[80] = {0};

    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-3)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x+9)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    if(num1 > 99) num1 = 99;
    if(num2 > 99) num2 = 99;

    sprintf(dabuf,"%ld",num1);
    if(num1 > 9)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%ld",num2);
    if(num2 > 9)
    {
        rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}

void weaponnum999(char ind,long x,long y,long num1, long num2,char ha)
{
    char dabuf[80] = {0};

    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-4)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%ld",num1);
    if(num1 > 99)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%ld",num2);
    if(num2 > 99)
    {
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num2 > 9)
    {
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}
void weapon_amounts(player_struct *p,long x,long y,long u)
{
     int cw;

     cw = p->curr_weapon;

     if (u&4)
     {
         if (u != 0xffffffff) patchstatusbar(96,178,96+12,178+6);
         weaponnum999(PISTOL_WEAPON,x,y,
                     p->ammo_amount[PISTOL_WEAPON],max_ammo_amount[PISTOL_WEAPON],
                     12-20*(cw == PISTOL_WEAPON) );
     }
     if (u&8)
     {
         if (u != 0xffffffff) patchstatusbar(96,184,96+12,184+6);
         weaponnum999(SHOTGUN_WEAPON,x,y+6,
                     p->ammo_amount[SHOTGUN_WEAPON],max_ammo_amount[SHOTGUN_WEAPON],
                     (!p->gotweapon[SHOTGUN_WEAPON]*9)+12-18*
                     (cw == SHOTGUN_WEAPON) );
     }
     if (u&16)
     {
         if (u != 0xffffffff) patchstatusbar(96,190,96+12,190+6);
         weaponnum999(CHAINGUN_WEAPON,x,y+12,
                      p->ammo_amount[CHAINGUN_WEAPON],max_ammo_amount[CHAINGUN_WEAPON],
                      (!p->gotweapon[CHAINGUN_WEAPON]*9)+12-18*
                      (cw == CHAINGUN_WEAPON) );
     }
     if (u&32)
     {
         if (u != 0xffffffff) patchstatusbar(135,178,135+8,178+6);
         weaponnum(RPG_WEAPON,x+39,y,
                  p->ammo_amount[RPG_WEAPON],max_ammo_amount[RPG_WEAPON],
                  (!p->gotweapon[RPG_WEAPON]*9)+12-19*
                  (cw == RPG_WEAPON) );
     }
     if (u&64)
     {
         if (u != 0xffffffff) patchstatusbar(135,184,135+8,184+6);
         weaponnum(HANDBOMB_WEAPON,x+39,y+6,
                     p->ammo_amount[HANDBOMB_WEAPON],max_ammo_amount[HANDBOMB_WEAPON],
                     (((!p->ammo_amount[HANDBOMB_WEAPON])|(!p->gotweapon[HANDBOMB_WEAPON]))*9)+12-19*
                     ((cw == HANDBOMB_WEAPON) || (cw == HANDREMOTE_WEAPON)));
     }
     if (u&128)
     {
         if (u != 0xffffffff) patchstatusbar(135,190,135+8,190+6);

#ifdef VOLUMEONE
         orderweaponnum(SHRINKER_WEAPON,x+39,y+12,
                     p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
                     (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
                     (cw == SHRINKER_WEAPON) );
#else
         if(p->subweapon&(1<<GROW_WEAPON))
             weaponnum(SHRINKER_WEAPON,x+39,y+12,
                 p->ammo_amount[GROW_WEAPON],max_ammo_amount[GROW_WEAPON],
                 (!p->gotweapon[GROW_WEAPON]*9)+12-18*
                 (cw == GROW_WEAPON) );
         else
             weaponnum(SHRINKER_WEAPON,x+39,y+12,
                 p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
                 (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
                 (cw == SHRINKER_WEAPON) );
#endif
     }
     if (u&256)
     {
         if (u != 0xffffffff) patchstatusbar(166,178,166+8,178+6);

#ifdef VOLUMEONE
        orderweaponnum(DEVISTATOR_WEAPON,x+70,y,
                     p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
                     (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
                     (cw == DEVISTATOR_WEAPON) );
#else
         weaponnum(DEVISTATOR_WEAPON,x+70,y,
                     p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
                     (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
                     (cw == DEVISTATOR_WEAPON) );
#endif
     }
     if (u&512)
     {
         if (u != 0xffffffff) patchstatusbar(166,184,166+8,184+6);
#ifdef VOLUMEONE
         orderweaponnum(TRIPBOMB_WEAPON,x+70,y+6,
                     p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
                     (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
                     (cw == TRIPBOMB_WEAPON) );
#else
         weaponnum(TRIPBOMB_WEAPON,x+70,y+6,
                     p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
                     (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
                     (cw == TRIPBOMB_WEAPON) );
#endif
     }

     if (u&65536L)
     {
         if (u != 0xffffffff) patchstatusbar(166,190,166+8,190+6);
#ifdef VOLUMEONE
        orderweaponnum(-1,x+70,y+12,
                     p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
                     (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
                     (cw == FREEZE_WEAPON) );
#else
         weaponnum(-1,x+70,y+12,
                     p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
                     (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
                     (cw == FREEZE_WEAPON) );
#endif
     }
}


void coolgaugetext(short snum)
{
    return;//draws main menu
    player_struct *p;
    long i, j, o, ss, u;
    char c, permbit;

    p = &ps[snum];

    if (p->invdisptime > 0) displayinventory(p);


    if(ps[snum].gm&MODE_MENU)
        if( (current_menu >= 400  && current_menu <= 405) )
            return;

    ss = ud.screen_size; if (ss < 4) return;

    if ( ud.multimode > 1 && ud.coop != 1 )
    {
        if (pus)
            { displayfragbar(); }
        else
        {
            for(i=connecthead;i>=0;i=connectpoint2[i])
                if (ps[i].frag != sbar.frag[i]) { displayfragbar(); break; }
        }
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != myconnectindex)
                sbar.frag[i] = ps[i].frag;
    }

    if (ss == 4)   //DRAW MINI STATUS BAR:
    {
        rotatesprite(5<<16,(200-28)<<16,65536L,0,HEALTHBOX,0,21,10+16,0,0,xdim-1,ydim-1);
        if (p->inven_icon)
            rotatesprite(69<<16,(200-30)<<16,65536L,0,INVENTORYBOX,0,21,10+16,0,0,xdim-1,ydim-1);

        if(sprite[p->i].pal == 1 && p->last_extra < 2)
            digitalnumber(20,200-17,1,-16,10+16);
        else digitalnumber(20,200-17,p->last_extra,-16,10+16);

        rotatesprite(37<<16,(200-28)<<16,65536L,0,AMMOBOX,0,21,10+16,0,0,xdim-1,ydim-1);

        if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
        digitalnumber(53,200-17,p->ammo_amount[i],-16,10+16);

        o = 158; permbit = 0;
        if (p->inven_icon)
        {
            switch(p->inven_icon)
            {
                case 1: i = FIRSTAID_ICON; break;
                case 2: i = STEROIDS_ICON; break;
                case 3: i = HOLODUKE_ICON; break;
                case 4: i = JETPACK_ICON; break;
                case 5: i = HEAT_ICON; break;
                case 6: i = AIRTANK_ICON; break;
                case 7: i = BOOT_ICON; break;
                default: i = -1;
            }
            if (i >= 0) rotatesprite((231-o)<<16,(200-21)<<16,65536L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);

            minitext(292-30-o,190,"%",6,10+16+permbit);

            j = 0x80000000;
            switch(p->inven_icon)
            {
                case 1: i = p->firstaid_amount; break;
                case 2: i = ((p->steroids_amount+3)>>2); break;
                case 3: i = ((p->holoduke_amount+15)/24); j = p->holoduke_on; break;
                case 4: i = ((p->jetpack_amount+15)>>4); j = p->jetpack_on; break;
                case 5: i = p->heat_amount/12; j = p->heat_on; break;
                case 6: i = ((p->scuba_amount+63)>>6); break;
                case 7: i = (p->boot_amount>>1); break;
            }
            invennum(284-30-o,200-6,(char)i,0,10+permbit);
            if (j > 0) minitext(288-30-o,180,"ON",0,10+16+permbit);
            else if (j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16+permbit);
            if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16+permbit);
        }
        return;
    }

        //DRAW/UPDATE FULL STATUS BAR:

    if (pus) { pus = 0; u = 0xffffffff; } else u = 0;

    if (sbar.frag[myconnectindex] != p->frag) { sbar.frag[myconnectindex] = p->frag; u |= 32768; }
    if (sbar.got_access != p->got_access) { sbar.got_access = p->got_access; u |= 16384; }
    if (sbar.last_extra != p->last_extra) { sbar.last_extra = p->last_extra; u |= 1; }
    if (sbar.shield_amount != p->shield_amount) { sbar.shield_amount = p->shield_amount; u |= 2; }
    if (sbar.curr_weapon != p->curr_weapon) { sbar.curr_weapon = p->curr_weapon; u |= (4+8+16+32+64+128+256+512+1024+65536L); }
    for(i=1;i < 10;i++)
    {
        if (sbar.ammo_amount[i] != p->ammo_amount[i]) {
        sbar.ammo_amount[i] = p->ammo_amount[i]; if(i < 9) u |= ((2<<i)+1024); else u |= 65536L+1024; }
        if (sbar.gotweapon[i] != p->gotweapon[i]) { sbar.gotweapon[i] =
        p->gotweapon[i]; if(i < 9 ) u |= ((2<<i)+1024); else u |= 65536L+1024; }
    }
    if (sbar.inven_icon != p->inven_icon) { sbar.inven_icon = p->inven_icon; u |= (2048+4096+8192); }
    if (sbar.holoduke_on != p->holoduke_on) { sbar.holoduke_on = p->holoduke_on; u |= (4096+8192); }
    if (sbar.jetpack_on != p->jetpack_on) { sbar.jetpack_on = p->jetpack_on; u |= (4096+8192); }
    if (sbar.heat_on != p->heat_on) { sbar.heat_on = p->heat_on; u |= (4096+8192); }
    if (sbar.firstaid_amount != p->firstaid_amount) { sbar.firstaid_amount = p->firstaid_amount; u |= 8192; }
    if (sbar.steroids_amount != p->steroids_amount) { sbar.steroids_amount = p->steroids_amount; u |= 8192; }
    if (sbar.holoduke_amount != p->holoduke_amount) { sbar.holoduke_amount = p->holoduke_amount; u |= 8192; }
    if (sbar.jetpack_amount != p->jetpack_amount) { sbar.jetpack_amount = p->jetpack_amount; u |= 8192; }
    if (sbar.heat_amount != p->heat_amount) { sbar.heat_amount = p->heat_amount; u |= 8192; }
    if (sbar.scuba_amount != p->scuba_amount) { sbar.scuba_amount = p->scuba_amount; u |= 8192; }
    if (sbar.boot_amount != p->boot_amount) { sbar.boot_amount = p->boot_amount; u |= 8192; }
    if (u == 0) return;

    //0 - update health
    //1 - update armor
    //2 - update PISTOL_WEAPON ammo
    //3 - update SHOTGUN_WEAPON ammo
    //4 - update CHAINGUN_WEAPON ammo
    //5 - update RPG_WEAPON ammo
    //6 - update HANDBOMB_WEAPON ammo
    //7 - update SHRINKER_WEAPON ammo
    //8 - update DEVISTATOR_WEAPON ammo
    //9 - update TRIPBOMB_WEAPON ammo
    //10 - update ammo display
    //11 - update inventory icon
    //12 - update inventory on/off
    //13 - update inventory %
    //14 - update keys
    //15 - update kills
    //16 - update FREEZE_WEAPON ammo

    if (u == 0xffffffff)
    {
        patchstatusbar(0,0,320,200);
        if (ud.multimode > 1 && ud.coop != 1)
            rotatesprite(277<<16,(200-27)<<16,65536L,0,KILLSICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
    }
    if (ud.multimode > 1 && ud.coop != 1)
    {
        if (u&32768)
        {
            if (u != 0xffffffff) patchstatusbar(276,183,299,193);
            digitalnumber(287,200-17,max(p->frag-p->fraggedself,0),-16,10+16+128);
        }
    }
    else
    {
        if (u&16384)
        {
            if (u != 0xffffffff) patchstatusbar(275,182,299,194);
            if (p->got_access&4) rotatesprite(275<<16,182<<16,65536L,0,ACCESS_ICON,0,23,10+16+128,0,0,xdim-1,ydim-1);
            if (p->got_access&2) rotatesprite(288<<16,182<<16,65536L,0,ACCESS_ICON,0,21,10+16+128,0,0,xdim-1,ydim-1);
            if (p->got_access&1) rotatesprite(281<<16,189<<16,65536L,0,ACCESS_ICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
        }
    }
    if (u&(4+8+16+32+64+128+256+512+65536L)) weapon_amounts(p,96,182,u);

    if (u&1)
    {
        if (u != 0xffffffff) patchstatusbar(20,183,43,193);
        if(sprite[p->i].pal == 1 && p->last_extra < 2)
            digitalnumber(32,200-17,1,-16,10+16+128);
        else digitalnumber(32,200-17,p->last_extra,-16,10+16+128);
    }
    if (u&2)
    {
        if (u != 0xffffffff) patchstatusbar(52,183,75,193);
        digitalnumber(64,200-17,p->shield_amount,-16,10+16+128);
    }

    if (u&1024)
    {
        if (u != 0xffffffff) patchstatusbar(196,183,219,193);
        if (p->curr_weapon != KNEE_WEAPON)
        {
            if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
            digitalnumber(230-22,200-17,p->ammo_amount[i],-16,10+16+128);
        }
    }

    if (u&(2048+4096+8192))
    {
        if (u != 0xffffffff)
        {
            if (u&(2048+4096)) { patchstatusbar(231,179,265,197); }
                              else { patchstatusbar(250,190,261,195); }
        }
        if (p->inven_icon)
        {
            o = 0; permbit = 128;

            if (u&(2048+4096))
            {
                switch(p->inven_icon)
                {
                    case 1: i = FIRSTAID_ICON; break;
                    case 2: i = STEROIDS_ICON; break;
                    case 3: i = HOLODUKE_ICON; break;
                    case 4: i = JETPACK_ICON; break;
                    case 5: i = HEAT_ICON; break;
                    case 6: i = AIRTANK_ICON; break;
                    case 7: i = BOOT_ICON; break;
                }
                rotatesprite((231-o)<<16,(200-21)<<16,65536L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
                minitext(292-30-o,190,"%",6,10+16+permbit);
                if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16+permbit);
            }
            if (u&(2048+4096))
            {
                switch(p->inven_icon)
                {
                    case 3: j = p->holoduke_on; break;
                    case 4: j = p->jetpack_on; break;
                    case 5: j = p->heat_on; break;
                    default: j = 0x80000000;
                }
                if (j > 0) minitext(288-30-o,180,"ON",0,10+16+permbit);
                else if (j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16+permbit);
            }
            if (u&8192)
            {
                switch(p->inven_icon)
                {
                    case 1: i = p->firstaid_amount; break;
                    case 2: i = ((p->steroids_amount+3)>>2); break;
                    case 3: i = ((p->holoduke_amount+15)/24); break;
                    case 4: i = ((p->jetpack_amount+15)>>4); break;
                    case 5: i = p->heat_amount/12; break;
                    case 6: i = ((p->scuba_amount+63)>>6); break;
                    case 7: i = (p->boot_amount>>1); break;
                }
                invennum(284-30-o,200-6,(char)i,0,10+permbit);
            }
        }
    }
}

