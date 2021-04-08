//Tyler Swearingen's NES game

#include <stdlib.h>
#include <string.h>

// include NESLIB header
#include "neslib.h"

// include CC65 NES Header (PPU)
#include <nes.h>

// include VRAMBUF Header
#include "vrambuf.h"

// link the pattern table into CHR ROM

//#link "sprites1.s"
//#link "vrambuf.c"


/////////////////// METASPRITES ///////////////////////

// define a 2x2 metasprite
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        0,      8,      (code)+1,   pal, \
        8,      0,      (code)+2,   pal, \
        8,      8,      (code)+3,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_2x2_FLIP_H(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};

// define a 2x2 metasprite, flipped vertically
#define DEF_METASPRITE_2x2_FLIP_V(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+3,   (pal)|OAM_FLIP_V, \
        8,      8,      (code)+2,   (pal)|OAM_FLIP_V, \
        0,      0,      (code)+1,   (pal)|OAM_FLIP_V, \
        0,      8,      (code)+0,   (pal)|OAM_FLIP_V, \
        128};

// define a 2x2 metasprite, flipped vertically and Horizontally
#define DEF_METASPRITE_2x2_FLIP_HV(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+1,   (pal)|OAM_FLIP_V|OAM_FLIP_H, \
        8,      8,      (code)+0,   (pal)|OAM_FLIP_V|OAM_FLIP_H, \
        0,      0,      (code)+3,   (pal)|OAM_FLIP_V|OAM_FLIP_H, \
        0,      8,      (code)+2,   (pal)|OAM_FLIP_V|OAM_FLIP_H, \
        128};

//Player 1
DEF_METASPRITE_2x2_FLIP_V(player1Idle, 0x4, 0);
DEF_METASPRITE_2x2(player1RRun1, 0xdc, 0);
DEF_METASPRITE_2x2(player1RRun2, 0xe0, 0);
DEF_METASPRITE_2x2(player1RRun3, 0xe4, 0);
DEF_METASPRITE_2x2(player1RJump, 0xe8, 0);
DEF_METASPRITE_2x2(player1RClimb, 0xec, 0);
DEF_METASPRITE_2x2(player1RSad, 0xf0, 0);

DEF_METASPRITE_2x2_FLIP_H(player1LStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP_H(player1LRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP_H(player1LRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP_H(player1LRun3, 0xe4, 0);
DEF_METASPRITE_2x2_FLIP_H(player1LJump, 0xe8, 0);
DEF_METASPRITE_2x2_FLIP_H(player1LClimb, 0xec, 0);
DEF_METASPRITE_2x2_FLIP_H(player1LSad, 0xf0, 0);

//Player 2
DEF_METASPRITE_2x2(player2Idle, 0x8, 1);
DEF_METASPRITE_2x2(player2RRun1, 0xdc, 3);
DEF_METASPRITE_2x2(player2RRun2, 0xe0, 3);
DEF_METASPRITE_2x2(player2RRun3, 0xe4, 3);
DEF_METASPRITE_2x2(player2RJump, 0xe8, 3);
DEF_METASPRITE_2x2(player2RClimb, 0xec, 3);
DEF_METASPRITE_2x2(player2RSad, 0xf0, 3);

DEF_METASPRITE_2x2_FLIP_H(player2LStand, 0x3, 0);
DEF_METASPRITE_2x2_FLIP_H(player2LRun1, 0xdc, 3);
DEF_METASPRITE_2x2_FLIP_H(player2LRun2, 0xe0, 3);
DEF_METASPRITE_2x2_FLIP_H(player2LRun3, 0xe4, 3);
DEF_METASPRITE_2x2_FLIP_H(player2LJump, 0xe8, 3);
DEF_METASPRITE_2x2_FLIP_H(player2LClimb, 0xec, 3);
DEF_METASPRITE_2x2_FLIP_H(player2LSad, 0xf0, 3);

const unsigned char* const playerRunSeq[2][19] = 
{
  {
    player1Idle,
    player1LRun1, player1LRun2, player1LRun3, 
    player1LRun1, player1LRun2, player1LRun3, 
    player1LRun1, player1LRun2, player1LRun3,
    player1RRun1, player1RRun2, player1RRun3, 
    player1RRun1, player1RRun2, player1RRun3, 
    player1RRun1, player1RRun2, player1RRun3,
  },
  {
    player2Idle,
    player2LRun1, player2LRun2, player2LRun3, 
    player2LRun1, player2LRun2, player2LRun3, 
    player2LRun1, player2LRun2, player2LRun3,
    player2RRun1, player2RRun2, player2RRun3, 
    player2RRun1, player2RRun2, player2RRun3, 
    player2RRun1, player2RRun2, player2RRun3
  }
};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x15,0x30,0x27,0x00,	// background palette 0
  0x1C,0x20,0x2C,0x00,	// background palette 1
  0x00,0x10,0x20,0x00,	// background palette 2
  0x06,0x16,0x26,0x00,	// background palette 3

  0x16,0x13,0x20,0x00,	// sprite palette 0
  0x16,0x04,0x3C,0x00,	// sprite palette 1
  0x0D,0x2D,0x3A,0x00,	// sprite palette 2
  0x0C,0x27,0x2A	// sprite palette 3
};

// number of actors (4 h/w sprites each)
#define NUM_ACTORS 2
#define NUM_STARS 8

// vertical mirroring for scroll
#define NES_MIRRORING 0

// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];
// actors direction (0 = left, 1 = right)
char actor_run_dir[NUM_ACTORS];

byte actor_run_animation[NUM_ACTORS];
byte actor_stand_sprite[NUM_ACTORS];

// setup PPU and tables
void setup_graphics() 
{
  // set palette colors
  pal_all(PALETTE);
  
  vrambuf_clear();
  set_vram_update(updbuf);
  
  // turn on PPU
  ppu_on_all();
}

void move_player()
{
  char i;
  byte pad;
  for (i=0; i<2; i++) 
  {
    // poll controller i (0-1)
    pad = pad_poll(i);
    
    // move actor[i] left/right
    if (pad & PAD_LEFT && actor_x[i] > 6) 
    {
      actor_dx[i]=-2;
      actor_run_dir[i] = 0;
    }
    else if (pad & PAD_RIGHT && actor_x[i] < 234) 
    {
      actor_dx[i]=2;
      actor_run_dir[i] = 1;
    }
    else 
      actor_dx[i]=0;
    
    // ACTIONS //
    
    if(pad == 0) //Player 1 Controls
    {
      if (pad & PAD_A)
      {
        
      }
    }
    else // Player 2 Controls
    {
      if (pad & PAD_A)
      {
        
      }
    }
  }
}

void move_stars()
{
  char i;
  for(i = 0; i<NUM_STARS; i++)
  {
    
  }
}
  
// main program
void main() 
{
  char oam_id;	// sprite ID	
  char i;
  
  // setup graphics
  setup_graphics();
  
  // initialize actors with random values
  for (i=0; i<NUM_ACTORS; i++) 
  {
    actor_x[i] = 240/2;
    if(i == 1)
    {
      actor_y[i] = 200;
    }
    else
    {
      actor_y[i] = 14;
    }
    actor_dx[i] = 0;
    actor_dy[i] = 0;
    actor_run_dir[i] = 0;
  }
  
  // loop forever
  while (1) 
  {
    // start with OAMid/sprite 0
    oam_id = 0;
    // set player 0/1 velocity based on controller
    vram_adr(NTADR_A(2, 2));
    vram_fill(0x1, 10);
    move_player();
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 8;
      runseq += 1;
      if (actor_dx[i] > 0)
      {
        runseq += 8;
        oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[i][runseq]);
      	actor_x[i] += actor_dx[i];
      }
      else if(actor_dx[i] < 0)
      {
        oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[i][runseq]);
      	actor_x[i] += actor_dx[i];
      }
      else
      {
        if(actor_run_dir[i] == 1)
          oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[i][1]);
        else
          oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[i][0]);
      }
      
    }
    
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    
    // wait for next frame
    vrambuf_flush();
  }
}
