#pragma once
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <clm.h>
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_SHOTS 100
#define NUM_MEN 100
#define MAX_SOUND_CHANNELS 8

typedef struct Shot {
  vec2 pos;
  vec2 vel;
} Shot;

typedef struct Man {
  vec2 pos;
  float speed;
} Man;

// Asset indices
typedef enum TextureIndex {
  TEX_IDX_SLINGSHOT,
  TEX_IDX_BALL,
  TEX_IDX_MAN,
  TEX_IDX_SKY,
  TEX_IDX_COUNT
} TextureIndex;

typedef enum SoundIndex {
  SFX_IDX_OOF,
  SFX_IDX_BOING,
  SFX_IDX_COUNT
} SoundIndex;

// Game state
typedef struct GameState {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *textures[TEX_IDX_COUNT];
  SDL_Rect rects[TEX_IDX_COUNT];
  Mix_Chunk *sfx[SFX_IDX_COUNT];
  Mix_Music *music;
  TTF_Font *font;
  
  vec2 ballStart;
  vec2 ballPos;
  vec2 slingshotPos;
  vec2 dim;
  float speed;
  float spawnInterval;
  float beginSpawnInterval;
  int score;
  int hiscore;
  bool slingshotActive;
  int mouseX, mouseY;
  int mouseButtonDown;
  
  Shot shots[NUM_SHOTS];
  int numShots;
  
  Man men[NUM_MEN]; 
  int numMen;
  
  SDL_TimerID spawnTimerId;
  SDL_TimerID rateTimerId;

  Uint64 lastTime;
  bool running;
  SDL_Event e;
} GameState;

// Function prototypes
static bool initSDL(void);
static bool loadAssets(void);
static void cleanupSDL(void);
static void handleEvents(SDL_Event *e, bool *quit);
static void renderScore(void);
static void renderSlingshot(void);
static void spawnMan();
static void update(float delta);
static void draw();
static void updateShots(float delta);
static void updateMen(float delta);
static void decreaseSpawnInterval();
// SDL_Timer Callback Wrappers
static Uint32 spawnManCallback(Uint32 interval, void *param);
static void decreaseSpawnInterval();
static Uint32 decreaseSpawnIntervalCallback(Uint32 interval, void *param);
static void scheduleTasks();
static void resetSpawnInterval();
int main(int argc, char *argv[]);
void main_loop();