#ifndef ELEFUN_GAME_H
#define ELEFUN_GAME_H
#include <stdint.h>
#define BUTTERFLY_COUNT 31
#define PI 3.14159265358979323846f
#define FLOOR_Y 684.0f
typedef enum { LOBBY, COUNTDOWN, PLAYING, PAUSED, FINISHED } Phase;
typedef enum { WAITING, AIR, GROUND, CAUGHT } ButterflyState;
typedef struct { float x,y,vx,vy,age,flutter; ButterflyState state; int owner,color,golden; } Butterfly;
typedef struct { float x,y,swing,cooldown,flash; int score; } Net;
typedef struct { float dx[2],dy[2],mouse_x,mouse_y; int scoop[2],mouse; } Input;
typedef struct {
    Phase phase,before_pause;
    Butterfly butterflies[BUTTERFLY_COUNT];
    Net nets[2];
    uint32_t rng;
    int mode,rule,difficulty,next,golden_index,golden_owner,sound_events;
    float clock,elapsed,countdown,launch_in,settle_time,puff;
} Game;
void game_init(Game *g,uint32_t seed);
void game_start(Game *g);
void game_pause(Game *g);
void game_step(Game *g,float dt,const Input *input);
int game_net_count(const Game *g);
int game_count(const Game *g,ButterflyState state);
int game_winner(const Game *g);
#endif
