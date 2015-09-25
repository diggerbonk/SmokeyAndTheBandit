/*
 *  Uzebox Default Patches
 *  Copyright (C) 2008  Alec Bourque
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

/*Payments:
	 
	Patches are made of a command stream made of 3 bytes per command:
		1=delta time
		2=command
		3=command parameter

	Patches must start with 1 byte describing the sound type:
		0=wave channel (i.e.; channel 0,1 and 2)
		1=noise channel (channel 3)
		2=PCM (channel 3)
		->For type=2 two more byte follows in order: sample adress low byte, sample adress hi byte

	It must end with <0,PATCH_END> and this command takes
	only two bytes (no parameter).
*/

////INST: east bound and down lead
//const char patch00[] PROGMEM ={ 
//    0,PC_WAVE,3,
//    0,PC_ENV_SPEED,-10,
//    0,PATCH_END
//};

//INST: organ (east bound and down lead)
const char patch00[] PROGMEM ={	
0,PC_WAVE,9,
1,PC_ENV_VOL,192,
//1,PC_NOTE_HOLD,0,
1,PC_ENV_SPEED,-4, 
0,PATCH_END
};

//INST: east bound and down bass
/*const char patch01[] PROGMEM ={ 
    0,PC_WAVE,3,
    0,PC_ENV_SPEED,-1,
    0,PATCH_END
};*/
const char patch01[] PROGMEM ={//Music Demo	
0,PC_WAVE,3,
1,PC_ENV_VOL,192,
//1,PC_NOTE_HOLD,0,
1,PC_ENV_SPEED,-4, 
0,PATCH_END
};

//INST: bass -  triangular  Channel 3...
const char patch02[] PROGMEM ={	
    0,PC_WAVE,3,
    0,PC_ENV_SPEED,0,
    0,PATCH_END
};

//INST: hi-hat              Channel 4...
const char patch03[] PROGMEM ={	
    0,PC_WAVE,3,
    0,PC_ENV_SPEED,0,
    0,PATCH_END
}; 
// coinup
const char patch04[] PROGMEM ={
0,PC_ENV_SPEED,-15, 
1,PC_ENV_VOL,255,
25,PC_NOISE_PARAMS,255,
3,PC_NOTE_CUT,0,
0,PATCH_END
};

// jump
const char patch05[] PROGMEM ={ 
0,PC_WAVE,0,
1,PC_ENV_VOL,255,
0,PC_ENV_SPEED,-4,
0,PC_PITCH,80,
6,PC_NOTE_UP,1, 
6,PC_NOTE_UP,1,
6,PC_NOTE_UP,1,
6,PC_NOTE_DOWN,1,  
6,PC_NOTE_DOWN,1,  
6,PC_NOTE_DOWN,1,  
0,PC_NOTE_CUT,0,	
0,PATCH_END
};



// engine

const char patch06[] PROGMEM ={
0,PC_WAVE,3,
0,PC_ENV_VOL,127,
0,PC_PITCH,80,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
1,PC_ENV_VOL,0,
3,PC_ENV_VOL,127,
0,PC_PITCH,80,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
1,PC_ENV_VOL,0,
3,PC_PITCH,80,
0,PC_ENV_VOL,32,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
1,PC_NOTE_DOWN,6,
//1,PC_ENV_VOL,0,
//3,PC_PITCH,80,
//0,PC_ENV_VOL,15,
//1,PC_NOTE_DOWN,6,
//1,PC_NOTE_DOWN,6,
//1,PC_NOTE_DOWN,6,
//1,PC_NOTE_DOWN,6,
//1,PC_ENV_VOL,0,
1,PC_NOTE_CUT,0,
0,PATCH_END 
};


// engine
//const char patch07[] PROGMEM ={
//    0,PC_WAVE,3,
//    0,PC_NOTE_CUT, 0, 
//    0,PATCH_END
//};


const char patch07[] PROGMEM ={// turn noise
0,PC_WAVE,10,//3 seems good too
0, PC_ENV_VOL, 255,
0,PC_ENV_SPEED,-4,
//0, PC_TREMOLO_LEVEL, 255, 
//0, PC_TREMOLO_RATE, 32, 
0,PC_PITCH,5,
0,PC_ENV_VOL,255,
4,PC_NOTE_CUT,0,
0,PATCH_END,0,
};

const char patch08[] PROGMEM ={// explosion!
0,PC_ENV_VOL,127,
0,PC_WAVE,10, 
0,PC_ENV_SPEED,-1, 
0,PC_PITCH,67,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
2,PC_ENV_VOL,100,
0,PC_PITCH,60,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
2,PC_ENV_VOL,70,
0,PC_PITCH,60,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
2,PC_ENV_VOL,40,
0,PC_PITCH,60,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
2,PC_ENV_VOL,20,
0,PC_PITCH,60,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_DOWN,4,
1,PC_NOTE_CUT,0,
0,PATCH_END 
};

const struct PatchStruct patches[] PROGMEM = {
    {0,NULL,patch00,0,0}, // 
    {0,NULL,patch01,0,0}, // bass
    {0,NULL,patch02,0,0},
    {0,NULL,patch03,0,0},
    {0,NULL,patch04,0,0},
    {0,NULL,patch05,0,0},
    {0,NULL,patch06,0,0},
    {0,NULL,patch07,0,0},
    {0,NULL,patch08,0,0},
};

