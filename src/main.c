#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#define WIDTH 1200
#define HEIGHT 820
#define SCALE 2
#define C(r,g,b) RGB(r,g,b)
static const COLORREF BG=C(247,244,227),PANEL=C(34,64,60),INK=C(31,63,60),MUTED=C(101,128,114),WHITE=C(255,253,238);
static const COLORREF COLORS[4]={C(244,162,111),C(127,192,165),C(166,159,215),C(245,203,96)};
static const COLORREF NET_COLORS[2]={C(42,133,114),C(218,117,88)};
static Game game;
static HDC canvas,frame_dc;
static HBITMAP bitmap,frame_bitmap;
static HGDIOBJ old_bitmap,frame_old_bitmap;
static void *pixels,*frame_pixels;
static BITMAPINFO frame_info;
static int frame_w,frame_h;
static HFONT fonts[7];
static int keys[256],tapped[2],mouse_down,mouse_tap,mouse_mode,muted,hover=-1;
static int client_w=WIDTH,client_h=HEIGHT,mouse_x=255,mouse_y=490;
static HWND window;
static double accumulator;
static LARGE_INTEGER last_tick,frequency;
static unsigned char sounds[3][9000];
static int smoke,smoke_ticks;

static void fill(float x,float y,float w,float h,COLORREF color) {
    HBRUSH b=CreateSolidBrush(color); RECT r={(LONG)x,(LONG)y,(LONG)(x+w),(LONG)(y+h)}; FillRect(canvas,&r,b); DeleteObject(b);
}
static void oval(float x,float y,float rx,float ry,COLORREF color) {
    HBRUSH b=CreateSolidBrush(color); HGDIOBJ old=SelectObject(canvas,b),pen=SelectObject(canvas,GetStockObject(NULL_PEN));
    Ellipse(canvas,(int)(x-rx),(int)(y-ry),(int)(x+rx),(int)(y+ry)); SelectObject(canvas,old); SelectObject(canvas,pen); DeleteObject(b);
}
static void roundbox(float x,float y,float w,float h,float r,COLORREF color) {
    HBRUSH b=CreateSolidBrush(color); HGDIOBJ old=SelectObject(canvas,b),pen=SelectObject(canvas,GetStockObject(NULL_PEN));
    RoundRect(canvas,(int)x,(int)y,(int)(x+w),(int)(y+h),(int)r,(int)r); SelectObject(canvas,old); SelectObject(canvas,pen); DeleteObject(b);
}
static void label(float x,float y,float w,float h,const char *s,int font,COLORREF color,UINT align) {
    RECT r={(LONG)x,(LONG)y,(LONG)(x+w),(LONG)(y+h)}; SelectObject(canvas,fonts[font]); SetTextColor(canvas,color); SetBkMode(canvas,TRANSPARENT);
    DrawTextA(canvas,s,-1,&r,align|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
}
static void line(float x,float y,float xx,float yy,int width,COLORREF color) {
    HPEN p=CreatePen(PS_SOLID,width,color); HGDIOBJ old=SelectObject(canvas,p); MoveToEx(canvas,(int)x,(int)y,NULL); LineTo(canvas,(int)xx,(int)yy); SelectObject(canvas,old); DeleteObject(p);
}
static void ring(float x,float y,float rx,float ry,int width,COLORREF color) {
    HPEN p=CreatePen(PS_SOLID,width,color); HGDIOBJ old=SelectObject(canvas,p),b=SelectObject(canvas,GetStockObject(NULL_BRUSH));
    Ellipse(canvas,(int)(x-rx),(int)(y-ry),(int)(x+rx),(int)(y+ry)); SelectObject(canvas,old);SelectObject(canvas,b);DeleteObject(p);
}
static void curve(POINT *points,int count,int width,COLORREF color) {
    LOGBRUSH lb={BS_SOLID,color,0}; HPEN pen=ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_ROUND|PS_JOIN_ROUND,(DWORD)width,&lb,0,NULL);
    HGDIOBJ old=SelectObject(canvas,pen); PolyBezier(canvas,points,(DWORD)count);SelectObject(canvas,old);DeleteObject(pen);
}
static void butterfly(float x,float y,float size,int color,int gold,float phase) {
    float flap=0.35f+fabsf(sinf(phase))*0.65f,w=size*flap;
    COLORREF c=gold?C(230,168,45):COLORS[color];
    oval(x-w*0.6f,y-size*0.25f,w*0.7f,size*0.63f,c);oval(x+w*0.6f,y-size*0.25f,w*0.7f,size*0.63f,c);
    oval(x-w*0.42f,y+size*0.45f,w*0.55f,size*0.46f,c);oval(x+w*0.42f,y+size*0.45f,w*0.55f,size*0.46f,c);
    oval(x-w*0.66f,y-size*0.35f,w*0.28f,size*0.25f,WHITE);oval(x+w*0.66f,y-size*0.35f,w*0.28f,size*0.25f,WHITE);
    line(x,y-size*0.4f,x-size*0.25f,y-size,1,INK);line(x,y-size*0.4f,x+size*0.25f,y-size,1,INK);
    oval(x,y,2.5f,size*0.65f,INK);
    if(gold) { line(x+size+5,y-8,x+size+5,y-20,2,C(214,154,45));line(x+size-1,y-14,x+size+11,y-14,2,C(214,154,45)); }
}
static void flower(float x,float y,int color) {
    line(x,y,x-3,y+24,2,C(95,145,111));
    for(int i=0;i<5;i++) {float a=i*2*PI/5;oval(x+cosf(a)*5,y+sinf(a)*5,4,4,COLORS[color]);}
    oval(x,y,3,3,WHITE);
}
static void elephant(void) {
    float bob=game.phase==PLAYING?sinf(game.clock*2)*2:0;
    oval(420,713,126,18,C(152,179,143));
    POINT tail[]={{500,651},{555,650},{551,607},{565,620}};curve(tail,4,9,C(94,145,168));oval(567,620,8,11,C(75,119,145));
    oval(420,636+bob,93,76,C(112,166,183));oval(420,623+bob,83,64,C(133,190,202));
    roundbox(340,652+bob,58,57,28,C(111,163,182));roundbox(442,652+bob,58,57,28,C(111,163,182));
    for(int i=0;i<3;i++) {oval(354+i*14,699+bob,6,8,C(198,219,214));oval(456+i*14,699+bob,6,8,C(198,219,214));}
    oval(344,550+bob,59,65,C(94,148,174));oval(496,550+bob,59,65,C(94,148,174));
    oval(342,552+bob,41,49,C(172,200,210));oval(498,552+bob,41,49,C(172,200,210));
    oval(420,551+bob,76,67,C(133,190,202));
    oval(386,548+bob,14,18,WHITE);oval(454,548+bob,14,18,WHITE);
    oval(390,545+bob,6,9,INK);oval(450,545+bob,6,9,INK);
    oval(388,542+bob,2,3,WHITE);oval(448,542+bob,2,3,WHITE);
    oval(367,576+bob,12,7,C(191,199,192));oval(473,576+bob,12,7,C(191,199,192));
    POINT smile[]={{393,588},{403,603},{438,603},{447,588}};curve(smile,4,3,C(72,123,139));
    POINT trunk[]={{420,573},{433,526},{423,503},{423,458},{423,428},{413,391},{420,367}};
    curve(trunk,7,57,C(89,143,164));curve(trunk,7,47,C(150,199,209));
    line(404,411,434,411,2,C(116,169,186));line(408,435,437,435,2,C(116,169,186));line(408,460,438,460,2,C(116,169,186));
    oval(420,365,30,14,C(85,135,154));oval(420,363,23,9,C(44,87,103));
    if(game.puff>0) {float d=(0.35f-game.puff)*60; ring(420,341-d,18+d*0.35f,5,2,C(187,214,196));}
    roundbox(394,638,52,30,12,C(232,211,151));label(394,638,52,30,"E",3,INK,DT_CENTER);
}
static void net(int id) {
    Net *n=&game.nets[id]; float x=n->x,y=n->y,boost=n->swing>0?5*sinf(n->swing/0.28f*PI):0;
    COLORREF c=NET_COLORS[id]; float rx=44+boost,ry=22+boost*0.45f;
    line(x+31,y+19,x+60,y+88,9,C(243,224,177));line(x+31,y+19,x+60,y+88,5,c);
    POINT bag[]={{(LONG)(x-rx),(LONG)y},{(LONG)(x-rx+4),(LONG)(y+65)},{(LONG)(x+rx-4),(LONG)(y+65)},{(LONG)(x+rx),(LONG)y}};
    curve(bag,4,2,c);
    for(int i=-2;i<=2;i++) {float a=(float)i/3;line(x+a*rx,y+6,x+a*rx*0.45f,y+47,1,c);}
    ring(x,y+25,rx*0.70f,13,1,c);ring(x,y+39,rx*0.42f,9,1,c);
    ring(x,y,rx,ry,5,c);ring(x,y,rx-5,ry-4,1,WHITE);
    if(n->swing>0) {line(x-rx-8,y-8,x-rx-15,y-14,2,c);line(x+rx+8,y-8,x+rx+15,y-14,2,c);}
    roundbox(x-22,y-42,44,20,8,c);label(x-22,y-42,44,20,id==0?"P1":game.mode==1?"CPU":"P2",0,WHITE,DT_CENTER);
    if(n->flash>0) label(x-22,y-79+(n->flash/0.45f)*12,44,32,"+1",3,c,DT_CENTER);
}
static void button(int id,int x,int y,int w,int h,const char *text,int active) {
    COLORREF c=active?C(239,193,102):hover==id?C(74,109,93):C(51,85,75);
    roundbox((float)x,(float)y,(float)w,(float)h,14,c);label((float)x,(float)y,(float)w,(float)h,text,2,active?INK:WHITE,DT_CENTER);
}
static int hit(int x,int y) {
    if(x>=857 && x<1127 && y>=214 && y<255) return (x-857)/92;
    if(x>=857 && x<1127 && y>=277 && y<315) return 11;
    if(x>=857 && x<1127 && y>=328 && y<366) return 12;
    if(x>=857 && x<1127 && y>=679 && y<735) return 10;
    if(x>=1064 && x<1152 && y>=49 && y<87) return 13;
    return -1;
}
static void overlay(const char *big,const char *small) {
    roundbox(238,280,364,124,25,PANEL);label(248,291,344,57,big,strlen(big)>4?4:5,WHITE,DT_CENTER);
    label(248,352,344,32,small,1,C(201,219,194),DT_CENTER);
}
static void render(void) {
    fill(0,0,WIDTH,HEIGHT,BG);
    label(48,28,500,70,"elefun",5,INK,DT_LEFT);
    label(51,98,670,22,"LITTLE WINGS. BIG ADVENTURE.",0,MUTED,DT_LEFT);
    butterfly(276,66,17,0,0,1.4f);butterfly(322,48,10,1,0,1.2f);
    label(837,42,215,25,"THE BUTTERFLY CLUB",2,INK,DT_LEFT);
    label(838,73,215,23,"CATCH A LITTLE JOY",0,MUTED,DT_LEFT);
    button(13,1064,49,88,38,muted?"SOUND OFF":"SOUND ON",0);
    roundbox(40,143,757,601,30,C(225,238,218));
    int saved=SaveDC(canvas); IntersectClipRect(canvas,40,143,797,744);
    oval(179,266,169,128,C(217,231,208));oval(640,270,196,165,C(215,231,211));
    oval(390,686,480,96,C(194,215,176));oval(51,726,280,90,C(183,206,164));oval(741,737,300,97,C(176,199,154));
    for(int i=0;i<10;i++) {float x=80+(float)i*74;line(x,706,x-5,696,2,C(144,173,127));line(x,706,x+4,696,2,C(144,173,127));}
    flower(119,672,0);flower(151,692,3);flower(684,665,2);flower(727,691,0);
    oval(196,198,46,15,WHITE);oval(172,198,22,19,WHITE);oval(206,187,28,22,WHITE);
    oval(639,184,38,13,WHITE);oval(655,178,20,18,WHITE);
    label(67,158,260,25,game.phase==LOBBY?"A POCKETFUL OF BUTTERFLIES":"FOLLOW THE FLUTTER",0,MUTED,DT_LEFT);
    elephant();
    if(game.phase==LOBBY) {
        const float xs[]={160,271,351,503,574,667},ys[]={362,278,208,237,313,399};
        for(int i=0;i<6;i++) butterfly(xs[i]+sinf(game.clock+i)*8,ys[i]+cosf(game.clock*0.8f+i)*10,15,i%4,i==3,game.clock*5+i);
        label(173,763,480,28,"Move your net. Scoop the butterflies. Smile.",1,MUTED,DT_CENTER);
    } else for(int i=0;i<BUTTERFLY_COUNT;i++) {
        Butterfly *b=&game.butterflies[i];if(b->state==AIR || b->state==GROUND) butterfly(b->x,b->y,b->golden?16:13,b->color,b->golden,b->state==AIR?game.clock*9+b->flutter:1.4f);
    }
    for(int i=0;i<game_net_count(&game);i++) net(i);
    RestoreDC(canvas,saved);
    roundbox(835,143,316,605,28,PANEL);
    label(858,159,270,24,"THE MORE, THE MERRIER",0,C(168,195,172),DT_LEFT);
    label(858,184,270,26,"Choose your adventure",2,WHITE,DT_LEFT);
    const char *modes[]={"SOLO","VS CPU","2 PLAYERS"};for(int i=0;i<3;i++) button(i,857+i*92,214,86,41,modes[i],game.mode==i);
    const char *rules[]={"RULE: MOST BUTTERFLIES","RULE: GOLDEN BUTTERFLY"};button(11,857,277,270,38,rules[game.rule],0);
    const char *winds[]={"BREEZE: GENTLE","BREEZE: BREEZY","BREEZE: GUSTY"};button(12,857,328,270,38,winds[game.difficulty],0);
    line(857,385,1127,385,1,C(72,102,85));
    label(857,397,270,22,"IN YOUR NET",0,C(168,195,172),DT_LEFT);
    int count=game_net_count(&game);char buf[96];
    for(int i=0;i<count;i++) {
        int y=426+i*49;oval(868,(float)y+15,6,6,i?C(240,155,127):C(156,217,185));
        label(887,(float)y,130,31,i?game.mode==1?"COMPUTER":"PLAYER TWO":"PLAYER ONE",2,WHITE,DT_LEFT);
        snprintf(buf,sizeof(buf),"%02d",game.nets[i].score);label(1052,(float)y-6,75,40,buf,3,WHITE,DT_RIGHT);
    }
    if(!game.mode) label(857,474,270,27,"Every butterfly is a little victory.",1,C(181,208,181),DT_LEFT);
    int caught=game_count(&game,CAUGHT);snprintf(buf,sizeof(buf),"%d / 31 COLLECTED",caught);label(857,535,270,22,buf,0,C(181,208,181),DT_LEFT);
    roundbox(857,565,270,6,6,C(63,99,81));if(caught)roundbox(857,565,270*(float)caught/31,6,6,C(239,193,102));
    snprintf(buf,sizeof(buf),"%02d TO FLY     %02d IN THE AIR",BUTTERFLY_COUNT-game.next,game_count(&game,AIR));label(857,585,270,22,buf,0,C(181,208,181),DT_LEFT);
    const char *hint=game.rule?"Catch gold to win immediately.":"Scoop from the air or the ground.";
    if(game.settle_time>0 && game.phase==PLAYING) {snprintf(buf,sizeof(buf),"Last chance! %d seconds to scoop.",(int)ceilf(8-game.settle_time));hint=buf;}
    label(857,617,270,26,hint,1,WHITE,DT_LEFT);
    button(10,857,679,270,56,game.phase==LOBBY?"LET THEM FLY":game.phase==FINISHED?"PLAY AGAIN":game.phase==PAUSED?"KEEP CATCHING":"PAUSE GAME",1);
    label(48,758,750,24,"P1: MOUSE + CLICK  or  WASD + SPACE     P2: ARROWS + CTRL",0,INK,DT_LEFT);
    label(48,787,750,20,"Hold to keep scooping. Fallen butterflies count too.",0,MUTED,DT_LEFT);
    label(838,764,320,22,"ENTER: PLAY   P: PAUSE   R: RESTART",0,MUTED,DT_CENTER);
    label(838,790,320,19,"ESC: LOBBY    M: SOUND",0,MUTED,DT_CENTER);
    if(game.phase==COUNTDOWN) {snprintf(buf,sizeof(buf),"%d",(int)ceilf(game.countdown));overlay(buf,"Nets ready. Here come the butterflies!");}
    if(game.phase==PAUSED)overlay("A LITTLE PAUSE","Press P or click to keep catching.");
    if(game.phase==FINISHED) {
        int winner=game_winner(&game);const char *title=winner<0?"A LOVELY TIE!":winner==0?"PLAYER ONE WINS!":game.mode==1?"COMPUTER WINS!":"PLAYER TWO WINS!";
        if(!game.mode) {snprintf(buf,sizeof(buf),"%d OF 31 CAUGHT",game.nets[0].score);title=game.rule && game.golden_owner==0?"YOU CAUGHT GOLD!":buf;}
        overlay(title,"Another adventure? Press Enter.");
    }
}

static void init_canvas(void) {
    canvas=CreateCompatibleDC(NULL);
    BITMAPINFO bi; memset(&bi,0,sizeof(bi)); bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth=WIDTH*SCALE; bi.bmiHeader.biHeight=-HEIGHT*SCALE; bi.bmiHeader.biPlanes=1; bi.bmiHeader.biBitCount=32; bi.bmiHeader.biCompression=BI_RGB;
    bitmap=CreateDIBSection(canvas,&bi,DIB_RGB_COLORS,&pixels,NULL,0); old_bitmap=SelectObject(canvas,bitmap);
    SetGraphicsMode(canvas,GM_ADVANCED); XFORM xf={SCALE,0,0,SCALE,0,0}; SetWorldTransform(canvas,&xf);
    const int sizes[]={12,15,16,29,32,52,42};
    for(int i=0;i<7;i++) fonts[i]=CreateFontA(-sizes[i],0,0,0,i==1?FW_NORMAL:FW_BOLD,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,"Segoe UI");
}
static void destroy_frame(void) {
    if(frame_dc && frame_old_bitmap) SelectObject(frame_dc,frame_old_bitmap);
    if(frame_bitmap) DeleteObject(frame_bitmap);
    if(frame_dc) DeleteDC(frame_dc);
    frame_old_bitmap=NULL;
    frame_dc=NULL; frame_bitmap=NULL; frame_pixels=NULL; frame_w=frame_h=0;
}
static int present_frame(HDC target,int cw,int ch) {
    if(cw<=0 || ch<=0) return 1;
    if(!frame_dc || cw!=frame_w || ch!=frame_h) {
        destroy_frame(); frame_dc=CreateCompatibleDC(target);
        memset(&frame_info,0,sizeof(frame_info)); frame_info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        frame_info.bmiHeader.biWidth=cw; frame_info.bmiHeader.biHeight=-ch;
        frame_info.bmiHeader.biPlanes=1; frame_info.bmiHeader.biBitCount=32;
        frame_bitmap=CreateDIBSection(frame_dc,&frame_info,DIB_RGB_COLORS,&frame_pixels,NULL,0);
        if(!frame_dc || !frame_bitmap) { destroy_frame(); return 0; }
        frame_old_bitmap=SelectObject(frame_dc,frame_bitmap); frame_w=cw; frame_h=ch;
    }
    // Compose the ENTIRE scaled frame, including margins, away from the window.
    // Clearing the visible window before the slow supersampling copy caused flashing.
    RECT bounds={0,0,cw,ch}; HBRUSH brush=CreateSolidBrush(BG);
    FillRect(frame_dc,&bounds,brush); DeleteObject(brush);
    float s=fminf((float)cw/WIDTH,(float)ch/HEIGHT);
    int w=(int)(WIDTH*s),h=(int)(HEIGHT*s);
    BITMAPINFO source; memset(&source,0,sizeof(source)); source.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    source.bmiHeader.biWidth=WIDTH*SCALE; source.bmiHeader.biHeight=-HEIGHT*SCALE;
    source.bmiHeader.biPlanes=1; source.bmiHeader.biBitCount=32;
    SetStretchBltMode(frame_dc,HALFTONE); SetBrushOrgEx(frame_dc,0,0,NULL);
    GdiFlush(); // Finish queued drawing before reading DIB memory.
    int copied=StretchDIBits(frame_dc,(cw-w)/2,(ch-h)/2,w,h,0,0,WIDTH*SCALE,HEIGHT*SCALE,pixels,&source,DIB_RGB_COLORS,SRCCOPY);
    if((DWORD)copied==GDI_ERROR || copied==0) return 0;
    GdiFlush();
    // The only write to the visible surface is the completed frame, at native size.
    return SetDIBitsToDevice(target,0,0,(DWORD)cw,(DWORD)ch,0,0,0,(UINT)ch,frame_pixels,&frame_info,DIB_RGB_COLORS)!=0;
}
static int test_presentation(void) {
    const int sizes[][2]={{1200,820},{960,700},{1500,900},{850,610}};
    int ok=1;
    for(int i=0;i<4;i++) {
        int cw=sizes[i][0],ch=sizes[i][1]; HDC target=CreateCompatibleDC(NULL);
        BITMAPINFO bi; memset(&bi,0,sizeof(bi)); bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth=cw; bi.bmiHeader.biHeight=-ch; bi.bmiHeader.biPlanes=1; bi.bmiHeader.biBitCount=32;
        void *data=NULL; HBITMAP bmp=CreateDIBSection(target,&bi,DIB_RGB_COLORS,&data,NULL,0);
        if(!target || !bmp) { if(bmp) DeleteObject(bmp); if(target) DeleteDC(target); return 0; }
        HGDIOBJ old=SelectObject(target,bmp);
        float s=fminf((float)cw/WIDTH,(float)ch/HEIGHT); int ox=(cw-(int)(WIDTH*s))/2,oy=(ch-(int)(HEIGHT*s))/2;
        for(int n=0;n<8;n++) {
            render(); ok&=present_frame(target,cw,ch); GdiFlush();
            ok&=GetPixel(target,ox+(int)(420*s),oy+(int)(160*s))==C(225,238,218);
            ok&=GetPixel(target,ox+(int)(850*s),oy+(int)(300*s))==PANEL;
            ok&=GetPixel(target,0,0)==BG;
        }
        SelectObject(target,old); DeleteObject(bmp); DeleteDC(target);
    }
    return ok;
}
static void destroy_canvas(void) {
    destroy_frame();
    SelectObject(canvas,GetStockObject(SYSTEM_FONT));
    for(int i=0;i<7;i++) DeleteObject(fonts[i]);
    SelectObject(canvas,old_bitmap); DeleteObject(bitmap); DeleteDC(canvas);
}
static int snapshot(const char *path) {
    render(); GdiFlush(); FILE *f=fopen(path,"wb"); if(!f) return 0;
    BITMAPFILEHEADER fh; BITMAPINFOHEADER ih; memset(&fh,0,sizeof(fh)); memset(&ih,0,sizeof(ih));
    fh.bfType=0x4d42; fh.bfOffBits=sizeof(fh)+sizeof(ih); fh.bfSize=fh.bfOffBits+WIDTH*SCALE*HEIGHT*SCALE*4;
    ih.biSize=sizeof(ih); ih.biWidth=WIDTH*SCALE; ih.biHeight=-HEIGHT*SCALE; ih.biPlanes=1; ih.biBitCount=32; ih.biSizeImage=WIDTH*SCALE*HEIGHT*SCALE*4;
    int ok=fwrite(&fh,sizeof(fh),1,f)==1 && fwrite(&ih,sizeof(ih),1,f)==1 && fwrite(pixels,ih.biSizeImage,1,f)==1;
    if(fclose(f)!=0) ok=0; return ok;
}
static void put16(unsigned char *p,unsigned int n) { p[0]=(unsigned char)n; p[1]=(unsigned char)(n>>8); }
static void put32(unsigned char *p,unsigned int n) { for(int i=0;i<4;i++) p[i]=(unsigned char)(n>>(8*i)); }
static void init_sounds(void) {
    for(int k=0;k<3;k++) {
        unsigned char *s=sounds[k]; int count=4000;
        memcpy(s,"RIFF",4); put32(s+4,36+count*2); memcpy(s+8,"WAVEfmt ",8); put32(s+16,16); put16(s+20,1); put16(s+22,1);
        put32(s+24,22050); put32(s+28,44100); put16(s+32,2); put16(s+34,16); memcpy(s+36,"data",4); put32(s+40,count*2);
        for(int i=0;i<count;i++) { float t=(float)i/22050,env=1-(float)i/(float)count; float f=k==0?600-1800*t:k==1?700:523+(float)(i/1300)*130;
            short sample=(short)(sinf(2*PI*f*t)*env*env*5500); put16(s+44+i*2,(unsigned short)sample); }
    }
}

static void clear_input(void) {
    memset(keys,0,sizeof(keys));memset(tapped,0,sizeof(tapped));mouse_down=mouse_tap=0;
}
static void action(int id) {
    int config=game.phase==LOBBY || game.phase==FINISHED;
    if(id>=0 && id<3 && config) game.mode=id;
    if(id==11 && config) game.rule=!game.rule;
    if(id==12 && config) game.difficulty=(game.difficulty+1)%3;
    if(id==13) {muted=!muted;if(muted)PlaySoundA(NULL,NULL,0);}
    if(id==10) {if(config)game_start(&game);else game_pause(&game);clear_input();accumulator=0;}
}
static void mouse_point(LPARAM lp,int *x,int *y) {
    float s=fminf((float)client_w/WIDTH,(float)client_h/HEIGHT);
    if(s<=0) {*x=*y=-1;return;}
    *x=(int)(((float)GET_X_LPARAM(lp)-((float)client_w-WIDTH*s)*0.5f)/s);
    *y=(int)(((float)GET_Y_LPARAM(lp)-((float)client_h-HEIGHT*s)*0.5f)/s);
}
static int on_field(int x,int y) {return x>=40 && x<797 && y>=143 && y<744;}
static void step_input(float dt) {
    Input in={0};in.dx[0]=(float)(keys['D']-keys['A']);in.dy[0]=(float)(keys['S']-keys['W']);
    in.dx[1]=(float)(keys[VK_RIGHT]-keys[VK_LEFT]);in.dy[1]=(float)(keys[VK_DOWN]-keys[VK_UP]);
    in.scoop[0]=keys[VK_SPACE]||tapped[0]||mouse_down||mouse_tap;
    in.scoop[1]=keys[VK_CONTROL]||tapped[1];in.mouse=mouse_mode;
    in.mouse_x=(float)mouse_x;in.mouse_y=(float)mouse_y;
    game_step(&game,dt,&in);memset(tapped,0,sizeof(tapped));mouse_tap=0;
}
static int native_checks(HWND hwnd) {
    int ok=1; FILE *log=fopen("smoke-details.txt","w");
#define CHECK(expr) do { if(!(expr)) {ok=0; if(log)fprintf(log,"Failed line %d: %s\n",__LINE__,#expr); } } while(0)
    CHECK(game.phase==COUNTDOWN);
    SendMessage(hwnd,WM_KEYDOWN,'P',0);CHECK(game.phase==PAUSED);
    SendMessage(hwnd,WM_KEYDOWN,VK_RETURN,0);CHECK(game.phase==COUNTDOWN);
    SendMessage(hwnd,WM_KILLFOCUS,0,0);CHECK(game.phase==PAUSED);
    SendMessage(hwnd,WM_KEYDOWN,VK_ESCAPE,0);CHECK(game.phase==LOBBY);
    SendMessage(hwnd,WM_KEYDOWN,'3',0);CHECK(game.mode==2);
    SendMessage(hwnd,WM_KEYDOWN,'G',0);CHECK(game.rule==1);
    SendMessage(hwnd,WM_KEYDOWN,'B',0);CHECK(game.difficulty==1);
    SendMessage(hwnd,WM_KEYDOWN,'M',0);CHECK(muted==1);
    SendMessage(hwnd,WM_SIZE,0,MAKELPARAM(WIDTH,HEIGHT));
    SendMessage(hwnd,WM_LBUTTONDOWN,0,MAKELPARAM(990,700));CHECK(game.phase==COUNTDOWN);
    game.phase=PLAYING;
    SendMessage(hwnd,WM_KEYDOWN,VK_SPACE,0);SendMessage(hwnd,WM_KEYUP,VK_SPACE,0);
    SendMessage(hwnd,WM_KEYDOWN,VK_CONTROL,0);SendMessage(hwnd,WM_KEYUP,VK_CONTROL,0);
    step_input(1.0f/120);CHECK(game.nets[0].swing>0 && game.nets[1].swing>0);
    float before=game.nets[0].x;
    SendMessage(hwnd,WM_KEYDOWN,'D',0);step_input(1.0f/120);SendMessage(hwnd,WM_KEYUP,'D',0);CHECK(game.nets[0].x>before);
    before=game.nets[1].x;SendMessage(hwnd,WM_KEYDOWN,VK_LEFT,0);step_input(1.0f/120);SendMessage(hwnd,WM_KEYUP,VK_LEFT,0);CHECK(game.nets[1].x<before);
    SendMessage(hwnd,WM_MOUSEMOVE,0,MAKELPARAM(600,500));CHECK(mouse_mode && mouse_x==600);
    game.nets[0].cooldown=game.nets[0].swing=0;
    SendMessage(hwnd,WM_LBUTTONDOWN,0,MAKELPARAM(600,500));SendMessage(hwnd,WM_LBUTTONUP,0,MAKELPARAM(600,500));
    step_input(1.0f/120);CHECK(game.nets[0].swing>0);
    SendMessage(hwnd,WM_KILLFOCUS,0,0);CHECK(!mouse_down && !keys[VK_SPACE] && game.phase==PAUSED);
    SendMessage(hwnd,WM_KEYDOWN,'R',0);CHECK(game.phase==COUNTDOWN && game.next==0);
    CHECK(test_presentation());
    DWORD handles=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(int i=0;i<60;i++)render();CHECK(handles==GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
    FILE *f=fopen("smoke-result.txt","w");if(f){fprintf(f,"%s: native window, countdown, pause, resume, focus loss, restart, mode/rule/breeze selection, mouse movement, mouse scoop, short keyboard scoops, both players movement, sound toggle, four presentation sizes, stable GDI resources.\n",ok?"PASS":"FAIL");fclose(f);}
    if(log)fclose(log);
#undef CHECK
    return ok;
}
static LRESULT CALLBACK wndproc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) {
    case WM_ERASEBKGND:return 1;
    case WM_SIZE:client_w=LOWORD(lp);client_h=HIWORD(lp);return 0;
    case WM_GETMINMAXINFO:((MINMAXINFO*)lp)->ptMinTrackSize.x=850;((MINMAXINFO*)lp)->ptMinTrackSize.y=610;return 0;
    case WM_KEYDOWN:
        if(wp<256)keys[wp]=1;
        if(wp=='W'||wp=='A'||wp=='S'||wp=='D')mouse_mode=0;
        if(lp&(1L<<30))return 0;
        if(game.phase==PLAYING) {if(wp==VK_SPACE)tapped[0]=1;if(wp==VK_CONTROL)tapped[1]=1;}
        if(wp==VK_RETURN && (game.phase==LOBBY||game.phase==FINISHED||game.phase==PAUSED))action(10);
        if(wp=='P'){game_pause(&game);clear_input();}
        if(wp=='R' && game.phase!=LOBBY){game_start(&game);clear_input();}
        if(wp=='M')action(13);
        if(wp=='G')action(11);
        if(wp=='B')action(12);
        if(wp>='1'&&wp<='3')action((int)(wp-'1'));
        if(wp==VK_ESCAPE){int mode=game.mode,rule=game.rule,diff=game.difficulty;uint32_t seed=game.rng;game_init(&game,seed);game.mode=mode;game.rule=rule;game.difficulty=diff;clear_input();accumulator=0;}
        return 0;
    case WM_KEYUP:if(wp<256)keys[wp]=0;return 0;
    case WM_KILLFOCUS:clear_input();if(game.phase==PLAYING||game.phase==COUNTDOWN)game_pause(&game);return 0;
    case WM_MOUSEMOVE:{int x,y;mouse_point(lp,&x,&y);hover=hit(x,y);if(on_field(x,y)){mouse_x=x;mouse_y=y;mouse_mode=1;}SetCursor(LoadCursor(NULL,hover>=0?IDC_HAND:IDC_ARROW));return 0;}
    case WM_LBUTTONDOWN:{SetFocus(hwnd);int x,y;mouse_point(lp,&x,&y);int id=hit(x,y);if(id>=0)action(id);else if(on_field(x,y)){mouse_x=x;mouse_y=y;mouse_mode=1;mouse_down=1;mouse_tap=1;SetCapture(hwnd);}return 0;}
    case WM_LBUTTONUP:mouse_down=0;if(GetCapture()==hwnd)ReleaseCapture();return 0;
    case WM_CAPTURECHANGED:mouse_down=0;return 0;
    case WM_TIMER:{
        LARGE_INTEGER now;QueryPerformanceCounter(&now);double dt=(double)(now.QuadPart-last_tick.QuadPart)/(double)frequency.QuadPart;last_tick=now;
        if(dt>0.1)dt=0.1;accumulator+=dt;while(accumulator>=1.0/120.0){step_input(1.0f/120);accumulator-=1.0/120.0;}
        if(game.sound_events&&!muted){int k=(game.sound_events&4)?2:(game.sound_events&2)?1:0;PlaySoundA((LPCSTR)sounds[k],NULL,SND_MEMORY|SND_ASYNC|SND_NODEFAULT);}game.sound_events=0;
        InvalidateRect(hwnd,NULL,FALSE);
        if(smoke&&++smoke_ticks==12){int ok=native_checks(hwnd);DestroyWindow(hwnd);if(!ok)PostQuitMessage(1);}return 0;
    }
    case WM_PAINT:{PAINTSTRUCT ps;HDC target=BeginPaint(hwnd,&ps);render();present_frame(target,client_w,client_h);EndPaint(hwnd,&ps);return 0;}
    case WM_CLOSE:DestroyWindow(hwnd);return 0;
    case WM_DESTROY:KillTimer(hwnd,1);PostQuitMessage(0);return 0;
    default:return DefWindowProc(hwnd,msg,wp,lp);
    }
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE previous,LPSTR cmd,int show) {
    (void)previous;(void)cmd;SetProcessDPIAware();game_init(&game,(uint32_t)GetTickCount());init_canvas();init_sounds();
    for(int i=1;i<__argc;i++) {
        if(!strcmp(__argv[i],"--snapshot")&&i+1<__argc){const char *path=__argv[++i];
            if(i+1<__argc){game_init(&game,375);game.mode=1;game_start(&game);game.phase=PLAYING;
                Input in={0};for(int t=0;t<1200;t++){in.mouse=1;in.scoop[0]=1;in.mouse_x=270;in.mouse_y=445;game_step(&game,1.0f/120,&in);}
                if(!strcmp(__argv[i+1],"paused"))game_pause(&game);
                if(!strcmp(__argv[i+1],"results"))game.phase=FINISHED;
            }
            int ok=snapshot(path);destroy_canvas();return ok?0:1;
        }
        if(!strcmp(__argv[i],"--smoke-test"))smoke=1;
    }
    WNDCLASSA wc;memset(&wc,0,sizeof(wc));wc.lpfnWndProc=wndproc;wc.hInstance=instance;wc.lpszClassName="ElefunButterflyClub";
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    if(!RegisterClassA(&wc)){destroy_canvas();return 1;}
    RECT rect={0,0,WIDTH,HEIGHT};AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);int ww=rect.right-rect.left,wh=rect.bottom-rect.top;
    if(wh>GetSystemMetrics(SM_CYSCREEN)-80){wh=GetSystemMetrics(SM_CYSCREEN)-80;ww=(int)((float)wh*WIDTH/HEIGHT);}
    window=CreateWindowA(wc.lpszClassName,"Elefun | The Butterfly Club",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,ww,wh,NULL,NULL,instance,NULL);
    if(!window){destroy_canvas();return 1;}
    QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&last_tick);SetTimer(window,1,16,NULL);ShowWindow(window,smoke?SW_HIDE:show);UpdateWindow(window);
    if(smoke)SendMessage(window,WM_KEYDOWN,VK_RETURN,0);
    MSG msg;int result;while((result=GetMessage(&msg,NULL,0,0))>0){TranslateMessage(&msg);DispatchMessage(&msg);}
    PlaySoundA(NULL,NULL,0);destroy_canvas();return result<0?1:(int)msg.wParam;
}
