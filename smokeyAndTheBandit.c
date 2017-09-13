/*
 *  Smokey and the Bandit
 *  Copyright (C) 2013  Trent McNair
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//
// Resolution is 224x224.
// Veritical scrolling controlled by value of Screen.scrollY
// Screen.scrollY can be a value from 0 to 255
// For Y: 256 scroll points - 250 max resolution = 16.
// For vertical scrolling, create a background with height at least 256.
//
// Screen Layout (turned sideways - the game is vertially oriented)
//
// +-------------------------+
// |     status area         |
// |     224x64              |
// +-------------------------+
// |     playfield           |
// |     224x160             |
// |                         |
// |                         |
// +-------------------------+
//
// * The road is 160 pixels, or 20 tiles wide
// * The driveable road width is defined by roadStart and roadEnd
//   variables, these are used for car/wall collision detection.
//
// 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19
//
// The driving course is made up of vertical strips of tile measuring
// 2x20 each. Each strip is defined in a 7 byte array:
//     1) top right border (how far the car can move right)
//     2) top left border (how far the car can move left)
//     3) top track index (where the tiles to draw this strip can be found)
//     4) bottom right border (how far the car can move right)
//     5) bottom left border (how far the car can move right)
//     6) bottom track index (where the tiles to draw this strip can be found)
//     7) Number of times to repeat this strip
//
// UZEM keyboard mappint (make sure to start with option -2)
//
//     o - coin up
//     u - coin up
//     w - p1 right
//     s - p1 left
//     a - p1 up
//     d - p1 down
//

#define LEFT_DIALOG_POS 21
#define MAX_BEERS 4
#define OFF_SCREEN 240

#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>

#include "data/patches.h"
#include "data/east.h"

#include "data/sprites.inc" // 3194 bytes
//#include "data/all-graphics.inc"
#include "data/smokey-screen-graphics.h"
#include "data/game-screen-graphics.h"
#include "data/spacebar-screen-graphics.h"
#include "data/transition-screen-graphics.h"

#include "highscores.c"

#define LANE1 22
#define LANE2 54
#define LANE3 86
#define LANE4 118
#define LANEOFFSET 12

//
// Table defining strips of track. Each strip is 2x20 tiles and is
// defined by 7 values in the table:
//
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//
// Table defining the track. Each row has 7 values, they are used to randomly
// generate the track as the game progresses.
//     1) right side of road first row
//     2) left side of road on first row
//     3) offset in the tile map where the tiles for this stripe are defined
//     4) right side of road second row
//     5) left side of road for second row
//     6) offset in the tile map where the tiles for the second strip are defined
//     7) number of times to repeat this pattern

const unsigned char dirtRoad[] PROGMEM = {
    // single lane, right most
    LANE1-LANEOFFSET, LANE1+LANEOFFSET, 4, LANE1-LANEOFFSET, LANE1+LANEOFFSET, 5, 0,
    // double lane, right most
    LANE1-LANEOFFSET, LANE2+LANEOFFSET, 12, LANE1-LANEOFFSET, LANE2+LANEOFFSET, 13, 3,
    // single lane, right of center
    LANE2-LANEOFFSET, LANE2+LANEOFFSET, 6, LANE2-LANEOFFSET, LANE2+LANEOFFSET, 7, 0,
    // double lane, middle
    LANE2-LANEOFFSET, LANE3+LANEOFFSET, 14, LANE2-LANEOFFSET, LANE3+LANEOFFSET, 15, 3,
    // single lane, left of center
    LANE3-LANEOFFSET, LANE3+LANEOFFSET, 8, LANE3-LANEOFFSET, LANE3+LANEOFFSET, 9, 0,
    // double lane, left most
    LANE3-LANEOFFSET, LANE4+LANEOFFSET, 16, LANE3-LANEOFFSET, LANE4+LANEOFFSET, 17, 3,
    // single lane, left most
    LANE4-LANEOFFSET, LANE4+LANEOFFSET, 10, LANE4-LANEOFFSET, LANE4+LANEOFFSET, 11, 0,

    // first half of causeway
    LANE1+1, LANE1, 21, LANE1+1, LANE1, 20, 1,
    // second half of causeway
    LANE1+1, LANE1, 19, LANE1+2, LANE1, 18, 1
};

const unsigned char highway[] PROGMEM= {
    // beer stage
    LANE1-LANEOFFSET, LANE4+LANEOFFSET, 2,  LANE1-LANEOFFSET, LANE4+LANEOFFSET, 3,  0,
};

#define COURSE_MAP_WIDTH 28
char lastCourseLineGenerated = 0;
unsigned char waterCounter = 0;
char storeCourseLine = 0;

// Game stages. Each stage increases the speed and the variance
// in beer placement.

#define MAX_STAGES 16
#define MAX_SUBSTAGES 5
#define MIN_SPEED 1
#define MAX_SPEED 5

// Minimum speed by stage.
// MAKE SURE WE HAVE MAX_STAGES ENTRIES!
const unsigned char minSpeedTable[] PROGMEM = {
    2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5
};
unsigned char minSpeed = 1;
unsigned char banditSpeed = MIN_SPEED;

// This table defines how "random" the coors placement is.
// MAKE SURE WE HAVE MAX_STAGES ENTRIES!
const char beerVarianceTable[]  PROGMEM = {
    100, 90, 80, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10
};

// how long each generated road segment will be, on average.
const char roadVarianceTable[] PROGMEM = {
    5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1
};

// format for each stage:
//     <lead up>, <beerStageLen>, <dodgeLen>, <after>
const unsigned char stageLengthTable[] PROGMEM = {
    3, 20, 3, 60, 5,
    3, 21, 3, 70, 5,
    3, 22, 3, 80, 5,
    3, 23, 3, 90, 5,
    3, 24, 3, 100, 5,
    3, 25, 3, 100, 5,
    3, 26, 3, 100, 5,
    3, 27, 3, 110, 5,
    3, 28, 3, 110, 5,
    3, 29, 3, 110, 5,
    3, 30, 3, 120, 5,
    3, 31, 3, 130, 5,
    3, 32, 3, 140, 5,
    3, 33, 3, 150, 5,
    3, 34, 3, 160, 5,
    3, 35, 3, 170, 5,
};

unsigned char stageLength = 0;
unsigned char subStage = 0; // 0 = highway no beer, 1 highway beer, 2 highway no beer, 3 dodge
unsigned char stageStep = 0;

char beerVariance = 100;
char roadVariance = 10;

const char textTable[] = {
    20, 21, 22, 23, 24, 25, 26, 27, 0, 1,
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19
};

// variables relating to beer placement
struct BeerCan {
    bool enabled;
    unsigned int xpos;
    unsigned int ypos;
};
struct BeerCan beerCans[MAX_BEERS];
char lastBeerSpawnLane = 1;

// variables relating to trans-am position and physics.
unsigned char banditX = 180;
unsigned char banditY = 22;  // y screen position
unsigned char banditZ = 0;   // z axis position (for jumping)

unsigned char numCredits = 0;
unsigned char creditDebounce = 0;
unsigned char joystickDebounce = 0;
unsigned char gameMode = 0; // 0 = logo, 1 = dialog, 2 = demo, 3 = highscore, > 3 gamePlay

// variables relating to scrolling and track loading
unsigned char destX=30;
unsigned int scrollMark = 0;
unsigned char stripeToggle = 0;
unsigned char trackIndex = 0;
unsigned char roadStart = 0;
unsigned char roadEnd = 19;
unsigned char trackIndex2 = 0;
unsigned char roadStart2 = 0;
unsigned char roadEnd2 = 19;
unsigned int courseCount = 0;

// player status
unsigned char currentPlayer = 0;
unsigned char guysLeft[2] = { 0, 0 };
unsigned int playerScore[2] = { 0, 0 };
unsigned char gameStage[2] = {0, 0 };

unsigned char nextYPos = LANE2;
unsigned char nextXPos = 180;
char buttonReset = 0;
char randomNumber;

// length of the course[] array

// track the the curently displayed left and right road boundaries for
// all 28 rows of on-screen road
unsigned char courseRightBoundary[32];
unsigned char courseLeftBoundary[32];

//
// forward declarations

void clearCans();
void endTurn();
void generateNextStripe(int increment);
void spacebarLogoScreen();
void spawnBeer(char y);
void initGame(bool twoPlayer);
void playGame();
void dialogMode();
void myPrint(int x,int y,const char *string);
void slowPrint(int x,int y,const char *string);
void myPrintInt(int x,int y, char len, unsigned int val);
void doScrolling(int speed);

void processCredits(int joy1, int joy2);
void processGameControls();
void processControlsAndWait(unsigned char waitFor);
void displayHighScoresScreen();
void waitCycle();

void transitionScreen(
        const char * line1,
        unsigned char line1len,
        bool showCoors);
void highScoreScreen(unsigned int);
void carCrash();
void smokeyAndTheBanditLogoScreen();

int main() {

	InitHighScores();

	Screen.overlayHeight=8;
    InitMusicPlayer(patches);
    SetSpritesTileTable(spritesTiles);

    numCredits = 0;
    creditDebounce = 0;
    gameMode = 0;

    while (1) {
        if (gameMode == 3) playGame();
        else waitCycle();
    }
}

// initialize the screen
//    * clear the vram
//    * tile the screen with all black tiles.

void initScreen(bool spriteVisibility) {

    ClearVram();

    // clear the screen by filling it with black tiles.  seems like ClearVram
    // should do this, but if we don't do this we get garbage showing up in
    // the overlay area
    unsigned char c;
    for (unsigned char x=0; x<28; x++) {
        for (unsigned char y=0; y<8; y++) {
	        c=pgm_read_byte(&(map_blank[2]));
			SetTile(x,y+VRAM_TILES_V,c);
        }
    }

    SetSpriteVisibility(spriteVisibility);
	WaitVsync(2);
}

void initGame(bool twoPlayer) {
    gameMode = 3;
    gameStage[0] = 0;
    gameStage[1] = 0;
    lastBeerSpawnLane = 2;
    playerScore[0] = 0;
    playerScore[1] = 0;
    guysLeft[0] = 3;
    if (twoPlayer) guysLeft[1] = 3;
    else guysLeft[1] = 0;
}

void playGame()  {

    StopSong();

    // bandit speed and position
    minSpeed = pgm_read_byte(minSpeedTable + gameStage[currentPlayer]);
    subStage = 0;
    stageLength = pgm_read_byte(stageLengthTable + (gameStage[currentPlayer]*5));
    stageStep = 0;
    banditSpeed = minSpeed;
    banditX = 190 - (4*banditSpeed);
    banditY = LANE2; // start bendit in the 2nd lane
    banditZ = 0;
    nextYPos = banditY;
    nextXPos = banditX;

    // initialize the beer cans
    beerVariance = pgm_read_byte(beerVarianceTable + gameStage[currentPlayer]);
    for (int i=0; i<MAX_BEERS; i++) {
        beerCans[i].enabled = false;
        sprites[i].x = OFF_SCREEN;
        sprites[i].y = 28;
    }

    roadVariance = pgm_read_byte(roadVarianceTable + gameStage[currentPlayer]);

    destX=30;
    scrollMark = 0;
    stripeToggle = 0;
    trackIndex = 0;
    roadStart = 0;
    roadEnd = 19;
    trackIndex2 = 0;
    roadStart2 = 0;
    roadEnd2 = 19;
    courseCount = 0;

    Screen.scrollX = 0;
    Screen.scrollY = 0;

    if (currentPlayer == 0) transitionScreen(PSTR("READY PLAYER ONE?"), 18, true);
    else transitionScreen(PSTR("READY PLAYER TWO?"), 18, true);

    TriggerNote(0, 3, 20+(2*banditSpeed), 128);

    unsigned char frameCounter = 0;
    unsigned char spawnCounter = 0;

	SetTileTable(backgroundTiles);
	SetFontTilesIndex(BACKGROUNDTILES_SIZE);
    initScreen(true);

    MapSprite2(MAX_BEERS, map_bandit, 0);


    if (currentPlayer == 0) myPrint(0,6, PSTR("P1"));
    else myPrint(0,6, PSTR("P2"));

    myPrint(3,6, PSTR("STAGE"));
    myPrint(4,2, PSTR("-"));
    myPrint(26, 6, PSTR("CREDIT"));

    printStats();

    MoveSprite(MAX_BEERS, banditX, banditY, 2, 2);

    for (int i=0; i<240; i++) {
        doScrolling(1);
    }

    for (unsigned char i=0; i<MAX_BEERS; i++) {
        MapSprite2(i, map_beer, 0);
        sprites[i].x = OFF_SCREEN;
        sprites[i].y = 0;
    }

    if (guysLeft[currentPlayer] == 1) {
            myPrint(24, 6, PSTR("\"  "));
    }
    else if (guysLeft[currentPlayer] == 2) {
            myPrint(24, 6, PSTR("\"\" "));
    }
    else if (guysLeft[currentPlayer] == 3) {
            myPrint(24, 6, PSTR("\"\"\""));
    }

    FadeIn(2, true);

    while (true) {
    if (GetVsyncFlag()) {
        ClearVsyncFlag();

        randomNumber = rand();

        frameCounter++;
        if (frameCounter == 8) {
            // print scores/stats
            printStats();
            frameCounter = 0;
        }

        // move the playfield
        doScrolling(banditSpeed/2);

        if (subStage == 1) {
            if (spawnCounter > 0) spawnCounter--;
            else {
                spawnBeer(randomNumber);
                spawnCounter = 6 + (abs(randomNumber/7));
            }
        }

        // move the car
        MoveSprite(MAX_BEERS, banditX, banditY, 2, 2);

        // move the beer cans
        for (unsigned char i=0; i<MAX_BEERS; i++) {
            if (beerCans[i].enabled) {
                if (sprites[i].x > 220) {
                    // a beer can got by, pop all visible cans.
                    beerCans[i].enabled = false;
                    sprites[i].x = OFF_SCREEN;
                    carCrash();
                    clearCans();
                    endTurn();
                    return;
                }
                else {
                    sprites[i].x = sprites[i].x + (banditSpeed/2);
                    // collision?
                    if ( sprites[i].x > (banditX-8) &&
                         sprites[i].y > (banditY-8) &&
                         sprites[i].x < (banditX+16) &&
                         sprites[i].y < (banditY+16) ) {
                            TriggerFx(6,0xff,true);
                            beerCans[i].enabled = false;
                            sprites[i].x = OFF_SCREEN;
                            playerScore[currentPlayer] += (banditSpeed*(banditSpeed/2));
                    }
                }
            }
            else {
                sprites[i].x = OFF_SCREEN;
                sprites[i].y = 0;
            }
        }


        // process bandit jumping: load jumping sprites, update
        // sounds.
		if (banditZ > 0) {
			banditZ++;
			if (banditZ == 36) {
                MapSprite2(MAX_BEERS, map_bandit, 0);
                TriggerNote(0, 3, 20+(2*banditSpeed), 128);
                banditZ = 0;
			}
			else if (banditZ == 24) {
                MapSprite2(MAX_BEERS, map_banditDown, 0);
                TriggerNote(0, 3, 23+(2*banditSpeed), 164);
			}
			else if (banditZ == 12) {
                MapSprite2(MAX_BEERS, map_banditBig, 0);
                TriggerNote(0, 3, 28+(2*banditSpeed), 192);
		 	}
		}


        if (banditY < nextYPos) {
            banditY+=4;
            if (banditY == nextYPos) MapSprite2(MAX_BEERS, map_bandit, 0);
        }
        else if (banditY > nextYPos) {
            banditY-=4;
            if (banditY == nextYPos) MapSprite2(MAX_BEERS, map_bandit, 0);
        }
        else {
			if (banditX < nextXPos) {
				banditX++;
			}
			else if (banditX > nextXPos) {
				banditX--;
			}
		}

        processGameControls();

        // do bounds checking here, but only if we're not jumping
        if (banditZ == 0) {
            unsigned char minY = courseRightBoundary[((Screen.scrollX/8)+(banditX/8))%32];
            unsigned char maxY = courseLeftBoundary[((Screen.scrollX/8)+(banditX/8))%32];

            if (banditY > maxY || banditY < minY) {
                carCrash();
                endTurn();
                return;

            }
        }

        // calculate new stage and substage here.
        if (stageStep > stageLength) {
            subStage++;
            if (subStage < MAX_SUBSTAGES) {
                stageLength = pgm_read_byte(stageLengthTable + (gameStage[currentPlayer]*MAX_SUBSTAGES+subStage));
                stageStep = 0;
                if (subStage == 3) {
//                    TriggerNote(0, 7, 0, 0);
//                    StartSong(midisong);
                }
            }
            else {
                subStage = 0;
                gameStage[currentPlayer] = gameStage[currentPlayer] + 1;
                FadeOut(4, true);
                WaitVsync(10);
                //StopSong(midisong);
                return;
            }
        }
    } // if GetVsyncFlag
    } // while(true)
}

void printStats() {
    myPrintInt(1,21, 6, playerScore[currentPlayer]);
    myPrintInt(4,23, 2, gameStage[currentPlayer]+1);
    myPrintInt(4,21, 1,subStage+1);
    printCredits();
}

void printCredits() {
    myPrintInt(27,21,2, numCredits);
}

void carCrash()
{
    TriggerNote(0, 7, 0, 0);
    TriggerFx(8, 0xff, false);
    MapSprite2(MAX_BEERS, map_enemy, 0);
    for (int i=0; i<10; i++) {
        processControlsAndWait(2);
    }
    MapSprite2(MAX_BEERS, map_enemy2, 0);
    for (int i=0; i<10; i++) {
        processControlsAndWait(2);
    }
    MapSprite2(MAX_BEERS, map_enemy, 0);
    for (int i=0; i<10; i++) {
        processControlsAndWait(2);
    }
    MapSprite2(MAX_BEERS, map_enemy2, 0);
    for (int i=0; i<10; i++) {
        processControlsAndWait(2);
    }
    MapSprite2(MAX_BEERS, map_enemy3, 0);
    for (int i=0; i<10; i++) {
        processControlsAndWait(2);
    }
}

void clearCans()
{
    TriggerNote(0, 7, 0, 0);
    for (unsigned char i=0; i<MAX_BEERS; i++) {
        if (beerCans[i].enabled  ) {
            TriggerFx(6,0xff,true);
            beerCans[i].enabled = false;
            sprites[i].x = OFF_SCREEN;
            MapSprite2(i, map_beer, 0);

            for (int i=0; i<7; i++) {
                processControlsAndWait(2);
            }

         }
    }
}

void endTurn()
{
    if ((guysLeft[currentPlayer] == 1) && IsHighScore(playerScore[currentPlayer]) ) {
        highScoreScreen(playerScore[currentPlayer]);
    }

    guysLeft[currentPlayer] = guysLeft[currentPlayer] - 1;

    WaitVsync(10);

    currentPlayer++;
    if (currentPlayer > 1 || guysLeft[currentPlayer] == 0) currentPlayer = 0;

    if (guysLeft[0] == 0 && guysLeft[1] == 0) {
        Screen.scrollX =0;
        scrollMark = 0;
        transitionScreen(PSTR("GAME OVER"), 9, false);
        gameMode = 0;
        displayHighScoresScreen();
    }
}


void doScrolling(int speed) {

    scrollMark += speed;


    if (scrollMark >= 8) {
        generateNextStripe(scrollMark/8);
        scrollMark = (scrollMark%8);
    }

    Screen.scrollX -= speed;
}


void processCredits(int joy1, int joy2) {

    // uzebox jamma mapping
    //     <uzebox> | <jamma function> | <jamma pin>  |  <uzem keyboard>
    //
    //     BTN_SL(0)      : Service         : R
    //     BTN_SR(0)      : Test            : 15
    //     BTN_START(0)   : 1 player start  : 17    : z
    //     BTN_SELECT(0)  : 2 player start  : U     : x
    //     BTN_START(1)   : Tilt            : S
    //     BTN_L(1)       : Coin 1          : 16    : u
    //     BTN_R(1)       : Coin 2          : T     : o
    //     BTN_X(*)       : Button 1        : 22    :

    if (creditDebounce > 0) creditDebounce--;

	if(joy2&BTN_SL || joy2&BTN_SR){
        if (creditDebounce == 0) {
            TriggerFx(4,0xff,true);
            numCredits++;
            creditDebounce = 25;
            if (numCredits == 1 && gameMode < 3) {
                gameMode = 0;
            }
            printCredits();
        }
	}else if(joy1&BTN_START){ // start 1 player game
        if (creditDebounce == 0) {
            if (numCredits > 0 && gameMode != 3) {
                numCredits--;
                creditDebounce = 25;
                initGame(false);
                printCredits();
            }
        }
	}else if(joy1&BTN_SELECT || joy1&BTN_Y || joy1&BTN_A | joy1&BTN_B){  // start 2 player game
        if (creditDebounce == 0) {
            if (numCredits > 1 && gameMode != 3) {
                numCredits-=2;
                creditDebounce = 25;
                initGame(true);
                printCredits();
            }
        }
	}
}

void processGameControls() {

	unsigned int joy1=ReadJoypad(0);
	unsigned int joy2=ReadJoypad(1);

    processCredits(joy1, joy2);

    if (buttonReset == 0) {
        if (banditZ == 0) {
            if (joy1&BTN_RIGHT) {
                if (nextYPos > 32) {
                    nextYPos -= 32;
                    MapSprite2(MAX_BEERS, map_banditRight, 0);
                }
                else {
					nextYPos = 0;
				}
                TriggerFx(7,0xff,true);
                buttonReset = 2;
            }
            else if (joy1&BTN_LEFT) {
                if (nextYPos == 0) {
					nextYPos = LANE1;
				}
                else {
					nextYPos += 32;
                    MapSprite2(MAX_BEERS, map_banditLeft, 0);
				}
                MapSprite2(MAX_BEERS, map_banditLeft, 0);
                TriggerFx(7,0xff,true);
                buttonReset = 2;
            }
            else if (joy1&BTN_DOWN && banditSpeed > minSpeed) {
                banditSpeed--;
                buttonReset = 2;
                nextXPos = 190 - (4*banditSpeed);
                TriggerNote(0, 3, 20+(banditSpeed*2), 128);
            }
            else if (joy1&BTN_UP && banditSpeed < MAX_SPEED) {
                banditSpeed++;
                buttonReset = 2;
                nextXPos = 190 - (4*banditSpeed);
                TriggerNote(0, 3, 20+(banditSpeed*2), 128);
            }
            else if(joy1&BTN_X){
                banditZ = 1;
                MapSprite2(MAX_BEERS, map_banditUp, 0);
                TriggerNote(0, 3, 23+(2*banditSpeed), 164);
	        }
        }
    }
    else if (!(joy1&BTN_UP) && !(joy1&BTN_DOWN)&& !(joy1&BTN_RIGHT)&& !(joy1&BTN_LEFT)) buttonReset--;
}

void processControlsAndWait(unsigned char waitFor)
{
    for (unsigned char i = 0; i < waitFor; i++) {
	    int joy1=ReadJoypad(0);
	    int joy2=ReadJoypad(1);
        processCredits(joy1, joy2);
        WaitVsync(1);
    }
}

//Print a string from flash
void myPrint(int x,int y,const char *string){

	int i=0;
	char c;

	while(1){
		c=pgm_read_byte(&(string[i++]));
		if(c!=0){
			PrintChar(x, textTable[y--], c);
		}else{
			break;
		}
	}

}

//Print a string from flash
void slowPrint(int x,int y,const char *string){

	int i=0;
	char c;

	while(1){
		c=pgm_read_byte(&(string[i++]));
		if(c!=0){
		    PrintChar(x, textTable[y--],c);
		}else{
			break;
		}
        processControlsAndWait(5);
	}

}

//Print an unsigned byte in decimal
void myPrintInt(int x,int y, char len, unsigned int val){
    while (len > 0) {
        PrintChar(x, y++, (val%10)+48);
        val = val / 10;
        len--;
	}
}

void generateNextStripe(int increment) {

    while (increment > 0) {

        if (courseCount == 0) {

            if (subStage == 3) {

                // increment the water counter (is it time for water)
                waterCounter++;

                // increment or decrement the last course line
                if (waterCounter == 31) {
                    storeCourseLine = lastCourseLineGenerated;
                    courseCount = 3;
                }
                else if (waterCounter == 32) {
                    lastCourseLineGenerated = 7;
                    courseCount = 1;
                }
                else if (waterCounter == 33) {
                    lastCourseLineGenerated = 8;
                    courseCount = 1;
                }
                else if (waterCounter == 34) {
                    lastCourseLineGenerated = storeCourseLine;
                    courseCount = 6;
                    waterCounter = 0;
                }
                else {
                    unsigned char r = (unsigned char)randomNumber;

                    playerScore[currentPlayer] += (banditSpeed*(banditSpeed/2));

                    // update the course.
                    if (r > 128) {
                        lastCourseLineGenerated++;
                    }
                    else {
                        lastCourseLineGenerated--;
                    }

                    if (lastCourseLineGenerated < 0) lastCourseLineGenerated = 1;
                    else if (lastCourseLineGenerated > 6) lastCourseLineGenerated = 5;
                    courseCount = 3+((unsigned char)randomNumber)%roadVariance;
                }


                roadStart = pgm_read_byte(dirtRoad+(lastCourseLineGenerated*7));
                roadEnd =pgm_read_byte(dirtRoad+(lastCourseLineGenerated*7)+1);
                trackIndex = pgm_read_byte(dirtRoad+(lastCourseLineGenerated*7)+2);
                roadStart2 = pgm_read_byte(dirtRoad+(lastCourseLineGenerated*7)+3);
                roadEnd2 = pgm_read_byte(dirtRoad+(lastCourseLineGenerated*7)+4);
                trackIndex2 = pgm_read_byte(dirtRoad+(lastCourseLineGenerated*7)+5);
            }
            else {
                roadStart = LANE1-LANEOFFSET;
                roadEnd = LANE4+LANEOFFSET;
                trackIndex = 2;
                roadStart2 =  LANE1-LANEOFFSET;
                roadEnd2 = LANE4+LANEOFFSET;
                trackIndex2 = 3;
                courseCount = 10;
            }

            stageStep++;

        }

        unsigned char y = 0;

        if (stripeToggle > 0) {
            courseRightBoundary[destX] = roadStart2;
            courseLeftBoundary[destX] = roadEnd2;
            while (y < 20) {
                SetTile(destX, y, pgm_read_byte(&(map_terrain[trackIndex2 + (y*COURSE_MAP_WIDTH)])));
                y++;
            }
            courseCount--;
            stripeToggle = 0;
        }
        else {
            courseRightBoundary[destX] = roadStart;
            courseLeftBoundary[destX] = roadEnd;
            while (y < 20) {
                SetTile(destX, y, pgm_read_byte(&(map_terrain[trackIndex + (y*COURSE_MAP_WIDTH)])));
                y++;
            }
            stripeToggle++;
        }

        destX--;
        if (destX == 255) destX = 31;
        increment--;
    }
}


void spawnBeer(char sc)
{
    for (int i=0; i<MAX_BEERS; i++) {
        if (!beerCans[i].enabled) {
            beerCans[i].enabled = true;
            sprites[i].x = 0;

            // divide random char by beerVariance. beerVariance is a positive
            // integeter between [0-127]. The result should be 0, > 0 or < 0
            // The car moves per the direction of the result, if the car is
            // on the border, it will move the opposite direction.
            char c = (char)sc/beerVariance;
            if (c > 0) {
                if (lastBeerSpawnLane == 3) lastBeerSpawnLane = 2;
                else lastBeerSpawnLane++;
            }
            else if (c < 0) {
                if (lastBeerSpawnLane == 0) lastBeerSpawnLane = 1;
                else lastBeerSpawnLane--;
            }

            sprites[i].y = 28 + (32*lastBeerSpawnLane);
            break;
        }
    }
}

void waitCycle() {
    spacebarLogoScreen();
    if (gameMode > 2) return;
    smokeyAndTheBanditLogoScreen();
    if (gameMode > 2) return;
    dialogMode();
    if (gameMode > 2) return;
    displayHighScoresScreen();
    if (gameMode > 2) return;
}

void spacebarLogoScreen() {

    Screen.scrollX =0;
    scrollMark = 0;
	SetTileTable(spacebarLogoTiles);
    SetFontTilesIndex(SPACEBARLOGOTILES_SIZE);
    initScreen(false);

    FadeOut(0,true);

    for (char i = 0; i < 32; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

	DrawMap2(2,0,sbl_1);
	DrawMap2(2,20,sbl_2);

    //myPrint(26, 6, PSTR("CREDIT"));
    printCredits();

    FadeIn(1, true);

    slowPrint(14,18, PSTR(" PRESENTS"));

    for (char i = 0; i < 127; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

    FadeOut(0,true);

    for (char i = 0; i < 16; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

}

void smokeyAndTheBanditLogoScreen()
{

    Screen.scrollX =0;
    scrollMark = 0;
	SetTileTable(smokeyAndTheBanditLogoTiles);
    SetFontTilesIndex(SMOKEYANDTHEBANDITLOGOTILES_SIZE);
    initScreen(false);

//    FadeOut(0,true);

    for (char i = 0; i < 32; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

	DrawMap2(6,0,sabl_1);
	DrawMap2(6,23,sabl_2);

    //myPrint(26, 6, PSTR("CREDIT"));
    printCredits();

    FadeIn(1, true);

    StartSong(midisong);

    for (char i = 0; i < 64; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

    slowPrint(17,26, PSTR("INSERT COIN YOU SOM-BITCH!"));
    if (gameMode > 2) return;
    slowPrint(19,21, PSTR("1 COIN 1 CREDIT"));
    if (gameMode > 2) return;

    for (int i = 0; i < 440; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

    StopSong();

    for (char i = 0; i < 64; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

    FadeOut(1,true);

    for (char i = 0; i < 16; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }
}

void dialogMode() {

	SetTileTable(backgroundTiles);
	SetFontTilesIndex(BACKGROUNDTILES_SIZE);
    initScreen(false);

    for (char i = 0; i < 64; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

    //myPrint(26, 6, PSTR("CREDIT"));
    printCredits();

    FadeIn(0, true);

    DrawMap2(3, 15, map_banditAvatar);
    slowPrint(3,LEFT_DIALOG_POS,PSTR("GETTIN' TO\0"));
    if (gameMode > 2) return;
    slowPrint(4,LEFT_DIALOG_POS,PSTR("TEXARKANA AND BACK\0"));
    if (gameMode > 2) return;
    slowPrint(5,LEFT_DIALOG_POS ,PSTR("IN 28 HOURS, THATS\0"));
    if (gameMode > 2) return;
    slowPrint(6,LEFT_DIALOG_POS ,PSTR("NO PROBLEM -\0"));
    if (gameMode > 2) return;
    processControlsAndWait(40);
    if (gameMode > 2) return;

    DrawMap2(8, 15, map_enos);

    slowPrint(8,LEFT_DIALOG_POS ,PSTR("IT AIN'T NEVER\0"));
    if (gameMode > 2) return;
    slowPrint(9,LEFT_DIALOG_POS ,PSTR("BEEN DONE BEFORE,\0"));
    if (gameMode > 2) return;
    slowPrint(10,LEFT_DIALOG_POS ,PSTR("HOT SHIT.\0"));
    if (gameMode > 2) return;
    processControlsAndWait(40);
    if (gameMode > 2) return;

    DrawMap2(12, 15, map_banditAvatar);
    slowPrint(12,LEFT_DIALOG_POS ,PSTR("- BUT COORS BEER,\0"));
    if (gameMode > 2) return;
    slowPrint(13,LEFT_DIALOG_POS ,PSTR("YOU TAKE THAT EAST\0"));
    if (gameMode > 2) return;
    slowPrint(14,LEFT_DIALOG_POS ,PSTR("OF TEXAS... AND\0"));
    if (gameMode > 2) return;
    slowPrint(15,LEFT_DIALOG_POS ,PSTR("THAT'S BOOTLEGGIN'\0"));
    if (gameMode > 2) return;
    processControlsAndWait(20);
    if (gameMode > 2) return;
    slowPrint(16,LEFT_DIALOG_POS ,PSTR("WHY DO YOU WANT\0"));
    if (gameMode > 2) return;
    slowPrint(17,LEFT_DIALOG_POS ,PSTR("THAT BEER SO BAD?\0"));
    if (gameMode > 2) return;
    processControlsAndWait(30);
    if (gameMode > 2) return;

    DrawMap2(19, 15, map_enos);
    slowPrint(19,LEFT_DIALOG_POS ,PSTR("BECAUSE WE'RE\0\0\0\0\0\0"));
    if (gameMode > 2) return;
    slowPrint(20,LEFT_DIALOG_POS ,PSTR("THIRSTY, DUMMY.\0\0\0"));
    if (gameMode > 2) return;
    processControlsAndWait(70);
    if (gameMode > 2) return;

    FadeOut(0, true);

    for (char i = 0; i < 64; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }
}

void transitionScreen(
        const char * line1,
        unsigned char line1len,
        bool showCoors)
{

	SetTileTable(coorsCan);
	SetFontTilesIndex(COORSCAN_SIZE);
    initScreen(false);

    //myPrint(26, 6, PSTR("CREDIT"));
    printCredits();


    for (char i = 0; i < 64; i++) {
       processControlsAndWait(1);
    }

    if (showCoors) {
	    DrawMap2(13,5,coors_can_map);
        myPrint(10, 25, PSTR("CLEDUS IS DROPPING COORS"));
        myPrint(11, 17, PSTR("GRAB IT!"));
    }


    myPrint(8, 13 + (line1len/2), line1);
    FadeIn(2, true);

    if (showCoors) StartSong(midisong);


    for (char i = 0; i < 66; i++) {
       processControlsAndWait(1);
    }

    if (showCoors) {
        StopSong();
        for (char i = 0; i < 64; i++) {
           processControlsAndWait(1);
        }
    }

    FadeOut(0, true);
}

void highScoreScreen(unsigned int score)
{
    Screen.scrollX =0;
    scrollMark = 0;
	SetTileTable(smokeyAndTheBanditLogoTiles);
    SetFontTilesIndex(SMOKEYANDTHEBANDITLOGOTILES_SIZE);
    initScreen(false);

    FadeOut(0,true);

    for (char i = 0; i < 32; i++) {
       processControlsAndWait(1);
    }

	DrawMap2(2,0,sabl_1);
	DrawMap2(2,23,sabl_2);

    //myPrint(26, 6, PSTR("CREDIT"));
    printCredits();

    myPrint(12,25, PSTR("    CONGRATULATIONS!"));
    myPrint(14,25, PSTR("  YOUR SCORE IS IN THE"));
    myPrint(15,25, PSTR("TOP 10, USE THE JOYSTICK"));
    myPrint(16,25, PSTR(" TO ENTER YOUR INITIALS"));

    FadeIn(1, true);

    unsigned char currentLetter = 0;
    char initials[3] = { 'A', ' ', ' ' };
    u32 timer = 0;

    while (true) {
		timer++;
		if (timer > 1000) break;
	    unsigned int joy1=ReadJoypad(0);
	    unsigned int joy2=ReadJoypad(1);

        processCredits(joy1, joy2);

        if (buttonReset == 0) {

            if (joy1&BTN_RIGHT) {
                buttonReset = 3;
                if (initials[currentLetter] == 'Z') initials[currentLetter] = ' ';
                else if (initials[currentLetter] == ' ') initials[currentLetter] = 'A';
                else initials[currentLetter] = initials[currentLetter] + 1;
            }
            else if (joy1&BTN_LEFT) {
                buttonReset = 3;
                if (initials[currentLetter] == ' ') initials[currentLetter] = 'Z';
                else if (initials[currentLetter] == 'A') initials[currentLetter] = ' ';
                else initials[currentLetter] = initials[currentLetter] - 1;
            }

            if (joy1&BTN_X) {
				timer = 0;
                buttonReset = 3;
                // set current letter
                currentLetter++;
                if (currentLetter < 3) initials[currentLetter] = 'A';
            }

        }
        else if (!(joy1&BTN_X) && !(joy1&BTN_LEFT)&& !(joy1&BTN_RIGHT)) {
            buttonReset--;
        }

        for (unsigned char i=0; i<3; i++) {
            if (i == currentLetter) {
                SetFont(20,8-i,RAM_TILES_COUNT + 28);
            }
            else {
                SetFont(20,8-i,RAM_TILES_COUNT);
            }
        }

        char c;
	    for (unsigned char i=0; i<3; i++) {

            c = initials[i];
            c = c + RAM_TILES_COUNT - 32;
            SetFont(19,8-i,c);
        }

        WaitVsync(1);
        if (currentLetter > 2) {
            break;
        }
    }

    NewHighScore(initials[0], initials[1], initials[2], score);

    FadeOut(1,true);
}


//
//
//          High Scores
//
//    TOM  .............  000678
//         .............  000000

void displayHighScoresScreen()
{
    Screen.scrollX = 0;
    scrollMark = 0;
	SetTileTable(smokeyAndTheBanditLogoTiles);
    SetFontTilesIndex(SMOKEYANDTHEBANDITLOGOTILES_SIZE);
    initScreen(false);

    // make sure we're dark while setting up the screen
    FadeOut(0,true);

    //myPrint(26, 6, PSTR("CREDIT"));
    printCredits();

    // pause
    for (unsigned char i = 0; i < 32; i++) {
       processControlsAndWait(1);
    }

    // print message
    myPrint(5,26, PSTR("       HIGH SCORES"));

    for (unsigned char i = 0; i<MAX_HIGH_SCORES; i++) {
		u32 score;
		char initials[3];

		GetHighScore(i, &initials[0], &initials[1], &initials[2], &score);

        if (score == 0) break;
	    for (unsigned char j=0; j<3; j++) {
            char c = initials[j];
            if (c != ' ' && (c < 'A' || c > 'Z')) c = ' ';

            c = c + RAM_TILES_COUNT - 32;

            SetFont(8+(i*2),textTable[25-j],c);

			myPrint(8+(i*2), 22, PSTR(" ............. "));
			myPrintInt(8+(i*2),22,6,score);
		}
    }

    FadeIn(1, true);

    for (unsigned char i = 0; i < 255; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }

    FadeOut(1,true);

    for (char i = 0; i < 16; i++) {
       processControlsAndWait(1);
       if (gameMode > 2) return;
    }
}

