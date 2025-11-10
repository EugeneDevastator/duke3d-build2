//
// Created by omnis on 11/4/2025.
//
#include "menues.h"

void cmenu(short cm)
{

}

void savetemp(char* fn, long daptr, long dasiz)
{

}

void getangplayers(short snum)
{
    short i,a;

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        if(i != snum)
        {
            a = ps[snum].ang+getangle(ps[i].posx-ps[snum].posx,ps[i].posy-ps[snum].posy);
            a = a-1024;
            rotatesprite(
                (320<<15) + (((sintable[(a+512)&2047])>>7)<<15),
                (320<<15) - (((sintable[a&2047])>>8)<<15),
                klabs(sintable[((a>>1)+768)&2047]<<2),0,APLAYER,0,ps[i].palookup,0,0,0,xdim-1,ydim-1);
        }
    }
}

int loadpheader(char spot, int32_t* vn, int32_t* ln, int32_t* psk, int32_t* numplr)
{
    return 0;
}

int loadplayer(signed char spot)
{

    return(0);
}

int saveplayer(signed char spot)
{


    return(0);
}

int probe(int x, int y, int i, int n)
{

    probey = 0;


}

int menutext(int x, int y, short s, short p, char* t)
{
    puts(t);
    return;//
    short i, ac, centre;

    y -= 12;

    i = centre = 0;

    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM-10;
            else if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = toupper(*(t+i)) - 'A' + BIGALPHANUM;
            else if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;
            else switch(*(t+i))
            {
            case '-':
                ac = BIGALPHANUM-11;
                break;
            case '.':
                ac = BIGPERIOD;
                break;
            case '\'':
                ac = BIGAPPOS;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGSEMI;
                break;
            default:
                centre += 5;
                i++;
                continue;
            }

            centre += tilesizx[ac]-1;
            i++;
        }
    }

    if(centre)
        x = (320-centre-10)>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM-10;
        else if(*t >= 'a' && *t <= 'z')
            ac = toupper(*t) - 'A' + BIGALPHANUM;
        else if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        else switch(*t)
        {
        case '-':
            ac = BIGALPHANUM-11;
            break;
        case '.':
            ac = BIGPERIOD;
            break;
        case ',':
            ac = BIGCOMMA;
            break;
        case '!':
            ac = BIGX;
            break;
        case '\'':
            ac = BIGAPPOS;
            break;
        case '?':
            ac = BIGQ;
            break;
        case ';':
            ac = BIGSEMI;
            break;
        case ':':
            ac = BIGCOLIN;
            break;
        default:
            x += 5;
            t++;
            continue;
        }

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac];
        t++;
    }
    return (x);
}

int menutextc(int x, int y, short s, short p, char* t)
{
    short i, ac, centre;

    s += 8;
    y -= 12;

    i = centre = 0;

    //    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM+26+26;
            if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = *(t+i) - 'a' + BIGALPHANUM+26;
            if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;

            else switch(*t)
            {
            case '-':
                ac = BIGALPHANUM-11;
                break;
            case '.':
                ac = BIGPERIOD;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGCOLIN;
                break;
            }

            centre += tilesizx[ac]-1;
            i++;
        }
    }

    x -= centre>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM+26+26;
        if(*t >= 'a' && *t <= 'z')
            ac = *t - 'a' + BIGALPHANUM+26;
        if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        switch(*t)
        {
        case '-':
            ac = BIGALPHANUM-11;
            break;
        case '.':
            ac = BIGPERIOD;
            break;
        case ',':
            ac = BIGCOMMA;
            break;
        case '!':
            ac = BIGX;
            break;
        case '?':
            ac = BIGQ;
            break;
        case ';':
            ac = BIGSEMI;
            break;
        case ':':
            ac = BIGCOLIN;
            break;
        }

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac];
        t++;
    }
    return (x);
}

void bar(int x, int y, short* p, short dainc, char damodify, short s, short pa)
{
    short xloc;
    char rev;

    if(dainc < 0) { dainc = -dainc; rev = 1; }
    else rev = 0;
    y-=2;

    if(damodify)
    {
        if(rev == 0)
        {
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ) ) // && onbar) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );

                *p -= dainc;
                if(*p < 0)
                    *p = 0;
                sound(KICK_HIT);
            }
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ) )//&& onbar) )
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );

                *p += dainc;
                if(*p > 63)
                    *p = 63;
                sound(KICK_HIT);
            }
        }
        else
        {
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ))//&& onbar ))
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );

                *p -= dainc;
                if(*p < 0)
                    *p = 0;
                sound(KICK_HIT);
            }
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ))// && onbar) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );

                *p += dainc;
                if(*p > 64)
                    *p = 64;
                sound(KICK_HIT);
            }
        }
    }

    xloc = *p;

    rotatesprite( (x+22)<<16,(y-3)<<16,65536L,0,SLIDEBAR,s,pa,10,0,0,xdim-1,ydim-1);
    if(rev == 0)
        rotatesprite( (x+xloc+1)<<16,(y+1)<<16,65536L,0,SLIDEBAR+1,s,pa,10,0,0,xdim-1,ydim-1);
    else
        rotatesprite( (x+(65-xloc) )<<16,(y+1)<<16,65536L,0,SLIDEBAR+1,s,pa,10,0,0,xdim-1,ydim-1);
}

void dispnames()
{
    short x, c = 160;

    c += 64;
    for(x = 0;x <= 108;x += 12)
        rotatesprite((c+91-64)<<16,(x+56)<<16,65536L,0,TEXTBOX,24,0,10,0,0,xdim-1,ydim-1);

    rotatesprite(22<<16,97<<16,65536L,0,WINDOWBORDER2,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(180<<16,97<<16,65536L,1024,WINDOWBORDER2,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(99<<16,50<<16,65536L,512,WINDOWBORDER1,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(103<<16,144<<16,65536L,1024+512,WINDOWBORDER1,24,0,10,0,0,xdim-1,ydim-1);

    minitext(c,48,ud.savegame[0],2,10+16);
    minitext(c,48+12,ud.savegame[1],2,10+16);
    minitext(c,48+12+12,ud.savegame[2],2,10+16);
    minitext(c,48+12+12+12,ud.savegame[3],2,10+16);
    minitext(c,48+12+12+12+12,ud.savegame[4],2,10+16);
    minitext(c,48+12+12+12+12+12,ud.savegame[5],2,10+16);
    minitext(c,48+12+12+12+12+12+12,ud.savegame[6],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12,ud.savegame[7],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12+12,ud.savegame[8],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12+12+12,ud.savegame[9],2,10+16);

}

int getfilenames(char kind[])
{

    return(0);
}

void sortfilenames()
{

}

void menus()
{

}


void endanimsounds(long fr)
{
    switch(ud.volume_number)
    {
    case 0:break;
    case 1:
        switch(fr)
        {
        case 1:
            sound(WIND_AMBIENCE);
            break;
        case 26:
            sound(ENDSEQVOL2SND1);
            break;
        case 36:
            sound(ENDSEQVOL2SND2);
            break;
        case 54:
            sound(THUD);
            break;
        case 62:
            sound(ENDSEQVOL2SND3);
            break;
        case 75:
            sound(ENDSEQVOL2SND4);
            break;
        case 81:
            sound(ENDSEQVOL2SND5);
            break;
        case 115:
            sound(ENDSEQVOL2SND6);
            break;
        case 124:
            sound(ENDSEQVOL2SND7);
            break;
        }
        break;
    case 2:
        switch(fr)
        {
        case 1:
            sound(WIND_REPEAT);
            break;
        case 98:
            sound(DUKE_GRUNT);
            break;
        case 82+20:
            sound(THUD);
            sound(SQUISHED);
            break;
        case 104+20:
            sound(ENDSEQVOL3SND3);
            break;
        case 114+20:
            sound(ENDSEQVOL3SND2);
            break;
        case 158:
            sound(PIPEBOMB_EXPLODE);
            break;
        }
        break;
    }
}

void logoanimsounds(long fr)
{
    switch(fr)
    {
    case 1:
        sound(FLY_BY);
        break;
    case 19:
        sound(PIPEBOMB_EXPLODE);
        break;
    }
}

void intro4animsounds(long fr)
{
    switch(fr)
    {
    case 1:
        sound(INTRO4_B);
        break;
    case 12:
    case 34:
        sound(SHORT_CIRCUIT);
        break;
    case 18:
        sound(INTRO4_5);
        break;
    }
}

void first4animsounds(long fr)
{
    switch(fr)
    {
    case 1:
        sound(INTRO4_1);
        break;
    case 12:
        sound(INTRO4_2);
        break;
    case 7:
        sound(INTRO4_3);
        break;
    case 26:
        sound(INTRO4_4);
        break;
    }
}

void intro42animsounds(long fr)
{
    switch(fr)
    {
    case 10:
        sound(INTRO4_6);
        break;
    }
}

void endanimvol41(long fr)
{
    switch(fr)
    {
    case 3:
        sound(DUKE_UNDERWATER);
        break;
    case 35:
        sound(VOL4ENDSND1);
        break;
    }
}

void endanimvol42(long fr)
{
    switch(fr)
    {
    case 11:
        sound(DUKE_UNDERWATER);
        break;
    case 20:
        sound(VOL4ENDSND1);
        break;
    case 39:
        sound(VOL4ENDSND2);
        break;
    case 50:
        FX_StopAllSounds();
        break;
    }
}

void endanimvol43(long fr)
{
    switch(fr)
    {
    case 1:
        sound(BOSS4_DEADSPEECH);
        break;
    case 40:
        sound(VOL4ENDSND1);
        sound(DUKE_UNDERWATER);
        break;
    case 50:
        sound(BIGBANG);
        break;
    }
}

void playanm(char* fn, char t)
{

}
