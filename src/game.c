#include "game.h"
#include <math.h>
#include <string.h>
static float rnd(Game *g) { uint32_t x=g->rng; x^=x<<13; x^=x>>17; x^=x<<5; g->rng=x; return (float)(x&0xffffff)/16777216.0f; }
static float clamp(float x,float a,float b) { return fmaxf(a,fminf(x,b)); }
int game_net_count(const Game *g) { return g->mode==0?1:2; }
int game_count(const Game *g,ButterflyState state) { int n=0; for(int i=0;i<BUTTERFLY_COUNT;i++) n+=g->butterflies[i].state==state; return n; }
int game_winner(const Game *g) {
    if(g->rule && g->golden_owner>=0) return g->golden_owner;
    if(!g->mode) return 0;
    return g->nets[0].score==g->nets[1].score?-1:g->nets[0].score>g->nets[1].score?0:1;
}
static void reset_round(Game *g) {
    memset(g->butterflies,0,sizeof(g->butterflies)); memset(g->nets,0,sizeof(g->nets));
    g->nets[0].x=255; g->nets[0].y=490; g->nets[1].x=620; g->nets[1].y=460;
    g->next=0; g->elapsed=0; g->launch_in=0.25f; g->settle_time=0; g->puff=0;
    g->sound_events=0; g->golden_owner=-1; g->golden_index=20+(int)(rnd(g)*11);
    for(int i=0;i<BUTTERFLY_COUNT;i++) { g->butterflies[i].owner=-1; g->butterflies[i].color=i%4; g->butterflies[i].golden=i==g->golden_index; }
}
void game_init(Game *g,uint32_t seed) { memset(g,0,sizeof(*g)); g->rng=seed?seed:67891; g->phase=LOBBY; g->difficulty=0; reset_round(g); }
void game_start(Game *g) { reset_round(g); g->phase=COUNTDOWN; g->countdown=3; }
void game_pause(Game *g) {
    if(g->phase==PAUSED) g->phase=g->before_pause;
    else if(g->phase==PLAYING || g->phase==COUNTDOWN) { g->before_pause=g->phase; g->phase=PAUSED; }
}
static void move_toward(Net *n,float x,float y,float speed,float dt) {
    float dx=x-n->x,dy=y-n->y,d=hypotf(dx,dy);
    if(d>0.1f) { float step=fminf(d,speed*dt); n->x+=dx/d*step; n->y+=dy/d*step; }
}
void game_step(Game *g,float dt,const Input *in) {
    if(!isfinite(dt) || dt<=0 || dt>0.05f || g->phase==PAUSED || g->phase==FINISHED) return;
    g->clock+=dt; g->puff=fmaxf(0,g->puff-dt);
    if(g->phase==LOBBY) return;
    if(g->phase==COUNTDOWN) { g->countdown-=dt; if(g->countdown<=0) {g->phase=PLAYING;g->countdown=0;g->sound_events|=2;} return; }
    g->elapsed+=dt; g->launch_in-=dt;
    if(g->next<BUTTERFLY_COUNT && g->launch_in<=0) {
        Butterfly *b=&g->butterflies[g->next++]; b->state=AIR; b->x=420; b->y=367;
        b->vx=(rnd(g)-0.5f)*(290+g->difficulty*45); b->vy=-225-rnd(g)*60;
        b->flutter=rnd(g)*2*PI; b->age=0;
        g->launch_in=0.62f+rnd(g)*0.35f; g->puff=0.35f;
    }
    for(int j=0;j<BUTTERFLY_COUNT;j++) {
        Butterfly *b=&g->butterflies[j];
        if(b->state!=AIR) continue;
        b->age+=dt; b->vx+=sinf(g->clock*1.7f+b->flutter)*(16+g->difficulty*12)*dt;
        b->vy=fminf(72+g->difficulty*18,b->vy+106*dt);
        b->x+=b->vx*dt; b->y+=b->vy*dt;
        if(b->x<95) {b->x=95;b->vx=fabsf(b->vx)*0.75f;} if(b->x>745) {b->x=745;b->vx=-fabsf(b->vx)*0.75f;}
        if(b->y<150) {b->y=150;b->vy=fmaxf(0,b->vy);}
        if(b->y>=FLOOR_Y) {b->y=FLOOR_Y;b->state=GROUND;b->vx=b->vy=0;}
    }
    for(int i=0;i<game_net_count(g);i++) {
        Net *n=&g->nets[i]; n->swing=fmaxf(0,n->swing-dt); n->cooldown=fmaxf(0,n->cooldown-dt); n->flash=fmaxf(0,n->flash-dt);
        int scoop=in->scoop[i];
        if(i==1 && g->mode==1) {
            int best=-1; float cost=1e9f;
            for(int j=0;j<BUTTERFLY_COUNT;j++) {
                Butterfly *b=&g->butterflies[j]; if((b->state!=AIR && b->state!=GROUND) || (b->state==AIR && b->vy<0)) continue;
                float c=hypotf(b->x-n->x,b->y-n->y)+(b->state==GROUND?75:0);
                if(c<cost) {cost=c;best=j;}
            }
            scoop=0;
            if(best>=0) { Butterfly *b=&g->butterflies[best]; move_toward(n,b->x+b->vx*0.12f,b->y,240+g->difficulty*35,dt); scoop=hypotf(b->x-n->x,b->y-n->y)<42; }
        } else if(i==0 && in->mouse) move_toward(n,in->mouse_x,in->mouse_y,680,dt);
        else {
            float dx=in->dx[i],dy=in->dy[i],d=hypotf(dx,dy); if(d>1) {dx/=d;dy/=d;}
            n->x+=dx*410*dt; n->y+=dy*410*dt;
        }
        n->x=clamp(n->x,92,748); n->y=clamp(n->y,180,FLOOR_Y);
        if(scoop && n->cooldown<=0) {n->swing=0.28f;n->cooldown=0.43f;}
    }
    // Each butterfly chooses the closest active net, avoiding fixed player priority.
    for(int j=0;j<BUTTERFLY_COUNT;j++) {
        Butterfly *b=&g->butterflies[j]; if((b->state!=AIR && b->state!=GROUND) || (b->state==AIR && (b->age<0.55f || b->vy<0))) continue;
        int winner=-1; float closest=1.0f;
        for(int i=0;i<game_net_count(g);i++) {
            Net *n=&g->nets[i]; if(n->swing<=0) continue;
            float dx=(b->x-n->x)/49,dy=(b->y-n->y)/34,dist=dx*dx+dy*dy;
            if(dist<closest) {closest=dist;winner=i;}
            else if(winner>=0 && fabsf(dist-closest)<0.00001f && (j%2)==i) winner=i;
        }
        if(winner>=0) { b->state=CAUGHT;b->owner=winner;g->nets[winner].score++;g->nets[winner].flash=0.45f;g->sound_events|=1;
            if(b->golden) g->golden_owner=winner;
        }
    }
    if(g->next==BUTTERFLY_COUNT && !game_count(g,AIR)) g->settle_time+=dt; else g->settle_time=0;
    if(game_count(g,CAUGHT)==BUTTERFLY_COUNT || g->settle_time>=8 || g->elapsed>=65 || (g->rule && g->golden_owner>=0)) {g->phase=FINISHED;g->sound_events|=4;}
}
