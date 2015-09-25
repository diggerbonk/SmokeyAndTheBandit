/*
        Smokey and the bandit high score save routines, borrowed from 
        Uzebox Ruby Crush. Original copyright notice follows:
        
        Ruby Crush for the Uzebox
        Copyright 2010, Kenton Hamaluik

        This file is part of Ruby Crush for the Uzebox.

        Ruby Crush for the Uzebox is free software: you can redistribute
        it and/or modify it under the terms of the GNU General Public License
        as published by the Free Software Foundation, either version 3 of the
        License, or (at your option) any later version.

        Ruby Crush for the Uzebox is distributed in the hope that it
        will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with The Uzebox Implementation of Pentago.  If not, see
        <http://www.gnu.org/licenses/>.
*/

#define SMOKEY_EEPROM_ID     (23)
#define MAX_HIGH_SCORES 4

struct EepromBlockStruct eeprom;

//
// Set the N'th highest score
//

void SetHighScore(u8 number, char i1, char i2, char i3, u32 score)
{
        // calculate the offset byte we're starting at
        u8 offset = (number * 6);
        
        // set the initials
        eeprom.data[offset + 0] = i1;
        eeprom.data[offset + 1] = i2;
        eeprom.data[offset + 2] = i3;
        
        // set the score
        eeprom.data[offset + 3] = (char)((score &0xff0000) >> 16);
        eeprom.data[offset + 4] = (char)((score &0xff00) >>  8);
        eeprom.data[offset + 5] = (char)((score &0xff));
}

// 
// Get the N'th highest score
//

void GetHighScore(u8 number, char *i1, char *i2, char *i3, u32 *score)
{
        // calculate the offset byte we're starting at
        u8 offset = (number * 6);
        
        // get the initials
        *i1 = eeprom.data[offset + 0];
        *i2 = eeprom.data[offset + 1];
        *i3 = eeprom.data[offset + 2];
        
        // get the score
        *score = (u32)(((u32)eeprom.data[offset + 3] << 16) | ((u32)eeprom.data[offset + 4] << 8) | ((u32)eeprom.data[offset + 5]));
}

//
// Initialize high scores. Read them from eeprom, if the eeprom is 
// not formatted then we set them to zero. 
void InitHighScores()
{
// This is commented out because it should never be the case that the 
// eeprom is not formatted.
//        // make sure we're formatted
//        if(!isEepromFormatted()) {
//			FormatEeprom();
//			SetHighScore(0, ' ', ' ', ' ', 0);
//			SetHighScore(1, ' ', ' ', ' ', 0);
//			SetHighScore(2, ' ', ' ', ' ', 0);
//			SetHighScore(3, ' ', ' ', ' ', 0);
//		}
                
        eeprom.id = SMOKEY_EEPROM_ID;
                
        // now read in the struct
        EepromReadBlock(SMOKEY_EEPROM_ID, &eeprom);
}

void WriteHighScores()
{
        // set the identifer bytes to be sure
        eeprom.id = SMOKEY_EEPROM_ID;
        
        // write it out!
        EepromWriteBlock(&eeprom);
}

bool IsHighScore(u32 newScore)
{
	// compare new score to the lowest high score, if it is greater 
	// then this is a new high score. 
	char i;
	u32 score;
    GetHighScore(3, &i, &i, &i, &score);
    
    if (newScore > score) return true;
    else return false;
}

u8 GetScoreRank(u32 score)
{
        char i;
        
        u32 score1, score2, score3, score4;
        GetHighScore(0, &i, &i, &i, &score1);
        GetHighScore(1, &i, &i, &i, &score2);
        GetHighScore(2, &i, &i, &i, &score3);
        GetHighScore(3, &i, &i, &i, &score4);
        

        if(score <= score4)
                return 5; // nope
        if(score <= score3)
                return 4;
        if(score <= score2)
                return 3;
        if(score <= score1)
                return 2;
        
        // the only way we get down here is if we beat score 1!
        return 1;
}

void NewHighScore(char i1, char i2, char i3, u32 score)
{
        u8 rank = GetScoreRank(score);
        
        // shift the old scores down
        for(u8 i = MAX_HIGH_SCORES; i > rank; i--)
        {
                char oi1, oi2, oi3;
                u32 oscore;
                GetHighScore(i - 2, &oi1, &oi2, &oi3, &oscore);
                SetHighScore(i - 1, oi1, oi2, oi3, oscore);
        }
        
        // set the new score
        SetHighScore(rank - 1, i1, i2, i3, score);
        
        // and write it out
        WriteHighScores();
}

