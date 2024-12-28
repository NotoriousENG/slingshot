#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "main.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

GameState game;

int main(int argc, char *argv[]) {
  // Initialize game state
  game.virtualWidth = VIRTUAL_WIDTH;
  game.virtualHeight = VIRTUAL_HEIGHT;
  game.ballStart = (vec2){350.0f, 536.0f};
  game.ballPos = game.ballStart;
  game.slingshotPos = (vec2){300, 472};
  game.dim = (vec2){16.0f, 16.0f};
  game.speed = 300.0f;
  game.beginSpawnInterval = 2.0f;
  game.spawnInterval = game.beginSpawnInterval;

  if (!initSDL() || !loadAssets()) {
    cleanupSDL();
    return -1;
  }

  // Create virtual screen texture that matches window size
  int w, h;
#ifdef EMSCRIPTEN
  // Get display size on web
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  SDL_SetWindowSize(game.window, w, h);
#else
  SDL_GetWindowSize(game.window, &w, &h);
#endif
  game.virtualWidth = w;
  game.virtualHeight = h;

  game.virtualScreen = SDL_CreateTexture(
      game.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      game.virtualWidth, game.virtualHeight);
  SDL_SetTextureBlendMode(game.virtualScreen, SDL_BLENDMODE_BLEND);

  // No scaling or letterboxing needed since virtual matches window
  game.scale = 1.0f;
  game.offsetX = 0;
  game.offsetY = 0;

  // Viewport covers entire window
  game.viewport.x = 0;
  game.viewport.y = 0;
  game.viewport.w = game.virtualWidth;
  game.viewport.h = game.virtualHeight;

  // Adjust game object positions based on screen dimensions
  float heightRatio = (float)game.virtualHeight / VIRTUAL_HEIGHT;
  float widthRatio = (float)game.virtualWidth / VIRTUAL_WIDTH;

  game.ballStart.y *= heightRatio;
  game.ballStart.x *= widthRatio;
  game.ballPos = game.ballStart;

  game.slingshotPos.x *= widthRatio;
  game.slingshotPos.y *= heightRatio;

  // Scale slingshot sprite dimensions
  game.rects[TEX_IDX_SLINGSHOT].w *= widthRatio;
  game.rects[TEX_IDX_SLINGSHOT].h *= heightRatio;

  Mix_PlayMusic(game.music, -1);

  game.running = true;
  game.lastTime = SDL_GetTicks64();

  scheduleTasks();

#ifdef EMSCRIPTEN
  emscripten_set_main_loop(main_loop, 0, game.running);
#else
  while (game.running) {
    main_loop();
  }
#endif

  cleanupSDL();
  return 0;
}

void main_loop() {
  Uint32 currentTime = SDL_GetTicks64();
  float delta = (currentTime - game.lastTime) / 1000.0f;
  game.lastTime = currentTime;

  handleEvents(&game.e, &game.running);
  update(delta);
  draw();
}

static bool initSDL(void) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
    return false;
  }

  Mix_AllocateChannels(MAX_SOUND_CHANNELS);

  if (TTF_Init() < 0) {
    printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
    return false;
  }

  game.window = SDL_CreateWindow("Slingshot", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                 SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (!game.window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

  game.renderer =
      SDL_CreateRenderer(game.window, -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
                             SDL_RENDERER_TARGETTEXTURE);
  if (!game.renderer) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (!(IMG_Init(IMG_INIT_PNG))) {
    printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
    return false;
  }

  SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
  SDL_SetRenderDrawBlendMode(game.renderer, SDL_BLENDMODE_BLEND);

  return true;
}

static bool loadAssets(void) {
  // Load textures
  printf("loading textures \n");
  game.textures[TEX_IDX_SLINGSHOT] =
      IMG_LoadTexture(game.renderer, "assets/textures/slingshot.png");
  game.textures[TEX_IDX_BALL] =
      IMG_LoadTexture(game.renderer, "assets/textures/ball.png");
  game.textures[TEX_IDX_MAN] =
      IMG_LoadTexture(game.renderer, "assets/textures/man.png");
  game.textures[TEX_IDX_SKY] =
      IMG_LoadTexture(game.renderer, "assets/textures/sky.png");
  printf("loaded textures SUCCESS \n");

  // Initialize rects
  for (int i = 0; i < TEX_IDX_COUNT; i++) {
    memset(&game.rects[i], 0, sizeof(SDL_Rect));
    SDL_QueryTexture(game.textures[i], NULL, NULL, &game.rects[i].w,
                     &game.rects[i].h);
  }

  printf("Loading audio\n");
  // Load audio
  game.font = TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24);
  game.sfx[SFX_IDX_OOF] = Mix_LoadWAV("assets/sfx/oof.ogg");
  game.sfx[SFX_IDX_BOING] = Mix_LoadWAV("assets/sfx/boing-jump.ogg");
  Mix_VolumeChunk(game.sfx[SFX_IDX_BOING], (int)(0.6f * 128.0f));
  game.music = Mix_LoadMUS("assets/music/LunarJoyride.ogg");
  if (!game.music) {
    printf("%s\n", Mix_GetError());
    return false;
  }
  printf("audio loaded\n");

  // Verify all assets loaded
  for (int i = 0; i < TEX_IDX_COUNT; i++) {
    if (!game.textures[i]) {
      printf("problem in texture %i\n", i);
      return false;
    }
  }
  for (int i = 0; i < SFX_IDX_COUNT; i++) {
    if (!game.sfx[i]) {
      printf("problem in sfx %i\n", i);
      return false;
    }
  }
  if (!game.font) {
    printf("problem in font\n");
    return false;
  }

  printf("loaded everything!\n");

  return true;
}

static void cleanupSDL(void) {
  for (int i = 0; i < TEX_IDX_COUNT; i++) {
    SDL_DestroyTexture(game.textures[i]);
  }
  for (int i = 0; i < SFX_IDX_COUNT; i++) {
    Mix_FreeChunk(game.sfx[i]);
  }
  Mix_FreeMusic(game.music);
  TTF_CloseFont(game.font);
  SDL_DestroyTexture(game.virtualScreen);
  SDL_DestroyRenderer(game.renderer);
  SDL_DestroyWindow(game.window);

  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

static void handleEvents(SDL_Event *e, bool *running) {
  while (SDL_PollEvent(e) != 0) {
    if (e->type == SDL_WINDOWEVENT) {
      if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
        int w = e->window.data1;
        int h = e->window.data2;

        // Recreate virtual screen at new window size
        SDL_DestroyTexture(game.virtualScreen);
        game.virtualWidth = w;
        game.virtualHeight = h;
        game.virtualScreen = SDL_CreateTexture(
            game.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            game.virtualWidth, game.virtualHeight);
        SDL_SetTextureBlendMode(game.virtualScreen, SDL_BLENDMODE_BLEND);

        // Update viewport to match new size
        game.viewport.w = w;
        game.viewport.h = h;

        // Adjust game object positions for new dimensions
        float heightRatio = (float)h / VIRTUAL_HEIGHT;
        float widthRatio = (float)w / VIRTUAL_WIDTH;

        game.ballStart.y = 536.0f * heightRatio;
        game.ballStart.x = 350.0f * widthRatio;
        game.ballPos = game.ballStart;

        game.slingshotPos.x = 300 * widthRatio;
        game.slingshotPos.y = 472 * heightRatio;

        // Scale slingshot sprite dimensions
        SDL_QueryTexture(game.textures[TEX_IDX_SLINGSHOT], NULL, NULL,
                         &game.rects[TEX_IDX_SLINGSHOT].w,
                         &game.rects[TEX_IDX_SLINGSHOT].h);
        game.rects[TEX_IDX_SLINGSHOT].w *= widthRatio;
        game.rects[TEX_IDX_SLINGSHOT].h *= heightRatio;
      }
    }

    // Mouse coordinates are already in virtual space since they match window
    // space
    SDL_GetMouseState(&game.mouseX, &game.mouseY);

    if (e->type == SDL_QUIT) {
      *running = false;
    }
    if (e->type == SDL_MOUSEBUTTONDOWN) {
      game.mouseButtonDown = 1;
    } else if (e->type == SDL_MOUSEBUTTONUP) {
      game.mouseButtonDown = 0;
    }
  }
}

static void update(float delta) {
  SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));

  // Scale slingshot origin based on screen size
  float heightRatio = (float)game.virtualHeight / VIRTUAL_HEIGHT;
  float widthRatio = (float)game.virtualWidth / VIRTUAL_WIDTH;
  vec2 slingshotOrigin = vec2_new(348.0f * widthRatio, 468.0f * heightRatio);
  vec2 mouse = vec2_new((float)game.mouseX, (float)game.mouseY);

  if (!game.slingshotActive) {
    float distance = vec2_distance(vec2_add(game.ballStart, game.dim), mouse);

    if (distance <= 32.0f) {
      SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND));

      if (game.mouseButtonDown) {
        game.slingshotActive = true;
      }
    }
  } else {
    SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND));

    game.ballPos.x = mouse.x - game.dim.x;
    game.ballPos.y = mouse.y - game.dim.y;

    game.ballPos.x = clampf(game.ballPos.x, -16.0f, game.virtualWidth + 16.0f);
    game.ballPos.y = clampf(game.ballPos.y, -16.0f, game.virtualHeight + 16.0f);

    if (!game.mouseButtonDown) {
      vec2 dir = vec2_normalize(vec2_subtract(slingshotOrigin, game.ballPos));

      float fac = fabsf(vec2_distance(slingshotOrigin, game.ballPos) * 10.0f);
      float facVolume =
          fabsf(vec2_distance(game.ballStart, game.ballPos)) * 10.0f;

      Shot newShot = {game.ballPos, vec2_scale(dir, fac)};
      game.shots[game.numShots++ % NUM_SHOTS] = newShot;

      game.slingshotActive = false;
      float pitch = clampf(facVolume / 1400.0f, 0.5f, 3.0f);
      Mix_PlayChannel(-1, game.sfx[SFX_IDX_BOING], 0);
      game.ballPos = game.ballStart;
    }
  }

  updateShots(delta);
  updateMen(delta);
}

static void draw() {
  // Set render target to virtual screen
  SDL_SetRenderTarget(game.renderer, game.virtualScreen);

  SDL_SetRenderDrawColor(game.renderer, 36, 171, 189, 255);
  SDL_RenderClear(game.renderer);

  // Draw sky background
  SDL_Rect destRect = {0, 0, game.virtualWidth, game.virtualHeight};
  SDL_RenderCopy(game.renderer, game.textures[TEX_IDX_SKY], NULL, &destRect);

  // Draw slingshot
  game.rects[TEX_IDX_SLINGSHOT].x = game.slingshotPos.x;
  game.rects[TEX_IDX_SLINGSHOT].y = game.slingshotPos.y;
  SDL_RenderCopy(game.renderer, game.textures[TEX_IDX_SLINGSHOT], NULL,
                 &game.rects[TEX_IDX_SLINGSHOT]);

  // Draw ball
  game.rects[TEX_IDX_BALL].x = (int)game.ballPos.x;
  game.rects[TEX_IDX_BALL].y = (int)game.ballPos.y;
  SDL_RenderCopy(game.renderer, game.textures[TEX_IDX_BALL], NULL,
                 &game.rects[TEX_IDX_BALL]);

  // Draw shots
  for (int i = 0; i < game.numShots; i++) {
    game.rects[TEX_IDX_BALL].x = (int)game.shots[i].pos.x;
    game.rects[TEX_IDX_BALL].y = (int)game.shots[i].pos.y;
    SDL_RenderCopy(game.renderer, game.textures[TEX_IDX_BALL], NULL,
                   &game.rects[TEX_IDX_BALL]);
  }

  // Draw men
  for (int i = 0; i < game.numMen; i++) {
    game.rects[TEX_IDX_MAN].x = (int)game.men[i].pos.x;
    game.rects[TEX_IDX_MAN].y = (int)game.men[i].pos.y;
    SDL_RenderCopy(game.renderer, game.textures[TEX_IDX_MAN], NULL,
                   &game.rects[TEX_IDX_MAN]);
  }

  renderScore();
  renderSlingshot();

  // Reset render target to window
  SDL_SetRenderTarget(game.renderer, NULL);
  SDL_RenderClear(game.renderer);

  // Draw virtual screen directly to window without letterboxing
  SDL_RenderCopy(game.renderer, game.virtualScreen, NULL, &game.viewport);
  SDL_RenderPresent(game.renderer);
}

static void renderScore(void) {
  SDL_Color textColor = {255, 255, 255, 255};
  char scoreText[64];
  snprintf(scoreText, sizeof(scoreText), "Score: %d", game.score);

  SDL_Surface *scoreSurface =
      TTF_RenderText_Blended(game.font, scoreText, textColor);
  SDL_Texture *scoreTexture =
      SDL_CreateTextureFromSurface(game.renderer, scoreSurface);

  SDL_Rect destRect = {24, 24, scoreSurface->w, scoreSurface->h};
  SDL_RenderCopy(game.renderer, scoreTexture, NULL, &destRect);

  SDL_FreeSurface(scoreSurface);
  SDL_DestroyTexture(scoreTexture);

  if (game.hiscore > 0) {
    char hiscoreText[64];
    snprintf(hiscoreText, sizeof(hiscoreText), "Hi-Score: %d", game.hiscore);

    SDL_Surface *hiscoreSurface =
        TTF_RenderText_Blended(game.font, hiscoreText, textColor);
    SDL_Texture *hiscoreTexture =
        SDL_CreateTextureFromSurface(game.renderer, hiscoreSurface);

    destRect.x = game.virtualWidth - hiscoreSurface->w - 24;
    destRect.w = hiscoreSurface->w;
    destRect.h = hiscoreSurface->h;
    SDL_RenderCopy(game.renderer, hiscoreTexture, NULL, &destRect);

    SDL_FreeSurface(hiscoreSurface);
    SDL_DestroyTexture(hiscoreTexture);
  }
}

static void renderSlingshot(void) {
  float widthRatio = (float)game.virtualWidth / VIRTUAL_WIDTH;
  float heightRatio = (float)game.virtualHeight / VIRTUAL_HEIGHT;

  SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
  SDL_RenderDrawLine(
      game.renderer, (int)(310 * widthRatio), (int)(500 * heightRatio),
      (int)(game.ballPos.x + game.dim.x), (int)(game.ballPos.y + game.dim.y));
  SDL_RenderDrawLine(
      game.renderer, (int)(418 * widthRatio), (int)(500 * heightRatio),
      (int)(game.ballPos.x + game.dim.x), (int)(game.ballPos.y + game.dim.y));
}

static void updateShots(float delta) {
  float screenWidth = (float)game.virtualWidth;
  float screenHeight = (float)game.virtualHeight;
  float gravity = 980.0f;
  float dim = 32.0f;

  for (int i = 0; i < game.numShots; ++i) {
    Shot *shot = &game.shots[i];

    shot->vel.y += gravity * delta;
    vec2 scaledVel = vec2_scale(shot->vel, delta);
    shot->pos = vec2_add(shot->pos, scaledVel);

    if (shot->pos.x > screenWidth + dim || shot->pos.x < -dim ||
        shot->pos.y > screenHeight + dim || shot->pos.y < -dim) {
      for (int j = i; j < game.numShots - 1; ++j) {
        game.shots[j] = game.shots[j + 1];
      }
      --game.numShots;
      --i;
    }
  }
}

static void updateMen(float delta) {
  for (int i = 0; i < game.numMen; ++i) {
    Man *man = &game.men[i];
    man->pos.y += man->speed * delta;

    if (man->pos.y > game.virtualHeight) {
      game.hiscore = (game.score > game.hiscore) ? game.score : game.hiscore;
      game.score = 0;
      game.numMen = 0;
      resetSpawnInterval();
      return;
    }

    for (int j = 0; j < game.numShots; ++j) {
      Shot *shot = &game.shots[j];

      if (vec2_distance(shot->pos, man->pos) <= 64.0f) {
        Mix_PlayChannel(-1, game.sfx[SFX_IDX_OOF], 0);
        for (int k = i; k < game.numMen - 1; ++k) {
          game.men[k] = game.men[k + 1];
        }
        --game.numMen;
        ++game.score;
        break;
      }
    }
  }
}

static void spawnMan() {
  float x = (float)(rand() % (game.virtualWidth - 32));
  float y = -100.0f;
  float speed = (48.0f + ((float)rand() / RAND_MAX) * 100.0f);

  int i = game.numMen % NUM_MEN;
  game.men[i].pos = vec2_new(x, y);
  game.men[i].speed = speed;
  ++game.numMen;
}

static Uint32 spawnManCallback(Uint32 interval, void *param) {
  spawnMan();
  return interval;
}

static Uint32 decreaseSpawnIntervalCallback(Uint32 interval, void *param) {
  decreaseSpawnInterval();
  return interval;
}

static void scheduleTasks() {
  game.spawnTimerId =
      SDL_AddTimer((Uint32)(game.spawnInterval * 1000), spawnManCallback, NULL);
  game.rateTimerId = SDL_AddTimer(10000, decreaseSpawnIntervalCallback, NULL);
}

static void decreaseSpawnInterval() {
  game.spawnInterval =
      (game.spawnInterval * 0.75f > 0.01f) ? game.spawnInterval * 0.75f : 0.01f;

  if (game.spawnTimerId) {
    SDL_RemoveTimer(game.spawnTimerId);
  }
  game.spawnTimerId =
      SDL_AddTimer((Uint32)(game.spawnInterval * 1000), spawnManCallback, NULL);
}

static void resetSpawnInterval() {
  game.spawnInterval = game.beginSpawnInterval;

  if (game.spawnTimerId) {
    SDL_RemoveTimer(game.spawnTimerId);
  }
  game.spawnTimerId =
      SDL_AddTimer((Uint32)(game.spawnInterval * 1000), spawnManCallback, NULL);

  if (game.rateTimerId) {
    SDL_RemoveTimer(game.rateTimerId);
  }
  game.rateTimerId = SDL_AddTimer(10000, decreaseSpawnIntervalCallback, NULL);
}