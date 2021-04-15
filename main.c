//Tyler Swearingen's NES game

#include <stdlib.h>
#include <string.h>
#include <time.h>

// vertical mirroring for scroll
#define NES_MIRRORING 0

// include NESLIB header
#include "neslib.h"

// include CC65 NES Header (PPU)
#include <nes.h>

// include VRAMBUF Header
#include "vrambuf.h"

// link the pattern table into CHR ROM
//#link "sprites.s"
//#link "vrambuf.c"


/////////////////// METASPRITES //////////////////////////////////

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

//Idle
DEF_METASPRITE_2x2_FLIP_V(p1Idle, 0x04, 0);
//Rolling Right
DEF_METASPRITE_2x2_FLIP_V(p1RollR1, 0x0c, 0);
DEF_METASPRITE_2x2_FLIP_V(p1RollR2, 0x14, 0);
//Rolling Left
DEF_METASPRITE_2x2_FLIP_HV(p1RollL1, 0x0c, 0);
DEF_METASPRITE_2x2_FLIP_HV(p1RollL2, 0x14, 0);




//Player 2

//Idle
DEF_METASPRITE_2x2(p2Idle, 0x08, 1);
//Rolling Right
DEF_METASPRITE_2x2(p2RollL1, 0x10, 1);
DEF_METASPRITE_2x2(p2RollL2, 0x18, 1);
//Rolling Left
DEF_METASPRITE_2x2_FLIP_H(p2RollR1, 0x10, 1);
DEF_METASPRITE_2x2_FLIP_H(p2RollR2, 0x18, 1);

////////////////////////////////////////////////////////////////////////


//////////////////////////// STATUS BAR /////////////////////////////////////////

const unsigned char status_bar[95]=
{
  0x01,0x00,0x71,0x6f,0x01,0x0c,0x73,0x71,0x6f,0x01,0x0c,0x73,0x00,0x00,0x6e,0x5f,
  0x60,0x00,0x01,0x08,0x5f,0x62,0x6d,0x6e,0x5f,0x63,0x00,0x5f,0x60,0x00,0x01,0x07,
  0x6d,0x00,0x00,0x6e,0x00,0x01,0x0a,0x65,0x67,0x6d,0x6e,0x69,0x6b,0x00,0x01,0x0a,
  0x6d,0x00,0x00,0x6e,0x34,0x60,0x00,0x00,0x5e,0x5f,0x60,0x00,0x01,0x03,0x66,0x68,
  0x6d,0x6e,0x6a,0x6c,0x00,0x35,0x60,0x00,0x00,0x5e,0x5f,0x60,0x00,0x01,0x02,0x6d,
  0x00,0x00,0x72,0x70,0x01,0x0c,0x74,0x72,0x70,0x01,0x0c,0x74,0x00,0x01,0x00
};

//////////////////////////////////////////////////////////////////////////////////


const unsigned char* const playerRollSeq[2][13] = 
{
  {
    p1Idle,
    p1RollL1, p1RollL1, p1RollL1,
    p1RollL2, p1RollL2, p1RollL2,
    p1RollR1, p1RollR1, p1RollR1,
    p1RollR2, p1RollR2, p1RollR2,
  },
  {
    p2Idle,
    p2RollL1, p2RollL1, p2RollL1,
    p2RollL2, p2RollL2, p2RollL2,
    p2RollR1, p2RollR1, p2RollR1,
    p2RollR2, p2RollR2, p2RollR2,
  }
};




/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,			// screen color

  0x16,0x13,0x20,0x00,	// background palette 0
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
#define NUM_MISSILES 8
#define YOFFSCREEN 240

#define MISSILE_ID 0x43


typedef struct {
  byte xpos;
  byte ypos;
  signed char dx;
  signed char dy;
  char isFired;
} Missile;


// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];

byte rollseq[NUM_ACTORS];
byte actor_missile_num[NUM_ACTORS];

byte actor_can_fire[NUM_ACTORS];
int actor_cooldown[NUM_ACTORS];

Missile missiles[NUM_ACTORS][NUM_MISSILES];

int scroll_pos = 48;




byte rand_tile()
{
   // random tile#, some stars, some blank
  byte val = 0;
  if((rand() & 0xd2) == 0)
  {
  	val = rand() % 6 + 0x58;
  }
  return val;
};




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


void move_missiles() {
  byte i;
  byte j;
  
  for(i = 0; i < NUM_ACTORS; i++)
  {
    for (j = 0; j < NUM_MISSILES; j++) 
    { 
      Missile* mis = &missiles[i][j];
      if (mis->ypos != YOFFSCREEN) 
      {
        // hit the bottom or top?
        if ((byte)(mis->ypos += mis->dy) > YOFFSCREEN) 
        {
          mis->ypos = YOFFSCREEN;
          mis->isFired = false;
          actor_missile_num[i] += 1;
        }
      }
    }
  }
}



// move the players and get input
void player_input()
{
  char i;
  char j;
  
  byte pad;
  
  for (i=0; i<2; i++) 
  {
    // poll controller i (0-1)
    pad = pad_poll(i);
    
    // move actor[i] left/right
    if (pad & PAD_LEFT && actor_x[i] > 8) 
    {
      actor_dx[i]=-2;
    }
    else if (pad & PAD_RIGHT && actor_x[i] < 232) 
    {
      actor_dx[i]=2;
    }
    else
    {
      actor_dx[i]=0;
    }
     
    
    // ACTIONS //
    
    if (pad & PAD_A && actor_can_fire[i])
    {
      if(actor_missile_num[i] > 0)
      {
        Missile* mis;
        for(j = 0; j < NUM_MISSILES; j++)
        {
          mis = &missiles[i][j];
          if(!mis->isFired)
          {
            mis->isFired = true;
            break;
          }
        }
        actor_missile_num[i] -= 1;
        actor_can_fire[i] = false;
        actor_cooldown[i] = 10;
        if(i == 1)
        {
          mis->xpos = actor_x[i] + 4;
          mis->ypos = actor_y[i] - 16;
          mis->dy = -2;
        }
        else
        {
          mis->xpos = actor_x[i] + 4;
          mis->ypos = actor_y[i] + 16;
          mis->dy = 2;
        }
      }
    }
  }
}
  
// main program
void main() 
{
  byte oam_id;	// sprite ID	
  char i;
  char j;
  
  // initialize actors with random values
  for (i=0; i<NUM_ACTORS; i++) 
  {
    actor_x[i] = 240/2;
    if(i == 1)
    {
      actor_y[i] = 205;
    }
    else
    {
      actor_y[i] = 50;
    }
    actor_dx[i] = 0;
    actor_dy[i] = 0;
    actor_missile_num[i] = 8;
    actor_can_fire[i] = true;
    actor_cooldown[i] = 0;
    rollseq[i] = 0;
  }  
  
  // Starfield
  for (i = 0; i < 30; i++)
  {
    for (j = 0; j < 32; j++)
    {
      if(i > 5)
      {
       byte val = rand_tile();
       vram_adr(NTADR_A(j, i));   // The screen 
       vram_put(val);
 
       vram_adr(NTADR_C(j, i));    // The screen "above" the starting screen
       vram_put(val);
      }
      else
      {
        vram_adr(NTADR_A(j, i));   // The screen 
        vram_put(rand_tile());
 
        vram_adr(NTADR_C(j, i));    // The screen "above" the starting screen
        vram_put(rand_tile());
      }  
    }
  }
  
    // draw status_bar
  vram_adr(NTADR_A(0, 1));
  vram_unrle(status_bar);
  
  // setup graphics
  setup_graphics();
  
  // loop forever
  while (1) 
  {
    // start with OAMid/sprite 0
    oam_id = 0;
    
    // sprite
    oam_id = oam_spr(1, 46, 0x6f, 0, 0);
    
    // set player 0/1 velocity based on controller and get input
    player_input();
    
    move_missiles();
    
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) 
    {
      if((actor_cooldown[i] -= 1) <= 0);
      {
        actor_can_fire[i] = true;
        actor_cooldown[i] = 0;
      }
      
      rollseq[i] += 1;
      
      if (actor_dx[i] > 0)
      {
        rollseq[i] += 6;
        if (rollseq[i] >= 12)
          rollseq[i] = 12;
        
        oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRollSeq[i][rollseq[i]]);
      	actor_x[i] += actor_dx[i];
      }
      else if(actor_dx[i] < 0)
      {
        if(rollseq[i] >= 6)
          rollseq[i] = 6;
        
        oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRollSeq[i][rollseq[i]]);
      	actor_x[i] += actor_dx[i];
      }
      else
      {
        rollseq[i] = 0;
        oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRollSeq[i][rollseq[i]]);
      }
      
      
      for(j = 0; j < NUM_MISSILES; j++)
      {
      	Missile* mis = &missiles[i][j];
      	if (mis->ypos != YOFFSCREEN) 
      	{
         oam_id = oam_spr(mis->xpos, mis->ypos, 0x43, 0, oam_id);
        }
      }
    }
    
    
    scroll_pos += 1;
      
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
   	
    if(scroll_pos >= 288)
      scroll_pos = 48;
    
    splitxy(0, scroll_pos);
    
    // wait for next frame
    vrambuf_flush();
  }
}
