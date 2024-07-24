//
// Created by ethan on 7/23/2024.
//

#ifndef MINESWEEPER_VIDEO_H
#define MINESWEEPER_VIDEO_H

#define MEM_OAM     0x07000000;

volatile unsigned short* bg0_control = (volatile unsigned short*) 0x4000008;
volatile unsigned short* bg1_control = (volatile unsigned short*) 0x400000A;
volatile unsigned short* bg_palette = (volatile unsigned short*) 0x5000000;
volatile unsigned short* obj_palette = (volatile unsigned short*) 0x5000200;
unsigned short* oam_control = (unsigned short*) MEM_OAM;

/* return a pointer to one of the 4 character blocks (0-3) */
INLINE volatile unsigned short* char_block(unsigned long block) {
    /* they are each 16K big */
    return (volatile unsigned short*) (0x6000000 + (block * 0x4000));
}

/* return a pointer to one of the 32 screen blocks (0-31) */
INLINE volatile unsigned short* screen_block(unsigned long block) {
    /* they are each 2K big */
    return (volatile unsigned short*) (0x6000000 + (block * 0x800));
}


#endif //MINESWEEPER_VIDEO_H
