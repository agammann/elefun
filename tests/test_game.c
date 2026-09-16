#include "../src/game.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static void valid(const Game *g) {
    int states[4]={0},owners[2]={0},gold=0;
    for(int i=0;i<BUTTERFLY_COUNT;i++) {
        const Butterfly *b=&g->butterflies[i]; assert(b->state>=WAITING&&b->state<=CAUGHT);states[b->state]++;gold+=b->golden;
        assert(isfinite(b->x)&&isfinite(b->y)&&isfinite(b->vx)&&isfinite(b->vy));
        if(b->state==CAUGHT){assert(b->owner>=0&&b->owner<game_net_count(g));owners[b->owner]++;}else assert(b->owner==-1);
        if(b->state==AIR||b->state==GROUND){assert(b->x>=95&&b->x<=745);assert(b->y>=150&&b->y<=FLOOR_Y);}
    }
    assert(gold==1);assert(states[WAITING]==BUTTERFLY_COUNT-g->next);
    assert(owners[0]==g->nets[0].score&&owners[1]==g->nets[1].score);
    assert(states[WAITING]+states[AIR]+states[GROUND]+states[CAUGHT]==BUTTERFLY_COUNT);
    for(int i=0;i<game_net_count(g);i++){assert(g->nets[i].x>=92&&g->nets[i].x<=748);assert(g->nets[i].y>=180&&g->nets[i].y<=FLOOR_Y);}
}
static Input chasing(const Game *g) {
    Input in={0};in.scoop[0]=in.scoop[1]=1;
    for(int p=0;p<2;p++) {
        float best=1e9f;int index=-1;
        for(int j=0;j<BUTTERFLY_COUNT;j++){const Butterfly *b=&g->butterflies[j];if(b->state!=AIR&&b->state!=GROUND)continue;
            float d=hypotf(b->x-g->nets[p].x,b->y-g->nets[p].y);if(d<best){best=d;index=j;}}
        if(index>=0){const Butterfly *b=&g->butterflies[index];if(p==0){in.mouse=1;in.mouse_x=b->x;in.mouse_y=b->y;}
            else{float d=fmaxf(1,best);in.dx[p]=(b->x-g->nets[p].x)/d;in.dy[p]=(b->y-g->nets[p].y)/d;}}
    }
    return in;
}
int main(void) {
    Game g;Input none={0};game_init(&g,123);valid(&g);assert(g.phase==LOBBY);
    game_start(&g);for(int i=0;i<361;i++)game_step(&g,1.0f/120,&none);assert(g.phase==PLAYING);
    game_pause(&g);Game copy=g;game_step(&g,0.02f,&none);assert(!memcmp(&copy,&g,sizeof(g)));game_pause(&g);assert(g.phase==PLAYING);
    copy=g;game_step(&g,NAN,&none);game_step(&g,1,&none);assert(!memcmp(&copy,&g,sizeof(g)));
    // Floor pickups have the same one point value as airborne catches.
    game_start(&g);g.phase=PLAYING;g.next=1;g.launch_in=10;g.nets[0].y=FLOOR_Y;
    g.butterflies[0].state=GROUND;g.butterflies[0].x=g.nets[0].x;g.butterflies[0].y=FLOOR_Y;
    Input scoop={0};scoop.scoop[0]=1;game_step(&g,1.0f/120,&scoop);assert(g.nets[0].score==1&&g.butterflies[0].owner==0);valid(&g);
    for(int i=0;i<30;i++)game_step(&g,1.0f/120,&scoop);assert(g.nets[0].score==1);
    // A golden catch ends the golden rule immediately, regardless of other scores.
    game_start(&g);g.phase=PLAYING;g.rule=1;g.next=1;g.launch_in=10;
    g.butterflies[g.golden_index].golden=0;g.golden_index=0;g.butterflies[0].golden=1;
    g.butterflies[0].state=GROUND;g.butterflies[0].x=g.nets[0].x;g.butterflies[0].y=FLOOR_Y;g.nets[0].y=FLOOR_Y;
    game_step(&g,1.0f/120,&scoop);assert(g.phase==FINISHED&&game_winner(&g)==0&&g.golden_owner==0);valid(&g);
    copy=g;game_step(&g,0.02f,&scoop);assert(!memcmp(&copy,&g,sizeof(g)));
    // Equidistant simultaneous nets share priority across butterfly IDs.
    game_init(&g,123);g.mode=2;game_start(&g);g.phase=PLAYING;g.next=2;g.launch_in=10;
    g.nets[1]=g.nets[0];g.nets[0].y=g.nets[1].y=FLOOR_Y;
    for(int i=0;i<2;i++){g.butterflies[i].state=GROUND;g.butterflies[i].x=255;g.butterflies[i].y=FLOOR_Y;}
    scoop.scoop[1]=1;game_step(&g,1.0f/120,&scoop);assert(g.nets[0].score==1&&g.nets[1].score==1&&game_winner(&g)==-1);valid(&g);
    // A completely idle game still releases every butterfly and finishes.
    game_init(&g,12);game_start(&g);for(int t=0;t<8500&&g.phase!=FINISHED;t++)game_step(&g,1.0f/120,&none);
    assert(g.phase==FINISHED&&game_count(&g,GROUND)==31);valid(&g);
    int rounds=0,total_catches=0;
    for(int mode=0;mode<3;mode++)for(int rule=0;rule<2;rule++)for(int difficulty=0;difficulty<3;difficulty++)for(int seed=1;seed<=20;seed++) {
        game_init(&g,(uint32_t)seed);g.mode=mode;g.rule=rule;g.difficulty=difficulty;game_start(&g);
        for(int t=0;t<8500&&g.phase!=FINISHED;t++){Input in=chasing(&g);game_step(&g,1.0f/120,&in);valid(&g);}
        assert(g.phase==FINISHED);assert(g.nets[0].score+g.nets[1].score>0);total_catches+=g.nets[0].score+g.nets[1].score;rounds++;
        game_start(&g);assert(g.phase==COUNTDOWN&&g.next==0&&g.nets[0].score==0&&g.nets[1].score==0);valid(&g);
    }
    printf("PASS: %d full rounds across 3 modes, 2 rules, 3 breeze levels and 20 seeds; %d catches.\n",rounds,total_catches);
    puts("PASS: score conservation, butterfly ownership, ground pickup, golden victory, tie fairness, bounds, pause, countdown, restart, idle finish, invalid time step and frozen results.");
    return 0;
}
