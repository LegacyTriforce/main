
//Tyler Swearingen's NES game

#include <stdlib.h>
#include <string.h>

// include NESLIB header
#include "neslib.h"

// include CC65 NES Header (PPU)
#include <nes.h>

// link the pattern table into CHR ROM
//#link "chr_generic.s"

///// METASPRITES

// define a 2x2 metasprite
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        0,      8,      (code)+1,   pal, \
        8,      0,      (code)+2,   pal, \
        8,      8,      (code)+3,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};

#define NES_MIRRORING 0

//Player 1
DEF_METASPRITE_2x2(player1RStand, 0xd8, 0);
DEF_METASPRITE_2x2(player1RRun1, 0xdc, 0);
DEF_METASPRITE_2x2(player1RRun2, 0xe0, 0);
DEF_METASPRITE_2x2(player1RRun3, 0xe4, 0);
DEF_METASPRITE_2x2(player1RJump, 0xe8, 0);
DEF_METASPRITE_2x2(player1RClimb, 0xec, 0);
DEF_METASPRITE_2x2(player1RSad, 0xf0, 0);

DEF_METASPRITE_2x2_FLIP(player1LStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(player1LRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(player1LRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(player1LRun3, 0xe4, 0);
DEF_METASPRITE_2x2_FLIP(player1LJump, 0xe8, 0);
DEF_METASPRITE_2x2_FLIP(player1LClimb, 0xec, 0);
DEF_METASPRITE_2x2_FLIP(player1LSad, 0xf0, 0);

//Player 2
DEF_METASPRITE_2x2(player2RStand, 0xd8, 3);
DEF_METASPRITE_2x2(player2RRun1, 0xdc, 3);
DEF_METASPRITE_2x2(player2RRun2, 0xe0, 3);
DEF_METASPRITE_2x2(player2RRun3, 0xe4, 3);
DEF_METASPRITE_2x2(player2RJump, 0xe8, 3);
DEF_METASPRITE_2x2(player2RClimb, 0xec, 3);
DEF_METASPRITE_2x2(player2RSad, 0xf0, 3);

DEF_METASPRITE_2x2_FLIP(player2LStand, 0xd8, 3);
DEF_METASPRITE_2x2_FLIP(player2LRun1, 0xdc, 3);
DEF_METASPRITE_2x2_FLIP(player2LRun2, 0xe0, 3);
DEF_METASPRITE_2x2_FLIP(player2LRun3, 0xe4, 3);
DEF_METASPRITE_2x2_FLIP(player2LJump, 0xe8, 3);
DEF_METASPRITE_2x2_FLIP(player2LClimb, 0xec, 3);
DEF_METASPRITE_2x2_FLIP(player2LSad, 0xf0, 3);

const unsigned char* const playerRunSeq[2][16] = 
{
  {
    player1LRun1, player1LRun2, player1LRun3, 
    player1LRun1, player1LRun2, player1LRun3, 
    player1LRun1, player1LRun2,
    player1RRun1, player1RRun2, player1RRun3, 
    player1RRun1, player1RRun2, player1RRun3, 
    player1RRun1, player1RRun2
  },
  {
    player2LRun1, player2LRun2, player2LRun3, 
    player2LRun1, player2LRun2, player2LRun3, 
    player2LRun1, player2LRun2,
    player2RRun1, player2RRun2, player2RRun3, 
    player2RRun1, player2RRun2, player2RRun3, 
    player2RRun1, player2RRun2
  }
};

const unsigned char* const playerStandR[2] = 
{
  player1RStand, player2RStand
};

const unsigned char* const playerStandL[2] = 
{
  player1LStand, player2LStand
};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x00,			// screen color

  0x15,0x30,0x27,0x00,	// background palette 0
  0x1C,0x20,0x2C,0x00,	// background palette 1
  0x00,0x10,0x20,0x00,	// background palette 2
  0x06,0x16,0x26,0x00,	// background palette 3

  0x16,0x35,0x24,0x00,	// sprite palette 0
  0x00,0x37,0x25,0x00,	// sprite palette 1
  0x0D,0x2D,0x3A,0x00,	// sprite palette 2
  0x0C,0x27,0x2A	// sprite palette 3
};

// setup PPU and tables
void setup_graphics() 
{
  // clear sprites
  oam_hide_rest(0);
  // set palette colors
  pal_all(PALETTE);
  // turn on PPU
  ppu_on_all();
}

// number of actors (4 h/w sprites each)
#define NUM_ACTORS 2

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

void move_player()
{
  char i;
  byte pad;
  for (i=0; i<2; i++) 
  {
    // poll controller i (0-1)
    pad = pad_poll(i);
    // move actor[i] left/right
    if (pad&PAD_LEFT && actor_x[i]>6) 
    {
      actor_dx[i]=-2;
      actor_run_dir[i] = 0;
    }
    else if (pad&PAD_RIGHT && actor_x[i]<234) 
    {
      actor_dx[i]=2;
      actor_run_dir[i] = 1;
    }
    else 
      actor_dx[i]=0;
    
    // ACTIONS
    
    if(pad == 0) //Player 1 Controls
    {
      if (pad&PAD_A)
      {
        
      }
    }
    else // Player 2 Controls
    {
      
    }
  }
}
  
// main program
void main() {
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
  while (1) {
    // start with OAMid/sprite 0
    oam_id = 0;
    // set player 0/1 velocity based on controller
    
    move_player();
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 7;
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
          oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerStandR[i]);
        else
          oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerStandL[i]);
      }
      
    }
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    ppu_wait_frame();
  }
}
