//This program by Michael Krause is licenced under the terms of the GPLv3.

#include <stdint.h>         //for uint8_t Datatype
#include <avr/pgmspace.h>   //for PROGMEM flag
#include <string.h>
#include "voice.h"
#include <stddef.h>         //for NULL definition
#include <ctype.h>          //for character operations
#include "talkie.h"
#include "HamTalk.h"

const uint8_t* StringToNato(const char* str) 
{
    if (strlen(str) == 1) 
    {
        char c = str[0];
        if (islower(c)) 
        {
            c = toupper(c);
        }

        switch(c) 
        {
            case 'A': return spALPHA;
            case 'B': return spBRAVO;
            case 'C': return spCHARLIE;
            case 'D': return spDELTA;
            case 'E': return spECHO;
            case 'F': return spFOXTROT;
            case 'G': return spGOLF;
            case 'H': return spHOTEL;
            case 'I': return spINDIA;
            case 'J': return spJULIET;
            case 'K': return spKILO;
            case 'L': return spLIMA;
            case 'M': return spMIKE;
            case 'N': return spNOVEMBER;
            case 'O': return spOSCAR;
            case 'P': return spPAPA;
            case 'Q': return spQUEBEC;
            case 'R': return spROMEO;
            case 'S': return spSIERRA;
            case 'T': return spTANGO;
            case 'U': return spUNIFORM;
            case 'V': return spVICTOR;
            case 'W': return spWHISKY;
            case 'X': return spXRAY;
            case 'Y': return spYANKEE;
            case 'Z': return spZULU;
            case '0': return spZERO;
            case '1': return spONE;
            case '2': return spTWO;
            case '3': return spTHREE;
            case '4': return spFOUR;
            case '5': return spFIVE;
            case '6': return spSIX;
            case '7': return spSEVEN;
            case '8': return spEIGHT;
            case '9': return spNINE;

            default: return NULL;
        }
    } else 
    {
        if (strcmp(str, "10") == 0) return spTEN;
        if (strcmp(str, "11") == 0) return spELEVEN;
        if (strcmp(str, "12") == 0) return spTWELVE;
        /*
        if (strcmp(str, "13") == 0) return spTHIRTEEN;
        if (strcmp(str, "14") == 0) return spFOURTEEN;
        if (strcmp(str, "15") == 0) return spFIFTEEN;
        if (strcmp(str, "16") == 0) return spSIXTEEN;
        if (strcmp(str, "17") == 0) return spSEVENTEEN;
        if (strcmp(str, "18") == 0) return spEIGHTEEN;
        if (strcmp(str, "19") == 0) return spNINETEEN;
        if (strcmp(str, "20") == 0) return spTWENTY;
        */

        return NULL;
    }
}

// This function spells a word
void spellString(Talkie* voice, char* str)
{
    if (str == NULL || voice == NULL)
    {
        return;
    }
    else
    {
        for (int i = 0; str[i] != '\0'; i++)
        {
            char tempStr[2] = {str[i], '\0'};
            if(StringToNato(tempStr) != NULL)
            {
                voice->say(StringToNato(tempStr));
            }
        }
    }
}

void intToCharArray(uint8_t number, char *charArray)
{
    // //raw DCF77 values are max. two digits
    if (number > 99)
    {
        number = 0;
    }

    charArray[0] = (number / 10) + '0'; // Zehnerstelle oder 0 bei Zahlen < 10
    charArray[1] = (number % 10) + '0'; // Einerstelle
    charArray[2] = '\0'; // Nullterminator
}