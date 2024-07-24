#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "toolbox.h"
#include "input.h"
#include "video.h"
#include "minesweeperTiles.h"
#include "menuTiles.h"
#include "selectorSprite.h"

#define gameBlock   ((u16*)screen_block(16))
#define menuBlock   ((u16*)screen_block(17))
#define gameTileset ((u32*)char_block(0))
#define menuTileset ((u32*)char_block(1))
#define objTiles    ((u32*)char_block(4))

#define FLAG    0x1
#define BOMB    0x2
#define EMPTY   0x3
#define BLANK   0xC

u16 palettes[52];
int boardWidth, boardHeight, bombs;
u32 randSeed;

void drawWord(int x, int y, char* text, int palette) {
    for(int i = 0; i < strlen(text); i++) {
        if(text[i] != ' ') menuBlock[y * 32 + x] = (text[i] - 0x40) | (palette << 12);
        x++;
    }
}

void numsWithArrows(int x, int y, int num, int palette) {
    menuBlock[y * 32 + x] = 37 | (palette << 12);
    menuBlock[y * 32 + x + 1] = (num/100) + 27 | (palette << 12);
    menuBlock[y * 32 + x + 2] = ((num / 10) % 10) + 27 | (palette << 12);
    menuBlock[y * 32 + x + 3] = (num % 10) + 27 | (palette << 12);
    menuBlock[y * 32 + x + 4] = 38 | (palette << 12);
}

void clearMenu() {
    for(int i = 0; i < 32 * 32; i++) {
        menuBlock[i] = 0;
    }
}

void clearGame() {
    for(int i = 0; i < 32 * 32; i++) {
        gameBlock[i] = BLANK;
    }
}

void menu() {
    REG_DISPCNT = 0x0200; //bg1 with no sprites
    drawWord(9, 2, "MINESWEEPER", 0);
    drawWord(11, 3, "FOR GBA", 0);

    drawWord(12, 5, "WIDTH", 0);
    numsWithArrows(12, 6, boardWidth, 1);


    drawWord(11, 8, "HEIGHT", 0);
    numsWithArrows(12, 9, boardHeight, 0);

    drawWord(12, 11, "BOMBS", 0);
    numsWithArrows(12, 12, bombs, 0);

    drawWord(12, 15, "START", 0);

    int select = 0;
    bool gameStart = false;

    while(!gameStart) {
        key_poll();
        if (select == 0) {
            if (key_hit(KEY_LEFT) && boardWidth > 5) {
                boardWidth--;
                numsWithArrows(12, 6, boardWidth, 1);
            } else if (key_hit(KEY_RIGHT) && boardWidth < 30) {
                boardWidth++;
                numsWithArrows(12, 6, boardWidth, 1);
            } else if (key_hit(KEY_DOWN) || key_hit(KEY_A)) {
                numsWithArrows(12, 6, boardWidth, 0);
                numsWithArrows(12, 9, boardHeight, 1);
                randSeed+=10;
                select = 1;
            }
        } else if (select == 1) {
            if (key_hit(KEY_LEFT) && boardHeight > 5) {
                boardHeight--;
                numsWithArrows(12, 9, boardHeight, 1);
            } else if (key_hit(KEY_RIGHT) && boardHeight < 20) {
                boardHeight++;
                randSeed-=30;
                numsWithArrows(12, 9, boardHeight, 1);
            } else if (key_hit(KEY_DOWN) || key_hit(KEY_A)) {
                numsWithArrows(12, 9, boardHeight, 0);
                numsWithArrows(12, 12, bombs, 1);
                select = 2;
                randSeed+=5;
            } else if (key_hit(KEY_UP) || key_hit(KEY_B)) {
                numsWithArrows(12, 6, boardWidth, 1);
                numsWithArrows(12, 9, boardHeight, 0);
                select = 0;
            }
        } else if (select == 2) {
            if (key_hit(KEY_LEFT) && bombs > 10) {
                bombs--;
                numsWithArrows(12, 12, bombs, 1);
            } else if (key_hit(KEY_RIGHT) && bombs < (boardWidth * boardHeight) / 2) {
                bombs++;
                numsWithArrows(12, 12, bombs, 1);
            } else if (key_hit(KEY_DOWN) || key_hit(KEY_A)) {
                numsWithArrows(12, 12, bombs, 0);
                drawWord(12, 15, "START", 1);
                select = 3;
            } else if (key_hit(KEY_UP) || key_hit(KEY_B)) {
                numsWithArrows(12, 9, boardHeight, 1);
                numsWithArrows(12, 12, bombs, 0);
                select = 1;
            }
        } else if (select == 3) {
            if (key_hit(KEY_UP) || key_hit(KEY_B)) {
                numsWithArrows(12, 12, bombs, 1);
                drawWord(12, 15, "START", 0);
                select = 2;
            } else if (key_hit(KEY_A) || key_hit(KEY_START)) {
                gameStart = true;
            }
        }
        if (bombs > (boardHeight * boardWidth / 2)) {
            bombs = (boardHeight * boardWidth) / 2;
            if (select == 2) numsWithArrows(12, 12, bombs, 1);
            else numsWithArrows(12, 12, bombs, 0);
        }
        randSeed++;
    }
}

void changeTile(int x, int y, int tileID) {
    gameBlock[x + y * 32] = tileID;
}

void gameOver(u32 bombBoard[boardWidth][boardHeight], u32 statusBoard[boardWidth][boardHeight]) {
    for(int x = 0; x < boardWidth; x++) {
        for(int y = 0; y < boardHeight; y++) {
            if(bombBoard[x][y] == 1 && statusBoard[x][y] != 2) {
                changeTile(x, y, BOMB);
            }
        }
    }
}

int clicked(int x, int y, u32 bombBoard[boardWidth][boardHeight], u32 statusBoard[boardWidth][boardHeight]) {
    if(statusBoard[x][y]!=0) return 0;
    if(bombBoard[x][y]==1) {
        changeTile(x, y, BOMB);
        gameOver(bombBoard, statusBoard);
        return -1;
    }
    int nearby = 0;
    for(int i = x-1; i < x+2; i++) {
        for(int j = y-1; j < y+2; j++) {
            if((i != x || j != y) && i < boardWidth && i >= 0 && j < boardHeight && j >= 0) {
                if(bombBoard[i][j] == 1) nearby++;
            }
        }
    }
    changeTile(x, y, nearby + 3);
    statusBoard[x][y] = 1;
    if(nearby == 0) {
        for(int i = x-1; i < x+2; i++) {
            for(int j = y-1; j < y+2; j++) {
                if((i != x || j != y) && i < boardWidth && i >= 0 && j < boardHeight && j >= 0) {
                    clicked(i, j, bombBoard, statusBoard);
                }
            }
        }
    }
    return 0;
}

void toggleFlag(int x, int y, u32 statusBoard[boardWidth][boardHeight]) {
    if(statusBoard[x][y]==0) {
        changeTile(x, y, FLAG);
        statusBoard[x][y] = 2;
    } else if(statusBoard[x][y]==2) {
        changeTile(x, y, 0);
        statusBoard[x][y] = 0;
    }
}

int main() {
    randSeed = 0;
    boardWidth = 20;
    boardHeight = 20;
    bombs = 50;
    REG_DISPCNT = DCNT_OBJ | DCNT_MODE0 | DCNT_BG0;

    palettes[0] = RGB15(20, 20, 20); //background grey
    palettes[1] = RGB15(15, 15, 15); //border grey
    palettes[2] = RGB15(17, 17, 17); //corner of unclicked
    palettes[3] = CLR_BLACK; //black
    palettes[4] = CLR_RED; //red
    palettes[5] = CLR_WHITE; //white
    palettes[6] = RGB15(0, 6, 31); //1 blue
    palettes[7] = RGB15(0, 18, 3); //2 green
    palettes[9] = RGB15(0, 2, 14); //4 dark blue
    palettes[10] = RGB15(15, 0, 0); //5 dark red
    palettes[11] = RGB15(6, 18, 16); //6 cyan
    palettes[12] = RGB15(23, 23, 23); //popped grey

    palettes[16] = palettes[0];
    palettes[19] = palettes[6];

    palettes[32] = palettes[0];
    palettes[35] = palettes[4];

    palettes[48] = palettes[0];
    palettes[51] = CLR_YELLOW;


    for(int i = 0; i < 52; i++) {
        bg_palette[i] = palettes[i];
    }

    obj_palette[1] = CLR_YELLOW;

    for(int i = 0; i < 13 * 8; i++) {
        gameTileset[i] = minesweeperTiles[i/8][i%8];
    }

    for(int i = 0; i < 39 * 8; i++) {
        menuTileset[i] = menuTiles[i/8][i%8];
    }


    for(int i = 0; i < 8; i++) {
        objTiles[i+8] = selectorSprite[i];
    }


    /* set all control the bits in this register */
    *bg0_control = 0 |    /* priority, 0 is highest, 3 is lowest */
                   (0 << 2)  |       /* the char block the image data is stored in */
                   (0 << 6)  |       /* the mosaic flag */
                   (0 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
                   (16 << 8) |       /* the screen block the tile data is stored in */
                   (1 << 13) |       /* wrapping flag */
                   (0 << 14);        /* bg size, 0 is 256x256 */

    *bg1_control = 0 |    /* priority, 0 is highest, 3 is lowest */
                   (1 << 2)  |       /* the char block the image data is stored in */
                   (0 << 6)  |       /* the mosaic flag */
                   (0 << 7)  |       /* color mode, 0 is 16 colors, 1 is 256 colors */
                   (17 << 8) |       /* the screen block the tile data is stored in */
                   (1 << 13) |       /* wrapping flag */
                   (0 << 14);        /* bg size, 0 is 256x256 */

    for(int i = 0; i < 30; i++) {
        for(int j = 0; j < 20; j++) {
            changeTile(i, j, BLANK);
        }
    }

    while(1) {

        menu();
        clearGame();
        REG_DISPCNT = 0x1100; //sprites enabled and bg0

        u32 bombBoard[boardWidth][boardHeight];

        //0 = untouched
        //1 = touched
        //2 = flagged
        u32 statusBoard[boardWidth][boardHeight];

        for(int i = 0; i < boardWidth; i++) {
            for(int j = 0; j < boardHeight; j++) {
                changeTile(i, j, 0);
                bombBoard[i][j] = 0;
                statusBoard[i][j] = 0;
            }
        }

        srand(randSeed);



        for (int i = 0; i < bombs + 1; i++) {
            int x = rand() % boardWidth;
            int y = rand() % boardHeight;
            if (bombBoard[x][y] == 0) {
                bombBoard[x][y] = 1;
            } else { i--; }
        }


        oam_control[2] = 0x00000001;
        u16 *yPos = &oam_control[0];
        u16 *xPos = &oam_control[1];
        *xPos = 0;
        *yPos = 0;
        int timeBuffer = 0;
        int maxBuffer = 30000;
        bool gameOver = false;

        while (!gameOver) {
            key_poll();
            if (key_is_down(KEY_LEFT)) {
                if (!key_was_down(KEY_LEFT)) {
                    if (*xPos > 0) *xPos -= 8;
                    timeBuffer = 0;
                } else if (key_was_down(KEY_LEFT) && timeBuffer == 0) {
                    if (*xPos > 0) *xPos -= 8;
                }
            }
            if (key_is_down(KEY_RIGHT)) {
                if (!key_was_down(KEY_RIGHT)) {
                    if (*xPos < boardWidth*8 - 8) *xPos += 8;
                    timeBuffer = 0;
                } else if (key_was_down(KEY_RIGHT) && timeBuffer == 0) {
                    if (*xPos < boardWidth*8 - 8) *xPos += 8;
                }
            }
            if (key_is_down(KEY_UP)) {
                if (!key_was_down(KEY_UP)) {
                    if (*yPos > 0) *yPos -= 8;
                    timeBuffer = 0;
                } else if (key_was_down(KEY_UP) && timeBuffer == 0) {
                    if (*yPos > 0) *yPos -= 8;
                }
            }
            if (key_is_down(KEY_DOWN)) {
                if (!key_was_down(KEY_DOWN)) {
                    if (*yPos < boardHeight*8 - 8) *yPos += 8;
                    timeBuffer = 0;
                } else if (key_was_down(KEY_DOWN) && timeBuffer == 0) {
                    if (*yPos < boardHeight*8 - 8) *yPos += 8;
                }
            }
            if (key_hit(KEY_A)) {
                if (clicked(*xPos / 8, *yPos / 8, bombBoard, statusBoard) == -1) {
                    gameOver = true;
                    clearMenu();
                    drawWord(13, 9, "YOU", 2);
                    drawWord(13, 10, "LOSE", 2);
                    drawWord(10, 15, "PRESS A OR", 0);
                    drawWord(11, 16, "START TO", 0);
                    drawWord(10, 17, "PLAY AGAIN", 0);
                    key_poll();
                    while(!key_hit(KEY_START) && !key_hit(KEY_A)) key_poll();

                } else {
                    bool gameWon = true;
                    for(int x = 0; x < boardWidth; x++) {
                        for(int y = 0; y < boardHeight; y++) {
                            if(statusBoard[x][y]!=1 && bombBoard[x][y]!=1) gameWon = false;
                        }
                    }
                    if(gameWon) {
                        gameOver = true;
                        clearMenu();
                        drawWord(13, 9, "YOU", 3);
                        drawWord(13, 10, "WIN", 3);
                        drawWord(10, 15, "PRESS A OR", 0);
                        drawWord(11, 16, "START TO", 0);
                        drawWord(10, 17, "PLAY AGAIN", 0);
                        key_poll();
                        while(!key_hit(KEY_START) && !key_hit(KEY_A)) key_poll();
                    }
                }
            }
            if (key_hit(KEY_B)) {
                toggleFlag(*xPos / 8, *yPos / 8, statusBoard);
            }
            timeBuffer = (timeBuffer > maxBuffer) ? 0 : timeBuffer + 1;
        }
        REG_DISPCNT = 0x0200; //bg1 with no sprites
        key_poll();
        while(!key_hit(KEY_START) && !key_hit(KEY_A)) key_poll();
        clearMenu();
    }
}
