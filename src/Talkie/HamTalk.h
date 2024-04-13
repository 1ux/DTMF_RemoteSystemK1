
#ifndef _HamTalk_h_
#define _HamTalk_h_

class Talkie;  // Forward declaration of Talkie
//const uint8_t* CharToNato(char c);
const uint8_t* StringToNato(const char* str);
void spellString(Talkie* voice, char* str);
void intToCharArray(uint8_t number, char *charArray);

#endif