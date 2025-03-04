/*
  vectoroids.c
  
  An asteroid shooting game with vector graphics.
  Based on "Agendaroids."
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/vectoroids/
  
  November 30, 2001 - January 30, 2025
*/

#define VER_VERSION "1.1.3"
#define VER_DATE "2025.01.30"

#ifndef EMBEDDED
#define STATE_FORMAT_VERSION "2025.01.24"
#else
#define STATE_FORMAT_VERSION "2025.01.24e"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#ifndef NOSOUND
#include <SDL2/SDL_mixer.h>
#endif


#ifndef DATA_PREFIX
#define DATA_PREFIX "data/"
#endif


/* Constraints: */

#define NUM_BULLETS 3

#ifndef EMBEDDED
#define NUM_ASTEROIDS 20
#define NUM_BITS 50
#else
#define NUM_ASTEROIDS 15
#define NUM_BITS 25
#endif

#define AST_SIDES 6
#ifndef EMBEDDED
#define AST_RADIUS 10
#define SHIP_RADIUS 20
#else
#define AST_RADIUS 7
#define SHIP_RADIUS 12
#endif

#define ZOOM_START 40
#define ONEUP_SCORE 10000
#define FPS 50

#ifndef EMBEDDED
#define WIDTH 640
#define HEIGHT 480
#else
#define WIDTH 240
#define HEIGHT 320
#endif


Uint8 drawn_at[HEIGHT + 1][WIDTH + 1];


enum
{ FALSE, TRUE };

#define LEFT_EDGE   0x0001
#define RIGHT_EDGE  0x0002
#define TOP_EDGE    0x0004
#define BOTTOM_EDGE 0x0008


/* Types: */

typedef struct letter_type
{
  int x, y;
  int xm, ym;
} letter_type;

typedef struct bullet_type
{
  int timer;
  int x, y;
  int xm, ym;
} bullet_type;

typedef struct shape_type
{
  int radius;
  int angle;
} shape_type;

typedef struct asteroid_type
{
  int alive, size;
  int x, y;
  int xm, ym;
  int angle, angle_m;
  shape_type shape[AST_SIDES];
} asteroid_type;

typedef struct bit_type
{
  int timer;
  int x, y;
  int xm, ym;
} bit_type;

typedef struct color_type
{
  Uint8 r;
  Uint8 g;
  Uint8 b;
} color_type;


/* Data: */

enum
{
  SND_BULLET,
  SND_AST1,
  SND_AST2,
  SND_AST3,
  SND_AST4,
  SND_THRUST,
  SND_EXPLODE,
  SND_GAMEOVER,
  SND_EXTRALIFE,
  NUM_SOUNDS
};

char *sound_names[NUM_SOUNDS] = {
  DATA_PREFIX "sounds/bullet.wav",
  DATA_PREFIX "sounds/ast1.wav",
  DATA_PREFIX "sounds/ast2.wav",
  DATA_PREFIX "sounds/ast3.wav",
  DATA_PREFIX "sounds/ast4.wav",
  DATA_PREFIX "sounds/thrust.wav",
  DATA_PREFIX "sounds/explode.wav",
  DATA_PREFIX "sounds/gameover.wav",
  DATA_PREFIX "sounds/extralife.wav"
};

#define CHAN_THRUST 0

char *mus_game_name = DATA_PREFIX "music/decision.s3m";


#ifdef JOY_YES
#define JOY_A 0
#define JOY_B 1
#define JOY_X 0
#define JOY_Y 1
#endif


/* Globals: */

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *bkgd;
SDL_Texture *bkgdTexture;
SDL_Surface *screen;
SDL_Texture *screenTexture;
#ifndef NOSOUND
Mix_Chunk *sounds[NUM_SOUNDS];
Mix_Music *game_music;
#endif
#ifdef JOY_YES
SDL_Joystick *js;
#endif
bullet_type bullets[NUM_BULLETS];
asteroid_type asteroids[NUM_ASTEROIDS];
bit_type bits[NUM_BITS];
int use_sound, use_joystick, fullscreen, text_zoom;
char zoom_str[24];
int x, y, xm, ym, angle;
int player_alive, player_die_timer;
int lives, score, high, level, game_pending;


/* Trig junk:  (thanks to Atari BASIC for this) */

int trig[12] = {
  1024,
  1014,
  984,
  935,
  868,
  784,
  685,
  572,
  448,
  316,
  117,
  0
};


/* Characters: */

int char_vectors[37][5][4] = {
  {
   /* 0 */
   {0, 0, 1, 0},
   {1, 0, 1, 2},
   {1, 2, 0, 2},
   {0, 2, 0, 0},
   {-1, -1, -1, -1}
   },

  {
   /* 1 */
   {1, 0, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* 2 */
   {1, 0, 0, 0},
   {1, 0, 1, 1},
   {0, 1, 1, 1},
   {0, 1, 0, 2},
   {1, 2, 0, 2},
   },

  {
   /* 3 */
   {0, 0, 1, 0},
   {1, 0, 1, 2},
   {0, 1, 1, 1},
   {0, 2, 1, 2},
   {-1, -1, -1, -1}
   },

  {
   /* 4 */
   {1, 0, 1, 2},
   {0, 0, 0, 1},
   {0, 1, 1, 1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* 5 */
   {1, 0, 0, 0},
   {0, 0, 0, 1},
   {0, 1, 1, 1},
   {1, 1, 1, 2},
   {1, 2, 0, 2}
   },

  {
   /* 6 */
   {1, 0, 0, 0},
   {0, 0, 0, 2},
   {0, 2, 1, 2},
   {1, 2, 1, 1},
   {1, 1, 0, 1}
   },

  {
   /* 7 */
   {0, 0, 1, 0},
   {1, 0, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* 8 */
   {0, 0, 1, 0},
   {0, 0, 0, 2},
   {1, 0, 1, 2},
   {0, 2, 1, 2},
   {0, 1, 1, 1}
   },

  {
   /* 9 */
   {1, 0, 1, 2},
   {0, 0, 1, 0},
   {0, 0, 0, 1},
   {0, 1, 1, 1},
   {-1, -1, -1, -1}
   },

  {
   /* A */
   {0, 2, 0, 1},
   {0, 1, 1, 0},
   {1, 0, 1, 2},
   {0, 1, 1, 1},
   {-1, -1, -1, -1}
   },

  {
   /* B */
   {0, 2, 0, 0},
   {0, 0, 1, 0},
   {1, 0, 0, 1},
   {0, 1, 1, 2},
   {1, 2, 0, 2}
   },

  {
   /* C */
   {1, 0, 0, 0},
   {0, 0, 0, 2},
   {0, 2, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* D */
   {0, 0, 1, 1},
   {1, 1, 0, 2},
   {0, 2, 0, 0},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* E */
   {1, 0, 0, 0},
   {0, 0, 0, 2},
   {0, 2, 1, 2},
   {0, 1, 1, 1},
   {-1, -1, -1, -1}
   },

  {
   /* F */
   {1, 0, 0, 0},
   {0, 0, 0, 2},
   {0, 1, 1, 1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* G */
   {1, 0, 0, 0},
   {0, 0, 0, 2},
   {0, 2, 1, 2},
   {1, 2, 1, 1},
   {-1, -1, -1, -1}
   },

  {
   /* H */
   {0, 0, 0, 2},
   {1, 0, 1, 2},
   {0, 1, 1, 1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* I */
   {1, 0, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* J */
   {1, 0, 1, 2},
   {1, 2, 0, 2},
   {0, 2, 0, 1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* K */
   {0, 0, 0, 2},
   {1, 0, 0, 1},
   {0, 1, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* L */
   {0, 0, 0, 2},
   {0, 2, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* M */
   {0, 0, 0, 2},
   {1, 0, 1, 2},
   {0, 0, 1, 1},
   {0, 1, 1, 0},
   {-1, -1, -1, -1}
   },

  {
   /* N */
   {0, 2, 0, 0},
   {0, 0, 1, 2},
   {1, 2, 1, 0},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* O */
   {0, 0, 1, 0},
   {1, 0, 1, 2},
   {1, 2, 0, 2},
   {0, 2, 0, 0},
   {-1, -1, -1, -1}
   },

  {
   /* P */
   {0, 2, 0, 0},
   {0, 0, 1, 0},
   {1, 0, 1, 1},
   {1, 1, 0, 1},
   {-1, -1, -1, -1}
   },

  {
   /* Q */
   {0, 0, 1, 0},
   {1, 0, 1, 2},
   {1, 2, 0, 2},
   {0, 2, 0, 0},
   {0, 1, 1, 2}
   },

  {
   /* R */
   {0, 2, 0, 0},
   {0, 0, 1, 0},
   {1, 0, 1, 1},
   {1, 1, 0, 1},
   {0, 1, 1, 2}
   },

  {
   /* S */
   {1, 0, 0, 0},
   {0, 0, 0, 1},
   {0, 1, 1, 1},
   {1, 1, 1, 2},
   {1, 2, 0, 2}
   },

  {
   /* T */
   {0, 0, 1, 0},
   {1, 0, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* U */
   {0, 0, 0, 2},
   {0, 2, 1, 2},
   {1, 2, 1, 0},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* V */
   {0, 0, 0, 1},
   {0, 1, 1, 2},
   {1, 2, 1, 0},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* W */
   {0, 0, 0, 2},
   {1, 0, 1, 2},
   {0, 1, 1, 2},
   {0, 2, 1, 1},
   {-1, -1, -1, -1}
   },

  {
   /* X */
   {0, 0, 1, 2},
   {0, 2, 1, 0},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* Y */
   {0, 0, 1, 1},
   {1, 0, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* Z */
   {0, 0, 1, 0},
   {1, 0, 0, 2},
   {0, 2, 1, 2},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   },

  {
   /* . */
   {0, 1, 1, 1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1},
   {-1, -1, -1, -1}
   }
};



/* Local function prototypes: */

int title(void);
int game(void);
void finish(void);
void setup(int argc, char *argv[]);
void seticon(void);
int fast_cos(int v);
int fast_sin(int v);
void draw_line(int x1, int y1, color_type c1, int x2, int y2, color_type c2);
int clip(int *x1, int *y1, int *x2, int *y2);
color_type mkcolor(int r, int g, int b);
void sdl_drawline(int x1, int y1, color_type c1,
                  int x2, int y2, color_type c2);
unsigned char encode(float x, float y);
void drawvertline(int x, int y1, color_type c1, int y2, color_type c2);
void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel);
void draw_segment(int r1, int a1,
                  color_type c1,
                  int r2, int a2, color_type c2, int cx, int cy, int ang);
int add_bullet(int x, int y, int a, int xm, int ym);
void add_asteroid(int x, int y, int xm, int ym, int size);
void add_bit(int x, int y, int xm, int ym);
void draw_asteroid(int size, int x, int y, int angle, shape_type * shape);
void playsound(int snd);
void hurt_asteroid(int j, int xm, int ym, int exp_size);
void add_score(int amount);
void draw_char(char c, int x, int y, int r, color_type cl);
void draw_text(char *str, int x, int y, int s, color_type c);
void draw_thick_line(int x1, int y1, color_type c1,
                     int x2, int y2, color_type c2);
void reset_level(void);
void show_version(void);
void show_usage(FILE * f, char *prg);
void set_vid_mode(unsigned flags);
void draw_centered_text(char *str, int y, int s, color_type c);


/* --- MAIN --- */

int main(int argc, char *argv[])
{
  int done;
  FILE *fi;
  char statefile[256], buf[256];
  char *tmp_str;


  setup(argc, argv);


  /* Set defaults: */

  score = 0;
  high = 0;
  game_pending = 0;


  /* Load state from disk: */

#ifndef _WIN32
  /* snprintf(statefile, sizeof(statefile), "%s/.vectoroids-state",
     getenv("HOME")); */
  snprintf(statefile, sizeof(statefile), "%s/.vectoroids-state", getenv("HOME"));
#else
  snprintf(statefile, sizeof(statefile), "vectoroids-state.dat");
#endif

  fi = fopen(statefile, "r");
  if (fi != NULL)
  {
    /* Skip comment line: */

    tmp_str = fgets(buf, sizeof(buf), fi);

    if (tmp_str != NULL)
    {
      /* Grab statefile version: */

      tmp_str = fgets(buf, sizeof(buf), fi);
    }

    if (tmp_str != NULL)
    {
      buf[strlen(buf) - 1] = '\0';

      if (strcmp(buf, STATE_FORMAT_VERSION) != 0)
      {
        fprintf(stderr, "Vectoroids state file format has been updated.\n"
                "Old game state is unreadable.  Sorry!\n");
      }
      else
      {
        size_t sz;              /* FIXME: Should pay attention to whether we got what we expected! -bjk 2025.01.24 */
        game_pending = fgetc(fi);
        lives = fgetc(fi);
        level = fgetc(fi);
        player_alive = fgetc(fi);
        player_die_timer = fgetc(fi);
        sz = fread(&score, sizeof(int), 1, fi);
        sz = fread(&high, sizeof(int), 1, fi);
        sz = fread(&x, sizeof(int), 1, fi);
        sz = fread(&y, sizeof(int), 1, fi);
        sz = fread(&xm, sizeof(int), 1, fi);
        sz = fread(&ym, sizeof(int), 1, fi);
        sz = fread(&angle, sizeof(int), 1, fi);
        sz = fread(bullets, sizeof(bullet_type), NUM_BULLETS, fi);
        sz = fread(asteroids, sizeof(asteroid_type), NUM_ASTEROIDS, fi);
        sz = fread(bits, sizeof(bit_type), NUM_BITS, fi);
        sz = sz;                /* FIXME */
      }
    }

    fclose(fi);
  }



  /* Main app loop! */

  do
  {
    done = title();

    if (!done)
    {
      done = game();
    }
  }
  while (!done);


  /* Save state: */

  fi = fopen(statefile, "w");
  if (fi == NULL)
  {
    perror(statefile);
  }
  else
  {
    fprintf(fi, "Vectoroids State File\n");
    fprintf(fi, "%s\n", STATE_FORMAT_VERSION);

    fputc(game_pending, fi);
    fputc(lives, fi);
    fputc(level, fi);
    fputc(player_alive, fi);
    fputc(player_die_timer, fi);
    fwrite(&score, sizeof(int), 1, fi);
    fwrite(&high, sizeof(int), 1, fi);
    fwrite(&x, sizeof(int), 1, fi);
    fwrite(&y, sizeof(int), 1, fi);
    fwrite(&xm, sizeof(int), 1, fi);
    fwrite(&ym, sizeof(int), 1, fi);
    fwrite(&angle, sizeof(int), 1, fi);
    fwrite(bullets, sizeof(bullet_type), NUM_BULLETS, fi);
    fwrite(asteroids, sizeof(asteroid_type), NUM_ASTEROIDS, fi);
    fwrite(bits, sizeof(bit_type), NUM_BITS, fi);

    fclose(fi);
  }

  finish();

  return (0);
}


/* Title screen: */

int title(void)
{
  int done, quit, hover;
  int i, snapped, angle, size, counter, x, y, xm, ym, z1, z2, z3;
  SDL_Event event;
  SDL_Keycode key;
  Uint32 now_time, last_time;
  char *titlestr = "VECTOROIDS";
  char str[64];
  letter_type letters[11];
  color_type tmp_color;


  /* Reset letters: */

  snapped = 0;

  for (i = 0; i < strlen(titlestr); i++)
  {
    letters[i].x = (rand() % WIDTH);
    letters[i].y = (rand() % HEIGHT);
    letters[i].xm = 0;
    letters[i].ym = 0;
  }

  x = (rand() % WIDTH);
  y = (rand() % HEIGHT);
  xm = (rand() % 4) + 2;
  ym = (rand() % 10) - 5;

  counter = 0;
  angle = 0;
  size = 40;

  done = 0;
  quit = 0;
  hover = 0;

  do
  {
    last_time = SDL_GetTicks();

    counter++;


    /* Rotate rock: */

    angle = ((angle + 2) % 360);


    /* Make rock grow: */

    if ((counter % 3) == 0)
    {
      if (size > 1)
        size--;
    }


    /* Move rock: */

    x = x + xm;

    if (x >= WIDTH)
      x = x - WIDTH;

    y = y + ym;

    if (y >= HEIGHT)
      y = y - HEIGHT;
    else if (y < 0)
      y = y + HEIGHT;


    /* Handle events: */

    while (SDL_PollEvent(&event) > 0)
    {
      if (event.type == SDL_QUIT)
      {
        done = 1;
        quit = 1;
      }
      else if (event.type == SDL_KEYDOWN)
      {
        key = event.key.keysym.sym;

        if (key == SDLK_SPACE || key == SDLK_RETURN)
        {
          if (hover == 1)
          {
            /* If hovering over "Start [Over]", start new game */
            /* (if hovering over "Continue", or nothing in particular,
               resume the paused game) */
            game_pending = 0;
          }
          done = 1;
        }
        else if (key == SDLK_UP || key == SDLK_DOWN)
        {
          if (key == SDLK_UP)
          {
            hover = hover - 1;
            if (hover < 1)
            {
              if (game_pending)
                hover = 2;
              else
                hover = 1;
            }
          }
          else if (key == SDLK_DOWN)
          {
            hover = hover + 1;
            if ((hover == 2 && !game_pending) || hover > 2)
              hover = 1;
          }

          SDL_WarpMouseInWindow(window, WIDTH / 2, 187 + (hover - 1) * 15);
        }
        else if (key == SDLK_ESCAPE)
        {
          done = 1;
          quit = 1;
        }
      }
#ifdef JOY_YES
      else if (event.type == SDL_JOYBUTTONDOWN)
      {
        done = 1;
      }
#endif
      else if (event.type == SDL_MOUSEBUTTONDOWN)
      {
        if (event.button.x >= (WIDTH - 50) / 2 &&
            event.button.x <= (WIDTH + 50) / 2 &&
            event.button.y >= 180 && event.button.y <= 195)
        {
          /* Start [Over] */

          game_pending = 0;
          done = 1;
        }
        else if (event.button.x >= (WIDTH - 80) / 2 &&
                 event.button.x <= (WIDTH + 80) / 2 &&
                 event.button.y >= 200 && event.button.y <= 215 &&
                 game_pending)
        {
          /* Continue */

          done = 1;
        }
      }
      else if (event.type == SDL_MOUSEMOTION)
      {
        if (event.motion.x >= (WIDTH - 50) / 2 &&
            event.motion.x <= (WIDTH + 50) / 2 &&
            event.motion.y >= 180 && event.motion.y <= 195)
        {
          hover = 1;
        }
        else if (event.motion.x >= (WIDTH - 80) / 2 &&
                 event.motion.x <= (WIDTH + 80) / 2 &&
                 event.motion.y >= 200 && event.motion.y <= 215)
        {
          hover = 2;
        }
        else
        {
          hover = 0;
        }
      }
    }


    /* Move title characters: */

    if (snapped < strlen(titlestr))
    {
      for (i = 0; i < strlen(titlestr); i++)
      {
        letters[i].x = letters[i].x + letters[i].xm;
        letters[i].y = letters[i].y + letters[i].ym;


        /* Home in on final spot! */

        if (letters[i].x > ((WIDTH - (strlen(titlestr) * 14)) / 2 +
                            (i * 14)) && letters[i].xm > -4)
          letters[i].xm--;
        else if (letters[i].x < ((WIDTH - (strlen(titlestr) * 14)) / 2 +
                                 (i * 14)) && letters[i].xm < 4)
          letters[i].xm++;

        if (letters[i].y > 100 && letters[i].ym > -4)
          letters[i].ym--;
        else if (letters[i].y < 100 && letters[i].ym < 4)
          letters[i].ym++;


        /* Snap into place: */

        if (letters[i].x >= ((WIDTH - (strlen(titlestr) * 14)) / 2 +
                             (i * 14)) - 8 &&
            letters[i].x <= ((WIDTH - (strlen(titlestr) * 14)) / 2 +
                             (i * 14)) + 8 &&
            letters[i].y >= 92 &&
            letters[i].y <= 108 && (letters[i].xm != 0 || letters[i].ym != 0))
        {
          letters[i].x = ((WIDTH - (strlen(titlestr) * 14)) / 2 + (i * 14));
          letters[i].xm = 0;

          letters[i].y = 100;
          letters[i].ym = 0;

          snapped++;
        }
      }
    }


    /* Draw screen: */

    /* (Erase first) */

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    memset(drawn_at, 0, sizeof(Uint8) * (HEIGHT + 1) * (WIDTH + 1));


    /* (Title) */

    if (snapped != strlen(titlestr))
    {
      for (i = 0; i < strlen(titlestr); i++)
      {
        draw_char(titlestr[i], letters[i].x, letters[i].y, 10,
                  mkcolor(255, 255, 255));
      }
    }
    else
    {
      for (i = 0; i < strlen(titlestr); i++)
      {
        z1 = (i + counter) % 255;
        z2 = ((i + counter + 128) * 2) % 255;
        z3 = ((i + counter) * 5) % 255;

        draw_char(titlestr[i], letters[i].x, letters[i].y, 10,
                  mkcolor(z1, z2, z3));
      }
    }


    /* (Credits) */

    if (snapped == strlen(titlestr))
    {
      draw_centered_text("BY BILL KENDRICK", 140, 5, mkcolor(128, 128, 128));
      draw_centered_text("NEW BREED SOFTWARE", 155, 5, mkcolor(96, 96, 96));

#ifndef EMBEDDED
      snprintf(str, sizeof(str), "VERSION %s   %s", VER_VERSION, VER_DATE);
#else
      snprintf(str, sizeof(str), "VER %s  %s", VER_VERSION, VER_DATE);
#endif
      draw_centered_text(str, (HEIGHT - 20), 5, mkcolor(96, 96, 96));

      snprintf(str, sizeof(str), "HIGH %.6d", high);
      draw_text(str, (WIDTH - 110) / 2, 5, 5, mkcolor(128, 255, 255));
      draw_text(str, (WIDTH - 110) / 2 + 1, 6, 5, mkcolor(128, 255, 255));

      if (score != 0 && (score != high || (counter % 20) < 10))
      {
        if (game_pending == 0)
          snprintf(str, sizeof(str), "LAST %.6d", score);
        else
          snprintf(str, sizeof(str), "SCR  %.6d", score);
        draw_text(str, (WIDTH - 110) / 2, 25, 5, mkcolor(128, 128, 255));
        draw_text(str, (WIDTH - 110) / 2 + 1, 26, 5, mkcolor(128, 128, 255));
      }
    }


    if (hover == 1)
      tmp_color = mkcolor(255, 255, 255);
    else
      tmp_color = mkcolor(0, 255, 0);

    if (game_pending)
    {
      draw_text("START OVER", (WIDTH - 100) / 2, 180, 5, tmp_color);

      if (hover == 2)
        tmp_color = mkcolor(255, 255, 255);
      else
        tmp_color = mkcolor(0, 255, 0);

      draw_text("CONTINUE", (WIDTH - 80) / 2, 200, 5, tmp_color);
    }
    else
    {
      draw_text("START", (WIDTH - 50) / 2, 180, 5, tmp_color);
    }


    /* (Giant rock) */

    draw_segment(40 / size, 0, mkcolor(255, 255, 255),
                 30 / size, 30, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(30 / size, 30, mkcolor(255, 255, 255),
                 40 / size, 55, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(40 / size, 55, mkcolor(255, 255, 255),
                 25 / size, 90, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(25 / size, 90, mkcolor(255, 255, 255),
                 40 / size, 120, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(40 / size, 120, mkcolor(255, 255, 255),
                 35 / size, 130, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(35 / size, 130, mkcolor(255, 255, 255),
                 40 / size, 160, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(40 / size, 160, mkcolor(255, 255, 255),
                 30 / size, 200, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(30 / size, 200, mkcolor(255, 255, 255),
                 45 / size, 220, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(45 / size, 220, mkcolor(255, 255, 255),
                 25 / size, 265, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(25 / size, 265, mkcolor(255, 255, 255),
                 30 / size, 300, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(30 / size, 300, mkcolor(255, 255, 255),
                 45 / size, 335, mkcolor(255, 255, 255), x, y, angle);
    draw_segment(45 / size, 335, mkcolor(255, 255, 255),
                 40 / size, 0, mkcolor(255, 255, 255), x, y, angle);


    /* Flush and pause! */

    /* SDL_Flip(screen); *//* SDL1.2 method */
    SDL_UpdateTexture(screenTexture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bkgdTexture, NULL, NULL);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    now_time = SDL_GetTicks();

    if (now_time < last_time + (1000 / FPS))
    {
      SDL_Delay(last_time + 1000 / FPS - now_time);
    }
  }
  while (!done);

  SDL_WarpMouseInWindow(window, WIDTH - 5, HEIGHT - 5);

  return (quit);
}



/* --- GAME --- */

#define TAP_V_TOP (HEIGHT / 2)
#define TAP_V_MID (HEIGHT / 2 + HEIGHT / 6)
#define TAP_V_BOT (HEIGHT / 2 + HEIGHT / 3)
#define TAP_V_FARBOT (HEIGHT - 1)

#define TAP_H_FARLFT 0
#define TAP_H_LFT (WIDTH / 3)
#define TAP_H_CTR (WIDTH / 2)
#define TAP_H_RGT (WIDTH * 2 / 3)
#define TAP_H_FARRGT (WIDTH - 1)

void handle_click_tap_controls(int x, int y, int *lft, int *rgt, int *up,
                               int *fire)
{
  *up = 0;
  *lft = 0;
  *rgt = 0;
  *fire = 0;

  if (y >= TAP_V_TOP && y < TAP_V_MID && x >= TAP_H_LFT && x < TAP_H_RGT)
  {
    *up = 1;
  }
  else if (y >= TAP_V_MID && y < TAP_V_BOT)
  {
    if (x >= TAP_H_FARLFT && x < TAP_H_CTR)
      *lft = 1;
    else if (x >= TAP_H_CTR && x < TAP_H_FARRGT)
      *rgt = 1;
  }
  else if (y >= TAP_V_BOT && y < TAP_V_FARBOT && x >= TAP_H_LFT
           && x < TAP_H_RGT)
  {
    *fire = 1;
  }
}

int game(void)
{
  int done, quit, counter;
  int i, j;
  int num_asteroids_alive;
  SDL_Event event;
  SDL_Keycode key;
  int left_pressed, right_pressed, up_pressed, shift_pressed;
  int fire_pressed, firing;
  char str[32];
  Uint32 now_time, last_time;
  color_type tmp_color;
  int tap_area_brightness;


  done = 0;
  quit = 0;
  counter = 0;

  left_pressed = 0;
  right_pressed = 0;
  up_pressed = 0;
  shift_pressed = 0;
  fire_pressed = 0;
  firing = 0;
  tap_area_brightness = 0;

  if (game_pending == 0)
  {
    lives = 3;
    score = 0;

    player_alive = 1;
    player_die_timer = 0;
    angle = 90;
    x = (WIDTH / 2) << 4;
    y = (HEIGHT / 2) << 4;
    xm = 0;
    ym = 0;

    level = 1;
    reset_level();
  }

  game_pending = 1;


  /* Hide mouse cursor: */

  if (fullscreen)
    SDL_ShowCursor(0);


  /* Play music: */

#ifndef NOSOUND
  if (use_sound)
  {
    if (!Mix_PlayingMusic())
      Mix_PlayMusic(game_music, -1);
  }
#endif


  do
  {
    last_time = SDL_GetTicks();
    counter++;


    /* Handle events: */

    while (SDL_PollEvent(&event) > 0)
    {
      if (event.type == SDL_QUIT)
      {
        /* Quit! */

        done = 1;
        quit = 1;
      }
      else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      {
        key = event.key.keysym.sym;

        if (event.type == SDL_KEYDOWN)
        {
          if (key == SDLK_ESCAPE)
          {
            /* Return to menu! */

            done = 1;
          }


          /* Key press... */

          if (key == SDLK_RIGHT)
          {
            /* Rotate CW */

            left_pressed = 0;
            right_pressed = 1;
          }
          else if (key == SDLK_LEFT)
          {
            /* Rotate CCW */

            left_pressed = 1;
            right_pressed = 0;
          }
          else if (key == SDLK_UP)
          {
            /* Thrust! */

            up_pressed = 1;
          }
          else if (key == SDLK_SPACE)
          {
            /* Fire a bullet! */

            fire_pressed = 1;
          }

          if (key == SDLK_LSHIFT || key == SDLK_RSHIFT)
          {
            /* Respawn now (if applicable) */

            shift_pressed = 1;
          }
        }
        else if (event.type == SDL_KEYUP)
        {
          /* Key release... */

          if (key == SDLK_RIGHT)
          {
            right_pressed = 0;
          }
          else if (key == SDLK_LEFT)
          {
            left_pressed = 0;
          }
          else if (key == SDLK_UP)
          {
            up_pressed = 0;
          }
          else if (key == SDLK_SPACE)
          {
            fire_pressed = 0;
            firing = 0;
          }

          if (key == SDLK_LSHIFT || key == SDLK_RSHIFT)
          {
            /* Respawn now (if applicable) */

            shift_pressed = 0;
          }
        }
      }
#ifdef JOY_YES
      else if (event.type == SDL_JOYBUTTONDOWN && player_alive)
      {
        if (event.jbutton.button == JOY_B)
        {
          /* Fire a bullet! */

          fire_pressed = 1;
        }
        else if (event.jbutton.button == JOY_A)
        {
          /* Thrust: */

          up_pressed = 1;
        }
        else
        {
          shift_pressed = 1;
        }
      }
      else if (event.type == SDL_JOYBUTTONUP)
      {
        if (event.jbutton.button == JOY_B)
        {
          /* Release firebutton: */

          fire_pressed = 0;
          firing = 0;
        }
        else if (event.jbutton.button == JOY_A)
        {
          /* Stop thrust: */

          up_pressed = 0;
        }
        else if (event.jbutton.button != JOY_B)
        {
          /* Any other button: respawn */

          shift_pressed = 0;
        }
      }
      else if (event.type == SDL_JOYAXISMOTION)
      {
        if (event.jaxis.axis == JOY_X)
        {
          if (event.jaxis.value < -256)
          {
            left_pressed = 1;
            right_pressed = 0;
          }
          else if (event.jaxis.value > 256)
          {
            left_pressed = 0;
            right_pressed = 1;
          }
          else
          {
            left_pressed = 0;
            right_pressed = 0;
          }
        }
      }
#endif
      else if (event.type == SDL_MOUSEMOTION)
      {
        tap_area_brightness = 255;

        if (SDL_GetMouseState(NULL, NULL) != 0)
          handle_click_tap_controls(event.motion.x, event.motion.y,
                                    &left_pressed, &right_pressed,
                                    &up_pressed, &fire_pressed);
      }
      else if (event.type == SDL_MOUSEBUTTONDOWN)
      {
        tap_area_brightness = 255;

        handle_click_tap_controls(event.button.x, event.button.y,
                                  &left_pressed, &right_pressed, &up_pressed,
                                  &fire_pressed);
      }
      else if (event.type == SDL_MOUSEBUTTONUP)
      {
        left_pressed = 0;
        right_pressed = 0;
        up_pressed = 0;
        fire_pressed = 0;
        firing = 0;
      }
    }


    /* Rotate ship: */

    if (right_pressed)
    {
      angle = angle - 8;
      if (angle < 0)
        angle = angle + 360;
    }
    else if (left_pressed)
    {
      angle = angle + 8;
      if (angle >= 360)
        angle = angle - 360;
    }

    /* Fire bullets */
    if (fire_pressed && player_alive)
    {
      if (!firing)
      {
        firing = add_bullet(x >> 4, y >> 4, angle, xm, ym);
      }
    }

    /* Thrust ship: */

    if (up_pressed && player_alive)
    {
      /* Move forward: */

      xm = xm + ((fast_cos(angle >> 3) * 3) >> 10);
      ym = ym - ((fast_sin(angle >> 3) * 3) >> 10);


      /* Start thruster sound: */
#ifndef NOSOUND
      if (use_sound)
      {
        if (!Mix_Playing(CHAN_THRUST))
        {
#ifndef EMBEDDED
          Mix_PlayChannel(CHAN_THRUST, sounds[SND_THRUST], -1);
#else
          Mix_PlayChannel(-1, sounds[SND_THRUST], 0);
#endif
        }
      }
#endif
    }
    else
    {
      /* Slow down (unrealistic, but.. feh!) */

      if ((counter % 20) == 0)
      {
        xm = (xm * 7) / 8;
        ym = (ym * 7) / 8;
      }


      /* Stop thruster sound: */

#ifndef NOSOUND
      if (use_sound)
      {
        if (Mix_Playing(CHAN_THRUST))
        {
#ifndef EMBEDDED
          Mix_HaltChannel(CHAN_THRUST);
#endif
        }
      }
#endif
    }


    /* Handle player death: */

    if (player_alive == 0)
    {
      player_die_timer--;

      if (player_die_timer <= 0)
      {
        if (lives > 0)
        {
          /* Reset player: */

          player_die_timer = 0;
          angle = 90;
          x = (WIDTH / 2) << 4;
          y = (HEIGHT / 2) << 4;
          xm = 0;
          ym = 0;


          /* Only bring player back when it's alright to! */

          player_alive = 1;

          if (!shift_pressed)
          {
            for (i = 0; i < NUM_ASTEROIDS && player_alive; i++)
            {
              if (asteroids[i].alive)
              {
                if (asteroids[i].x >= (x >> 4) - (WIDTH / 5) &&
                    asteroids[i].x <= (x >> 4) + (WIDTH / 5) &&
                    asteroids[i].y >= (y >> 4) - (HEIGHT / 5) &&
                    asteroids[i].y <= (y >> 4) + (HEIGHT / 5))
                {
                  /* If any asteroid is too close for comfort,
                     don't bring ship back yet! */

                  player_alive = 0;
                }
              }
            }
          }
        }
        else
        {
          done = 1;
          game_pending = 0;
        }
      }
    }


    /* Erase screen: */

    SDL_BlitSurface(bkgd, NULL, screen, NULL);
    memset(drawn_at, 0, sizeof(Uint8) * (HEIGHT + 1) * (WIDTH + 1));


    /* Draw click/tap-based control area */
    if (tap_area_brightness >= 128)
    {
      tmp_color =
        mkcolor(tap_area_brightness >> 1, tap_area_brightness >> 1,
                tap_area_brightness);

      draw_line(TAP_H_LFT, TAP_V_TOP, tmp_color, TAP_H_RGT, TAP_V_TOP,
                tmp_color);
      draw_line(TAP_H_FARLFT, TAP_V_MID, tmp_color, TAP_H_FARRGT, TAP_V_MID,
                tmp_color);
      draw_line(TAP_H_FARLFT, TAP_V_BOT, tmp_color, TAP_H_FARRGT, TAP_V_BOT,
                tmp_color);

      draw_line(TAP_H_LFT, TAP_V_TOP, tmp_color, TAP_H_LFT, TAP_V_MID,
                tmp_color);
      draw_line(TAP_H_RGT, TAP_V_TOP, tmp_color, TAP_H_RGT, TAP_V_MID,
                tmp_color);

      draw_line(TAP_H_CTR, TAP_V_MID, tmp_color, TAP_H_CTR, TAP_V_BOT,
                tmp_color);

      draw_line(TAP_H_LFT, TAP_V_BOT, tmp_color, TAP_H_LFT, TAP_V_FARBOT,
                tmp_color);
      draw_line(TAP_H_RGT, TAP_V_BOT, tmp_color, TAP_H_RGT, TAP_V_FARBOT,
                tmp_color);

      /* It fades out if you're not using it */
      tap_area_brightness--;
    }

    /* Move ship: */

    x = x + xm;
    y = y + ym;


    /* Wrap ship around edges of screen: */

    if (x >= (WIDTH << 4))
      x = x - (WIDTH << 4);
    else if (x < 0)
      x = x + (WIDTH << 4);

    if (y >= (HEIGHT << 4))
      y = y - (HEIGHT << 4);
    else if (y < 0)
      y = y + (HEIGHT << 4);


    /* Move bullets: */

    for (i = 0; i < NUM_BULLETS; i++)
    {
      if (bullets[i].timer >= 0)
      {
        /* Bullet wears out: */

        bullets[i].timer--;


        /* Move bullet: */

        bullets[i].x = bullets[i].x + bullets[i].xm;
        bullets[i].y = bullets[i].y + bullets[i].ym;


        /* Wrap bullet around edges of screen: */

        if (bullets[i].x >= WIDTH)
          bullets[i].x = bullets[i].x - WIDTH;
        else if (bullets[i].x < 0)
          bullets[i].x = bullets[i].x + WIDTH;

        if (bullets[i].y >= HEIGHT)
          bullets[i].y = bullets[i].y - HEIGHT;
        else if (bullets[i].y < 0)
          bullets[i].y = bullets[i].y + HEIGHT;


        /* Check for collision with any asteroids! */

        for (j = 0; j < NUM_ASTEROIDS; j++)
        {
          if (bullets[i].timer > 0 && asteroids[j].alive)
          {
            if ((bullets[i].x + 5 >=
                 asteroids[j].x - asteroids[j].size * AST_RADIUS) &&
                (bullets[i].x - 5 <=
                 asteroids[j].x + asteroids[j].size * AST_RADIUS) &&
                (bullets[i].y + 5 >=
                 asteroids[j].y - asteroids[j].size * AST_RADIUS) &&
                (bullets[i].y - 5 <=
                 asteroids[j].y + asteroids[j].size * AST_RADIUS))
            {
              /* Remove bullet! */

              bullets[i].timer = 0;


              hurt_asteroid(j, bullets[i].xm, bullets[i].ym,
                            asteroids[j].size * 3);
            }
          }
        }
      }
    }


    /* Move asteroids: */

    num_asteroids_alive = 0;

    for (i = 0; i < NUM_ASTEROIDS; i++)
    {
      if (asteroids[i].alive)
      {
        num_asteroids_alive++;

        /* Move asteroid: */

        if ((counter % 4) == 0)
        {
          asteroids[i].x = asteroids[i].x + asteroids[i].xm;
          asteroids[i].y = asteroids[i].y + asteroids[i].ym;
        }


        /* Wrap asteroid around edges of screen: */

        if (asteroids[i].x >= WIDTH)
          asteroids[i].x = asteroids[i].x - WIDTH;
        else if (asteroids[i].x < 0)
          asteroids[i].x = asteroids[i].x + WIDTH;

        if (asteroids[i].y >= HEIGHT)
          asteroids[i].y = asteroids[i].y - HEIGHT;
        else if (asteroids[i].y < 0)
          asteroids[i].y = asteroids[i].y + HEIGHT;


        /* Rotate asteroid: */

        asteroids[i].angle = (asteroids[i].angle + asteroids[i].angle_m);


        /* Wrap rotation angle... */

        if (asteroids[i].angle < 0)
          asteroids[i].angle = asteroids[i].angle + 360;
        else if (asteroids[i].angle >= 360)
          asteroids[i].angle = asteroids[i].angle - 360;


        /* See if we collided with the player: */

        if (asteroids[i].x >= (x >> 4) - SHIP_RADIUS &&
            asteroids[i].x <= (x >> 4) + SHIP_RADIUS &&
            asteroids[i].y >= (y >> 4) - SHIP_RADIUS &&
            asteroids[i].y <= (y >> 4) + SHIP_RADIUS && player_alive)
        {
          hurt_asteroid(i, xm >> 4, ym >> 4, NUM_BITS);

          player_alive = 0;
          player_die_timer = 30;

          playsound(SND_EXPLODE);

          /* Stop thruster sound: */

#ifndef NOSOUND
          if (use_sound)
          {
            if (Mix_Playing(CHAN_THRUST))
            {
#ifndef EMBEDDED
              Mix_HaltChannel(CHAN_THRUST);
#endif
            }
          }
#endif

          lives--;

          if (lives == 0)
          {
#ifndef NOSOUND
            if (use_sound)
            {
              playsound(SND_GAMEOVER);
              playsound(SND_GAMEOVER);
              playsound(SND_GAMEOVER);
              /* Mix_PlayChannel(CHAN_THRUST,
                 sounds[SND_GAMEOVER], 0); */
            }
#endif
            player_die_timer = 100;
          }
        }
      }
    }


    /* Move bits: */

    for (i = 0; i < NUM_BITS; i++)
    {
      if (bits[i].timer > 0)
      {
        /* Countdown bit's lifespan: */

        bits[i].timer--;


        /* Move the bit: */

        bits[i].x = bits[i].x + bits[i].xm;
        bits[i].y = bits[i].y + bits[i].ym;


        /* Wrap bit around edges of screen: */

        if (bits[i].x >= WIDTH)
          bits[i].x = bits[i].x - WIDTH;
        else if (bits[i].x < 0)
          bits[i].x = bits[i].x + WIDTH;

        if (bits[i].y >= HEIGHT)
          bits[i].y = bits[i].y - HEIGHT;
        else if (bits[i].y < 0)
          bits[i].y = bits[i].y + HEIGHT;
      }
    }


    /* Draw asteroids: */

    for (i = 0; i < NUM_ASTEROIDS; i++)
    {
      if (asteroids[i].alive)
      {
        draw_asteroid(asteroids[i].size,
                      asteroids[i].x, asteroids[i].y,
                      asteroids[i].angle, asteroids[i].shape);
      }
    }


    /* Draw bits: */

    for (i = 0; i < NUM_BITS; i++)
    {
      if (bits[i].timer > 0)
      {
        draw_line(bits[i].x, bits[i].y, mkcolor(255, 255, 255),
                  bits[i].x + bits[i].xm,
                  bits[i].y + bits[i].ym, mkcolor(255, 255, 255));
      }
    }


    /* Draw score: */

#ifndef EMBEDDED
    snprintf(str, sizeof(str), "SCORE %.6d", score);
    draw_text(str, 3, 3, 14, mkcolor(255, 255, 255));
    draw_text(str, 4, 4, 14, mkcolor(255, 255, 255));
#else
    snprintf(str, sizeof(str), "%.6d", score);
    draw_text(str, 3, 3, 10, mkcolor(255, 255, 255));
    draw_text(str, 4, 4, 10, mkcolor(255, 255, 255));
#endif


    /* Level: */

#ifndef EMBEDDED
    snprintf(str, sizeof(str), "LEVEL %d", level);
    draw_text(str, (WIDTH - strlen(str) * 14) / 2, 3, 14,
              mkcolor(255, 255, 255));
    draw_text(str, (WIDTH - strlen(str) * 14) / 2 + 1, 4, 14,
              mkcolor(255, 255, 255));
#else
    snprintf(str, sizeof(str), "%d", level);
    draw_text(str, (WIDTH - 14) / 2, 3, 10, mkcolor(255, 255, 255));
    draw_text(str, (WIDTH - 14) / 2 + 1, 4, 10, mkcolor(255, 255, 255));
#endif


    /* Draw lives: */

    for (i = 0; i < lives; i++)
    {
      draw_segment(16, 0, mkcolor(255, 255, 255),
                   4, 135, mkcolor(255, 255, 255),
                   WIDTH - 10 - i * 10, 20, 90);

      draw_segment(8, 135, mkcolor(255, 255, 255),
                   0, 0, mkcolor(255, 255, 255), WIDTH - 10 - i * 10, 20, 90);

      draw_segment(0, 0, mkcolor(255, 255, 255),
                   8, 225, mkcolor(255, 255, 255),
                   WIDTH - 10 - i * 10, 20, 90);

      draw_segment(8, 225, mkcolor(255, 255, 255),
                   16, 0, mkcolor(255, 255, 255),
                   WIDTH - 10 - i * 10, 20, 90);
    }


    if (player_die_timer > 0)
    {
      if (player_die_timer > 30)
        j = 30;
      else
        j = player_die_timer;

      draw_segment((16 * j) / 30, 0, mkcolor(255, 255, 255),
                   (4 * j) / 30, 135, mkcolor(255, 255, 255),
                   WIDTH - 10 - i * 10, 20, 90);

      draw_segment((8 * j) / 30, 135, mkcolor(255, 255, 255),
                   0, 0, mkcolor(255, 255, 255), WIDTH - 10 - i * 10, 20, 90);

      draw_segment(0, 0, mkcolor(255, 255, 255),
                   (8 * j) / 30, 225, mkcolor(255, 255, 255),
                   WIDTH - 10 - i * 10, 20, 90);

      draw_segment((8 * j) / 30, 225, mkcolor(255, 255, 255),
                   (16 * j) / 30, 0, mkcolor(255, 255, 255),
                   WIDTH - 10 - i * 10, 20, 90);

    }


    /* Draw ship: */

    if (player_alive)
    {
      draw_segment(SHIP_RADIUS, 0, mkcolor(128, 128, 255),
                   SHIP_RADIUS / 2, 135, mkcolor(0, 0, 192),
                   x >> 4, y >> 4, angle);

      draw_segment(SHIP_RADIUS / 2, 135, mkcolor(0, 0, 192),
                   0, 0, mkcolor(64, 64, 230), x >> 4, y >> 4, angle);

      draw_segment(0, 0, mkcolor(64, 64, 230),
                   SHIP_RADIUS / 2, 225, mkcolor(0, 0, 192),
                   x >> 4, y >> 4, angle);

      draw_segment(SHIP_RADIUS / 2, 225, mkcolor(0, 0, 192),
                   SHIP_RADIUS, 0, mkcolor(128, 128, 255),
                   x >> 4, y >> 4, angle);


      /* Draw flame: */

      if (up_pressed)
      {
#ifndef EMBEDDED
        draw_segment(0, 0, mkcolor(255, 255, 255),
                     (rand() % 20), 180, mkcolor(255, 0, 0),
                     x >> 4, y >> 4, angle);
#else
        i = (rand() % 128) + 128;

        draw_segment(0, 0, mkcolor(255, i, i),
                     (rand() % 20), 180, mkcolor(255, i, i),
                     x >> 4, y >> 4, angle);
#endif
      }
    }


    /* Draw bullets: */

    for (i = 0; i < NUM_BULLETS; i++)
    {
      if (bullets[i].timer >= 0)
      {
        draw_line(bullets[i].x - (rand() % 3) - bullets[i].xm * 2,
                  bullets[i].y - (rand() % 3) - bullets[i].ym * 2,
                  mkcolor((rand() % 3) * 128,
                          (rand() % 3) * 128,
                          (rand() % 3) * 128),
                  bullets[i].x + (rand() % 3) - bullets[i].xm * 2,
                  bullets[i].y + (rand() % 3) - bullets[i].ym * 2,
                  mkcolor((rand() % 3) * 128,
                          (rand() % 3) * 128, (rand() % 3) * 128));

        draw_line(bullets[i].x + (rand() % 3) - bullets[i].xm * 2,
                  bullets[i].y - (rand() % 3) - bullets[i].ym * 2,
                  mkcolor((rand() % 3) * 128,
                          (rand() % 3) * 128,
                          (rand() % 3) * 128),
                  bullets[i].x - (rand() % 3) - bullets[i].xm * 2,
                  bullets[i].y + (rand() % 3) - bullets[i].ym * 2,
                  mkcolor((rand() % 3) * 128,
                          (rand() % 3) * 128, (rand() % 3) * 128));



        draw_thick_line(bullets[i].x - (rand() % 5),
                        bullets[i].y - (rand() % 5),
                        mkcolor((rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64),
                        bullets[i].x + (rand() % 5),
                        bullets[i].y + (rand() % 5),
                        mkcolor((rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64));

        draw_thick_line(bullets[i].x + (rand() % 5),
                        bullets[i].y - (rand() % 5),
                        mkcolor((rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64),
                        bullets[i].x - (rand() % 5),
                        bullets[i].y + (rand() % 5),
                        mkcolor((rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64,
                                (rand() % 3) * 128 + 64));
      }
    }



    /* Zooming level effect: */

    if (text_zoom > 0)
    {
      if ((counter % 2) == 0)
        text_zoom--;

#ifndef EMBEDDED
      draw_text(zoom_str, (WIDTH - (strlen(zoom_str) * text_zoom)) / 2,
                (HEIGHT - text_zoom) / 2,
                text_zoom, mkcolor(text_zoom * (256 / ZOOM_START), 0, 0));
#else
      draw_text(zoom_str, (WIDTH - (strlen(zoom_str) * text_zoom)) / 2,
                (HEIGHT - text_zoom) / 2,
                text_zoom, mkcolor(text_zoom * (256 / ZOOM_START), 128, 128));
#endif
    }


    /* Game over? */

    if (player_alive == 0 && lives == 0)
    {
      if (player_die_timer > 14)
      {
        draw_text("GAME OVER",
                  (WIDTH - 9 * player_die_timer) / 2,
                  (HEIGHT - player_die_timer) / 2,
                  player_die_timer,
                  mkcolor(rand() % 255, rand() % 255, rand() % 255));
      }
      else
      {
        draw_text("GAME OVER",
                  (WIDTH - 9 * 14) / 2,
                  (HEIGHT - 14) / 2, 14, mkcolor(255, 255, 255));

      }
    }


    /* Go to next level? */

    if (num_asteroids_alive == 0)
    {
      level++;

      reset_level();
    }


    /* Flush and pause! */

    /* SDL_Flip(screen); *//* SDL1.2 method */
    SDL_UpdateTexture(screenTexture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bkgdTexture, NULL, NULL);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    now_time = SDL_GetTicks();

    if (now_time < last_time + (1000 / FPS))
    {
      SDL_Delay(last_time + 1000 / FPS - now_time);
    }
  }
  while (!done);


  /* Record, if a high score: */

  if (score >= high)
  {
    high = score;
  }


  /* Display mouse cursor: */

  if (fullscreen)
    SDL_ShowCursor(1);


  return (quit);
}


void finish(void)
{
  SDL_Quit();
}


void setup(int argc, char *argv[])
{
  int i;
  SDL_Surface *tmp;


  /* Options: */

  score = 0;
  use_sound = TRUE;
  fullscreen = FALSE;


  /* Check command-line options: */

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--fullscreen") == 0 || strcmp(argv[i], "-f") == 0)
    {
      fullscreen = TRUE;
    }
    else if (strcmp(argv[i], "--nosound") == 0 || strcmp(argv[i], "-q") == 0)
    {
      use_sound = FALSE;
    }
    else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
    {
      show_version();

      printf("\n"
             "Programming: Bill Kendrick, New Breed Software - bill@newbreedsoftware.com\n"
             "Music:       Mike Faltiss (Hadji/Digital Music Kings) - deadchannel@hotmail.com\n"
             "\n"
             "Keyboard controls:\n"
             "  Left/Right - Rotate ship\n"
             "  Up         - Thrust engines\n"
             "  Space      - Fire weapons\n"
             "  Shift      - Respawn after death (or wait)\n"
             "  Escape     - Return to title screen\n");
      printf("\n"
             "Joystick controls:\n"
             "  Left/Right - Rotate ship\n"
             "  Fire-A     - Thrust engines\n"
             "  Fire-B     - Fire weapons\n"
             "\n"
             "Run with \"--usage\" for command-line options...\n"
             "Run with \"--copying\" for copying information...\n" "\n");

      exit(0);
    }
    else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
    {
      show_version();
      printf("State format file version " STATE_FORMAT_VERSION "\n");
      exit(0);
    }
    else if (strcmp(argv[i], "--copying") == 0 || strcmp(argv[i], "-c") == 0)
    {
      show_version();
      printf("\n"
             "This program is free software; you can redistribute it\n"
             "and/or modify it under the terms of the GNU General Public\n"
             "License as published by the Free Software Foundation;\n"
             "either version 2 of the License, or (at your option) any\n"
             "later version.\n"
             "\n"
             "This program is distributed in the hope that it will be\n"
             "useful and entertaining, but WITHOUT ANY WARRANTY; without\n"
             "even the implied warranty of MERCHANTABILITY or FITNESS\n"
             "FOR A PARTICULAR PURPOSE.  See the GNU General Public\n"
             "License for more details.\n" "\n");
      printf("You should have received a copy of the GNU General Public\n"
             "License along with this program; if not, write to the Free\n"
             "Software Foundation, Inc., 59 Temple Place, Suite 330,\n"
             "Boston, MA  02111-1307  USA\n" "\n");
      exit(0);
    }
    else if (strcmp(argv[i], "--usage") == 0 || strcmp(argv[i], "-u") == 0)
    {
      show_usage(stdout, argv[0]);
      exit(0);
    }
    else
    {
      show_usage(stderr, argv[0]);
      exit(1);
    }
  }


  /* Seed random number generator: */

  srand(SDL_GetTicks());


  /* Init SDL video: */

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr,
            "\nError: I could not initialize video!\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());
    exit(1);
  }


  /* Init joysticks: */

#ifdef JOY_YES
  use_joystick = 1;

  if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
  {
    fprintf(stderr,
            "\nWarning: I could not initialize joystick.\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());

    use_joystick = 0;
  }
  else
  {
    /* Look for joysticks: */

    if (SDL_NumJoysticks() <= 0)
    {
      fprintf(stderr, "\nWarning: No joysticks available.\n");

      use_joystick = 0;
    }
    else
    {
      /* Open joystick: */

      js = SDL_JoystickOpen(0);

      if (js == NULL)
      {
        fprintf(stderr,
                "\nWarning: Could not open joystick 1.\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", SDL_GetError());

        use_joystick = 0;
      }
      else
      {
        /* Check for proper stick configuration: */

        if (SDL_JoystickNumAxes(js) < 2)
        {
          fprintf(stderr, "\nWarning: Joystick doesn't have enough axes!\n");

          use_joystick = 0;
        }
        else
        {
          if (SDL_JoystickNumButtons(js) < 2)
          {
            fprintf(stderr,
                    "\nWarning: Joystick doesn't have enough " "buttons!\n");

            use_joystick = 0;
          }
        }
      }
    }
  }
#else
  use_joystick = 0;
#endif


  /* Open window: */

  if (fullscreen)
  {
    set_vid_mode(SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (screen == NULL)
    {
      fprintf(stderr,
              "\nWarning: I could not set up fullscreen video for "
              "%dx%d mode.\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", WIDTH, HEIGHT, SDL_GetError());
      fullscreen = 0;
    }
  }

  if (!fullscreen)
  {
    set_vid_mode(0);

    if (screen == NULL)
    {
      fprintf(stderr,
              "\nError: I could not open the display.\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", SDL_GetError());
      exit(1);
    }
  }


  /* Load background image: */

#ifndef EMBEDDED
  tmp = IMG_Load(DATA_PREFIX "images/redspot.jpg");

  if (tmp == NULL)
  {
    fprintf(stderr,
            "\nError: I could not open the background image:\n"
            DATA_PREFIX "images/redspot.jpg\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());
    exit(1);
  }

  bkgd = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGB888, 0);
  if (bkgd == NULL)
  {
    fprintf(stderr,
            "\nError: I couldn't convert the background image"
            "to the display format!\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());
    exit(1);
  }
  bkgdTexture = SDL_CreateTextureFromSurface(renderer, bkgd);

  SDL_FreeSurface(tmp);

#else

  tmp = SDL_LoadBMP(DATA_PREFIX "images/redspot-e.bmp");

  if (tmp == NULL)
  {
    fprintf(stderr,
            "\nError: I could not open the background image:\n"
            DATA_PREFIX "images/redspot-e.bmp\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());
    exit(1);
  }

  bkgd = SDL_DisplayFormat(tmp);
  if (bkgd == NULL)
  {
    fprintf(stderr,
            "\nError: I couldn't convert the background image"
            "to the display format!\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", SDL_GetError());
    exit(1);
  }

  SDL_FreeSurface(tmp);
#endif


#ifndef NOSOUND
  /* Init sound: */

  if (use_sound)
  {
    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 512) < 0)
    {
      fprintf(stderr,
              "\nWarning: I could not set up audio for 22050 Hz "
              "16-bit stereo.\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", SDL_GetError());
      use_sound = FALSE;
    }
  }


  /* Load sound files: */

  if (use_sound)
  {
    for (i = 0; i < NUM_SOUNDS; i++)
    {
      sounds[i] = Mix_LoadWAV(sound_names[i]);
      if (sounds[i] == NULL)
      {
        fprintf(stderr,
                "\nError: I could not load the sound file:\n"
                "%s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", sound_names[i], SDL_GetError());
        exit(1);
      }
    }


    game_music = Mix_LoadMUS(mus_game_name);
    if (game_music == NULL)
    {
      fprintf(stderr,
              "\nError: I could not load the music file:\n"
              "%s\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", mus_game_name, SDL_GetError());
      exit(1);
    }
  }
#endif


  seticon();
}


/* Set the window's icon: */

void seticon(void)
{
#ifndef EMBEDDED
  SDL_Surface *icon;


  /* Load icon into a surface: */

  icon = IMG_Load(DATA_PREFIX "images/icon.png");
  if (icon == NULL)
  {
    fprintf(stderr,
            "\nError: I could not load the icon image: %s\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n", DATA_PREFIX "images/icon.png", SDL_GetError());
    exit(1);
  }

  /* Set icon: */

  SDL_SetWindowIcon(window, icon);


  /* Free icon surface: */

  SDL_FreeSurface(icon);
#endif
}


/* Fast approximate-integer, table-based cosine! Whee! */

int fast_cos(int angle)
{
  angle = (angle % 45);

  if (angle < 12)
    return (trig[angle]);
  else if (angle < 23)
    return (-trig[10 - (angle - 12)]);
  else if (angle < 34)
    return (-trig[angle - 22]);
  else
    return (trig[45 - angle]);
}


/* Sine based on fast cosine... */

int fast_sin(int angle)
{
  return (-fast_cos((angle + 11) % 45));
}


/* Draw a line: */

void draw_line(int x1, int y1, color_type c1, int x2, int y2, color_type c2)
{
  sdl_drawline(x1, y1, c1, x2, y2, c2);

  if (x1 < 0 || x2 < 0)
  {
    sdl_drawline(x1 + WIDTH, y1, c1, x2 + WIDTH, y2, c2);
  }
  else if (x1 >= WIDTH || x2 >= WIDTH)
  {
    sdl_drawline(x1 - WIDTH, y1, c1, x2 - WIDTH, y2, c2);
  }

  if (y1 < 0 || y2 < 0)
  {
    sdl_drawline(x1, y1 + HEIGHT, c1, x2, y2 + HEIGHT, c2);
  }
  else if (y1 >= HEIGHT || y2 >= HEIGHT)
  {
    sdl_drawline(x1, y1 - HEIGHT, c1, x2, y2 - HEIGHT, c2);
  }
}


/* Create a color_type struct out of RGB values: */

color_type mkcolor(int r, int g, int b)
{
  color_type c;

  if (r > 255)
    r = 255;
  if (g > 255)
    g = 255;
  if (b > 255)
    b = 255;

  c.r = (Uint8) r;
  c.g = (Uint8) g;
  c.b = (Uint8) b;

  return c;
}


/* Draw a line on an SDL surface: */

void sdl_drawline(int x1, int y1, color_type c1,
                  int x2, int y2, color_type c2)
{
  int dx, dy;
#ifndef EMBEDDED
  float cr, cg, cb, rd, gd, bd;
#endif
  float m, b;


  if (clip(&x1, &y1, &x2, &y2))
  {
    dx = x2 - x1;
    dy = y2 - y1;

    if (dx != 0)
    {
      m = ((float) dy) / ((float) dx);
      b = y1 - m * x1;

      if (x2 >= x1)
        dx = 1;
      else
        dx = -1;

#ifndef EMBEDDED
      cr = c1.r;
      cg = c1.g;
      cb = c1.b;

      rd = (float) (c2.r - c1.r) / (float) (x2 - x1) * dx;
      gd = (float) (c2.g - c1.g) / (float) (x2 - x1) * dx;
      bd = (float) (c2.b - c1.b) / (float) (x2 - x1) * dx;
#endif

      while (x1 != x2)
      {
        y1 = m * x1 + b;
        y2 = m * (x1 + dx) + b;

#ifndef EMBEDDED
        drawvertline(x1, y1, mkcolor(cr, cg, cb),
                     y2, mkcolor(cr + rd, cg + gd, cb + bd));
#else
        drawvertline(x1, y1, mkcolor(c1.r, c1.g, c1.b),
                     y2, mkcolor(c1.r, c1.g, c1.b));
#endif

        x1 = x1 + dx;


#ifndef EMBEDDED
        cr = cr + rd;
        cg = cg + gd;
        cb = cb + bd;
#endif
      }
    }
    else
      drawvertline(x1, y1, c1, y2, c2);
  }
}


/* Clip lines to window: */

int clip(int *x1, int *y1, int *x2, int *y2)
{
#ifndef EMBEDDED

  float fx1, fx2, fy1, fy2, tmp;
  float m;
  unsigned char code1, code2;
  int done, draw, swapped;
  unsigned char ctmp;
  fx1 = (float) *x1;
  fy1 = (float) *y1;
  fx2 = (float) *x2;
  fy2 = (float) *y2;


  done = FALSE;
  draw = FALSE;
  m = 0;
  swapped = FALSE;


  while (!done)
  {
    code1 = encode(fx1, fy1);
    code2 = encode(fx2, fy2);

    if (!(code1 | code2))
    {
      done = TRUE;
      draw = TRUE;
    }
    else if (code1 & code2)
    {
      done = TRUE;
    }
    else
    {
      if (!code1)
      {
        swapped = TRUE;
        tmp = fx1;
        fx1 = fx2;
        fx2 = tmp;

        tmp = fy1;
        fy1 = fy2;
        fy2 = tmp;

        ctmp = code1;
        code1 = code2;
        code2 = ctmp;
      }


      if (fx2 != fx1)
        m = (fy2 - fy1) / (fx2 - fx1);
      else
        m = 1;

      if (code1 & LEFT_EDGE)
      {
        fy1 += ((0 - (fx1)) * m);
        fx1 = 0;
      }
      else if (code1 & RIGHT_EDGE)
      {
        fy1 += (((WIDTH - 1) - (fx1)) * m);
        fx1 = (WIDTH - 1);
      }
      else if (code1 & TOP_EDGE)
      {
        if (fx2 != fx1)
          fx1 += ((0 - (fy1)) / m);
        fy1 = 0;
      }
      else if (code1 & BOTTOM_EDGE)
      {
        if (fx2 != fx1)
          fx1 += (((HEIGHT - 1) - (fy1)) / m);
        fy1 = (HEIGHT - 1);
      }
    }
  }


  if (swapped)
  {
    tmp = fx1;
    fx1 = fx2;
    fx2 = tmp;

    tmp = fy1;
    fy1 = fy2;
    fy2 = tmp;
  }


  *x1 = (int) fx1;
  *y1 = (int) fy1;
  *x2 = (int) fx2;
  *y2 = (int) fy2;

  return (draw);
#else

  if (*x1 < 0 || *x1 >= WIDTH ||
      *y1 < 0 || *y1 >= HEIGHT ||
      *x2 < 0 || *x2 >= WIDTH || *y2 < 0 || *y2 >= HEIGHT)
    return FALSE;
  else
    return TRUE;


#endif
}


/* Where does this line clip? */

unsigned char encode(float x, float y)
{
  unsigned char code;

  code = 0x00;

  if (x < 0.0)
    code = code | LEFT_EDGE;
  else if (x >= (float) WIDTH)
    code = code | RIGHT_EDGE;

  if (y < 0.0)
    code = code | TOP_EDGE;
  else if (y >= (float) HEIGHT)
    code = code | BOTTOM_EDGE;

  return code;
}


/* Draw a verticle line: */

void drawvertline(int x, int y1, color_type c1, int y2, color_type c2)
{
  int tmp, dy;
#ifndef EMBEDDED
  float cr, cg, cb, rd, gd, bd;
#else
  int cr, cg, cb;
#endif

  if (y1 > y2)
  {
    tmp = y1;
    y1 = y2;
    y2 = tmp;

#ifndef EMBEDDED
    tmp = c1.r;
    c1.r = c2.r;
    c2.r = tmp;

    tmp = c1.g;
    c1.g = c2.g;
    c2.g = tmp;

    tmp = c1.b;
    c1.b = c2.b;
    c2.b = tmp;
#endif
  }

  cr = c1.r;
  cg = c1.g;
  cb = c1.b;

#ifndef EMBEDDED
  if (y1 != y2)
  {
    rd = (float) (c2.r - c1.r) / (float) (y2 - y1);
    gd = (float) (c2.g - c1.g) / (float) (y2 - y1);
    bd = (float) (c2.b - c1.b) / (float) (y2 - y1);
  }
  else
  {
    rd = 0;
    gd = 0;
    bd = 0;
  }
#endif

  for (dy = y1; dy <= y2; dy++)
  {
    if (drawn_at[dy + 1][x + 1] == 0)
      putpixel(screen, x + 1, dy + 1, SDL_MapRGB(screen->format, 0, 0, 0));

    putpixel(screen, x, dy, SDL_MapRGB(screen->format,
                                       (Uint8) cr, (Uint8) cg, (Uint8) cb));
    drawn_at[dy][x] = 1;

#ifndef EMBEDDED
    cr = cr + rd;
    cg = cg + gd;
    cb = cb + bd;
#endif
  }
}


/* Draw a single pixel into the surface: */

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp;
  Uint8 *p;


  /* Assuming the X/Y values are within the bounds of this surface... */

  if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
  {
    /* Determine bytes-per-pixel for the surface in question: */

    bpp = surface->format->BytesPerPixel;


    /* Set a pointer to the exact location in memory of the pixel
       in question: */

    p = (((Uint8 *) surface->pixels) +  /* Start at beginning of RAM */
         (y * surface->pitch) + /* Go down Y lines */
         (x * bpp));            /* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
       to the pixel value sent in: */

    if (bpp == 1)
      *p = pixel;
    else if (bpp == 2)
      *(Uint16 *) p = pixel;
    else if (bpp == 3)
    {
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      }
      else
      {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
    }
    else if (bpp == 4)
    {
      *(Uint32 *) p = pixel;
    }
  }
}



/* Draw a line segment, rotated around a center point: */

void draw_segment(int r1, int a1,
                  color_type c1,
                  int r2, int a2, color_type c2, int cx, int cy, int a)
{
  draw_line(((fast_cos((a1 + a) >> 3) * r1) >> 10) + cx,
            cy - ((fast_sin((a1 + a) >> 3) * r1) >> 10),
            c1,
            ((fast_cos((a2 + a) >> 3) * r2) >> 10) + cx,
            cy - ((fast_sin((a2 + a) >> 3) * r2) >> 10), c2);
}


/* Add a bullet: */

int add_bullet(int x, int y, int a, int xm, int ym)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BULLETS && found == -1; i++)
  {
    if (bullets[i].timer <= 0)
      found = i;
  }

  if (found != -1)
  {
#ifndef EMBEDDED
    bullets[found].timer = 50;
#else
    bullets[found].timer = 30;
#endif

    bullets[found].x = x;
    bullets[found].y = y;

    bullets[found].xm = ((fast_cos(a >> 3) * 5) >> 10) + (xm >> 4);
    bullets[found].ym = -((fast_sin(a >> 3) * 5) >> 10) + (ym >> 4);


    playsound(SND_BULLET);
  }

  return (found != -1);
}


/* Add an asteroid: */

void add_asteroid(int x, int y, int xm, int ym, int size)
{
  int i, found;


  /* Find a slot: */

  found = -1;

  for (i = 0; i < NUM_ASTEROIDS && found == -1; i++)
  {
    if (asteroids[i].alive == 0)
      found = i;
  }


  /* Hack: No asteroids should be stationary! */

  while (xm == 0)
  {
    xm = (rand() % 3) - 1;
  }


  if (found != -1)
  {
    asteroids[found].alive = 1;

    asteroids[found].x = x;
    asteroids[found].y = y;
    asteroids[found].xm = xm;
    asteroids[found].ym = ym;

    asteroids[found].angle = (rand() % 360);
    asteroids[found].angle_m = (rand() % 6) - 3;

    asteroids[found].size = size;

    for (i = 0; i < AST_SIDES; i++)
    {
      asteroids[found].shape[i].radius = (rand() % 3);
      asteroids[found].shape[i].angle = i * 60 + (rand() % 40);
    }
  }
}


/* Add a bit: */

void add_bit(int x, int y, int xm, int ym)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BITS && found == -1; i++)
  {
    if (bits[i].timer <= 0)
      found = i;
  }


  if (found != -1)
  {
    bits[found].timer = 16;

    bits[found].x = x;
    bits[found].y = y;
    bits[found].xm = xm;
    bits[found].ym = ym;
  }
}


/* Draw an asteroid: */

void draw_asteroid(int size, int x, int y, int angle, shape_type *shape)
{
  int i, b1, b2;
  int div;

#ifndef EMBEDDED
  div = 240;
#else
  div = 120;
#endif

  for (i = 0; i < AST_SIDES - 1; i++)
  {
    b1 = (((shape[i].angle + angle) % 180) * 255) / div;
    b2 = (((shape[i + 1].angle + angle) % 180) * 255) / div;

    draw_segment((size * (AST_RADIUS - shape[i].radius)),
                 shape[i].angle, mkcolor(b1, b1, b1),
                 (size * (AST_RADIUS - shape[i + 1].radius)),
                 shape[i + 1].angle, mkcolor(b2, b2, b2), x, y, angle);
  }

  b1 = (((shape[AST_SIDES - 1].angle + angle) % 180) * 255) / div;
  b2 = (((shape[0].angle + angle) % 180) * 255) / div;

  draw_segment((size * (AST_RADIUS - shape[AST_SIDES - 1].radius)),
               shape[AST_SIDES - 1].angle, mkcolor(b1, b1, b1),
               (size * (AST_RADIUS - shape[0].radius)),
               shape[0].angle, mkcolor(b2, b2, b2), x, y, angle);
}


/* Queue a sound! */

void playsound(int snd)
{
  int which;
#ifndef EMBEDDED
  int i;
#endif

#ifndef NOSOUND
  if (use_sound)
  {
#ifdef EMBEDDED
    which = -1;
#else
    which = (rand() % 3) + CHAN_THRUST;
    for (i = CHAN_THRUST; i < 4; i++)
    {
      if (!Mix_Playing(i))
        which = i;
    }
#endif

    Mix_PlayChannel(which, sounds[snd], 0);
  }
#endif
}


/* Break an asteroid and add an explosion: */

void hurt_asteroid(int j, int xm, int ym, int exp_size)
{
  int k;

  add_score(100 / (asteroids[j].size + 1));

  if (asteroids[j].size > 1)
  {
    /* Break the rock into two smaller ones! */

    add_asteroid(asteroids[j].x,
                 asteroids[j].y,
                 ((asteroids[j].xm + xm) / 2),
                 (asteroids[j].ym + ym), asteroids[j].size - 1);

    add_asteroid(asteroids[j].x,
                 asteroids[j].y,
                 (asteroids[j].xm + xm),
                 ((asteroids[j].ym + ym) / 2), asteroids[j].size - 1);
  }


  /* Make the original go away: */

  asteroids[j].alive = 0;


  /* Add explosion: */

  playsound(SND_AST1 + (asteroids[j].size) - 1);

  for (k = 0; k < exp_size; k++)
  {
    add_bit((asteroids[j].x -
             (asteroids[j].size * AST_RADIUS) +
             (rand() % (AST_RADIUS * 2))),
            (asteroids[j].y -
             (asteroids[j].size * AST_RADIUS) +
             (rand() % (AST_RADIUS * 2))),
            ((rand() % (asteroids[j].size * 3)) -
             (asteroids[j].size) +
             ((xm + asteroids[j].xm) / 3)),
            ((rand() % (asteroids[j].size * 3)) -
             (asteroids[j].size) + ((ym + asteroids[j].ym) / 3)));
  }
}


/* Increment score: */

void add_score(int amount)
{
  /* See if they deserve a new life: */

  if (score / ONEUP_SCORE < (score + amount) / ONEUP_SCORE)
  {
    lives++;
    strcpy(zoom_str, "EXTRA LIFE");
    text_zoom = ZOOM_START;
    playsound(SND_EXTRALIFE);
  }



  /* Add to score: */

  score = score + amount;
}


/* Draw a character: */

void draw_char(char c, int x, int y, int r, color_type cl)
{
  int i, v;

  /* Which vector is this character? */

  v = -1;
  if (c >= '0' && c <= '9')
    v = (c - '0');
  else if (c >= 'A' && c <= 'Z')
    v = (c - 'A') + 10;
  else if (c == '.')
    v = 36;


  if (v != -1)
  {
    for (i = 0; i < 5; i++)
    {
      if (char_vectors[v][i][0] != -1)
      {
        draw_line(x + (char_vectors[v][i][0] * r),
                  y + (char_vectors[v][i][1] * r),
                  cl,
                  x + (char_vectors[v][i][2] * r),
                  y + (char_vectors[v][i][3] * r), cl);
      }
    }
  }
}

#define CHAR_SPACING 4

void draw_text(char *str, int x, int y, int s, color_type c)
{
  int i;

  for (i = 0; i < strlen(str); i++)
    draw_char(str[i], i * (s + CHAR_SPACING) + x, y, s, c);
}


void draw_thick_line(int x1, int y1, color_type c1,
                     int x2, int y2, color_type c2)
{
  draw_line(x1, y1, c1, x2, y2, c2);
  draw_line(x1 + 1, y1, c1, x2 + 1, y2, c2);
  draw_line(x1 + 1, y1 + 1, c1, x2 + 1, y2 + 1, c2);
}


void reset_level(void)
{
  int i;


  for (i = 0; i < NUM_BULLETS; i++)
    bullets[i].timer = 0;

  for (i = 0; i < NUM_ASTEROIDS; i++)
    asteroids[i].alive = 0;

  for (i = 0; i < NUM_BITS; i++)
    bits[i].timer = 0;

  for (i = 0; i < (level + 1) && i < 10; i++)
  {
#ifndef EMBEDDED
    add_asteroid( /* x */ (rand() % 40) + ((WIDTH - 40) * (rand() % 2)),
                 /* y */ (rand() % HEIGHT),
                 /* xm */ (rand() % 9) - 4,
                 /* ym */ ((rand() % 9) - 4) * 4,
                 /* size */ (rand() % 3) + 2);
#else
    add_asteroid( /* x */ (rand() % WIDTH),
                 /* y */ (rand() % 40) + ((HEIGHT - 40) * (rand() % 2)),
                 /* xm */ ((rand() % 9) - 4) * 4,
                 /* ym */ (rand() % 9) - 4,
                 /* size */ (rand() % 3) + 2);
#endif
  }


  snprintf(zoom_str, sizeof(zoom_str), "LEVEL %d", level);

  text_zoom = ZOOM_START;
}


/* Show program version: */

void show_version(void)
{
  printf("Vectoroids - Version " VER_VERSION " (" VER_DATE ")\n");
}


/* Show usage display: */

void show_usage(FILE *f, char *prg)
{
  fprintf(f, "Usage: %s {--help | --usage | --version | --copying }\n"
          "       %s [--fullscreen] [--nosound]\n\n", prg, prg);
}


/* Set video mode & window caption: */

void set_vid_mode(unsigned flags)
{
  char str[64];

  /* Prefer 16bpp, but also prefer native modes to emulated 16bpp. */

  snprintf(str, sizeof(str), "Vectorids %s", VER_VERSION);
  window = SDL_CreateWindow(str,
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, flags);
  renderer = SDL_CreateRenderer(window, -1, 0);
  screen = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32,
                                0x00FF0000,
                                0x0000FF00, 0x000000FF, 0xFF000000);
  screenTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    WIDTH, HEIGHT);
}


/* Draw text, centered horizontally: */

void draw_centered_text(char *str, int y, int s, color_type c)
{
  draw_text(str, (WIDTH - strlen(str) * (s + CHAR_SPACING)) / 2, y, s, c);
}
