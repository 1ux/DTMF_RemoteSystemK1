/*
This software allows remote control of an amateur radio station using DTMF tones. 
A transmitting station S sends the DTMF tones to a receiving station R. 
The software and hardware for controlling the receiving station is implemented in this project.
The receiving station does not need to be able to decode the DTMF tones. 
This task is performed by the external MT8870 chip, which converts the analogue signals into a bit sequence and sends it to a microcontroller. 
The actual control of the receiving station is performed by an 8-bit microcontroller (e.g. 
ATmega328 or LGT8F328P) with a clock frequency of at least 16 MHz. 
Although the control possibilities are limited only by your imagination, here is an example of a DCF77 time announcement via a synthetic voice output. 
In addition, various phrases, e.g. your own callsign, can be called up from receiving station R for a reception test.
This programme by Michael Krause is licensed under the GPLv3.
Please note that the authors of the libraries used may use different licences. (see links)
Please also note the legal regulations for radio transmissions in your country.
 *  
*/
#include "src/Talkie/talkie.h"   // emulates a hardware Texas Instruments speech synthesis architecture on fix digital pin 3
//Source: https://github.com/going-digital/Talkie
#include "src/Talkie/voice.h"    // predefined words for syntetized speech output
//Source: http://www.wo4rob.com/DTMF-Remote-Control.html
#include "src/Talkie/HamTalk.h"    //
#include "src/LED_RGB_Control/BasicColorLedControl.h"
//Source: https://github.com/1ux/LED_RGB_Control
#include "src/MT8870/MT8870.h"   // smal DTMF lib for MT8870 IC
//Source: https://github.com/RobTillaart/MT8870
#include "src/DCF77Decode/src/basic_dcf77.h"
//Source: https://github.com/1ux/DCF77Decode
#include "src/DebugProject.h"

Talkie voice;             		// initialize speech synthesis architecture on Pin 3
MT8870 DTMF;              		// initialize DTMF Chip
BasicColorLedControl RGBled;
const int buttonPin = A0; 		// define application pushbutton Port
const int PTTpin = A1;    		// define PTT button Port of the VHF/UHF radio, puts the radio on air
const byte interruptPin = 2;  // Interrupt pin, shows incoming DTMF signal

char inputString[5];      		//define received DTMF string, sent from other radios to this one
unsigned int inputIndex = 0;  //Variable for string indexing for inputString
char ch = 0;              		//DTMF receives sequentially, this variable holds the latest received character

uint8_t bitArray[DCF77_STRING_SIZE]; //for DCF77 receiving

//+++++++++++++++++++++++++++++++++ This string is sent via the spellString() function when the PTT is active
char callsign[]= {"DL1xxx"};
//+++++++++++++++++++++++++++++++++ and should contain your callsign

unsigned long buttonPressTime = 0;
bool isButtonPressed = false;

enum AppState                 // possible states of the state machine for defined program control
{
  WAIT_FOR_INPUT,
  BUTTON_PRESSED,
  PROCESS_INPUT,
};

TimeStampDCF77 time;
char TmpDCF77Num[3];

AppState currentState = WAIT_FOR_INPUT; // define start state

void setup() 
{
  PowerSaver(false);

  #ifdef DEBUG_SERIAL
    Serial.begin(115200);
    Serial.println("Debug Mode on\n");
  #endif

  #ifdef DEBUG_WATCHDOG
      wdt_enable(WDTO_8S); //WTO_64MS, WTO_128MS, WTO_256MS, WTO_512MS, WTO_1S - WTO_32S
  #endif
  
  DTMF.begin(2, 4, 5, 6, 7);        //External MT8870 DTMF decoder, triggered MC on rising edge of Pin 2

  RGBled.setupLEDs(9,10,11,COMMON_CATHODE);
  pinMode(buttonPin, INPUT);        // Application related pushbutton, capacitive, no debouncing necessary);
  pinMode(PTTpin, OUTPUT);          // PTT button of the VHF/UHF radio
  pinMode(interruptPin, INPUT);     // Declaring interruptPin as INPUT
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);
}

/*
void loop() 
{         
  if (digitalRead(buttonPin) == HIGH) 
  {
    RGBled.unsetLEDs(); // LEDs Switch off
  }

  if (currentState == PROCESS_INPUT) {
    currentState = WAIT_FOR_INPUT;
    processReceivedTone(ch);
  }
  
  #ifdef DEBUG_SERIAL
    //wdt_reset(); // Watchdog-Timer reset
  	Serial.println(millis());
  #endif
}
*/

void loop() {
  // implemented state machine
  switch (currentState) 
  {
    case WAIT_FOR_INPUT:
      if (digitalRead(buttonPin) == HIGH) 
      {
        if (!isButtonPressed) //Measure time if button is pressed.
        {
          buttonPressTime = millis();
          isButtonPressed = true;
        }
        // Check whether the button has been pressed for 1 second.
        if (isButtonPressed && (millis() - buttonPressTime >= 1000)) 
        {
          currentState = BUTTON_PRESSED;
        }
      } 
      else if (isButtonPressed) 
      {
        // Button was released before 1 second had passed.
        isButtonPressed = false;
        RGBled.unsetLEDs();
      }
      break;

    case BUTTON_PRESSED: // Action for long button press
      #ifdef DEBUG_SERIAL
        Serial.print(currentState);
        Serial.println(": BUTTON_PRESSED");
      #endif
      
      //Do something when the pushbutton is held down
      ;
      break;

    case PROCESS_INPUT:// Action for incoming transmission
      processReceivedTone(ch);
      currentState = WAIT_FOR_INPUT;
      break;
  }
  #ifdef DEBUG_SERIAL
  //wdt_reset(); // Watchdog-Timer reset
    //Serial.print(currentState);
    //Serial.println(": WAIT_FOR_INPUT");
  #endif
}




// Function for processing received tones
void processReceivedTone(char DTMFtone) 
{
  #ifdef DEBUG_SERIAL
    Serial.print(currentState);
    Serial.println(": PROCESS_INPUT");
  #endif
  
  if (DTMFtone == '*') // # is the terminating character of each DTMF string
  {
    inputString[inputIndex] = '\0'; // close valid DTMF string

    // program the microcontroller to do interesting things with their radio
    switch (atoi(inputString)) 
    {
      case 159:
        #ifdef DEBUG_SERIAL
          Serial.println("DTNF Code: 159");
        #endif
        RGBled.setLEDColor(GREEN);
        setPTT(true);
        delay(1500);
        spellString(&voice,callsign);
        voice.say(spIS);
        voice.say(spQ);
        voice.say(spR);
        voice.say(spV);
        setPTT(false);
        RGBled.unsetLEDs();
        break;

      case 110:
        #ifdef DEBUG_SERIAL
          Serial.println("DTNF Code: 110");
        #endif
        RGBled.setLEDColor(GREEN);
        setPTT(true);
        delay(1300);
        voice.say(spEMERGENCY);
        voice.say(spCHECK);
        setPTT(false);
        RGBled.unsetLEDs();
        RGBled.setLEDColor(RED);
        break;        

      case 0:
        #ifdef DEBUG_SERIAL
          Serial.println("DTNF Code: 0");
        #endif
        RGBled.setLEDColor(GREEN);
        setPTT(true);
        delay(1300);
        voice.say(spCHECK);
        setPTT(false);
        RGBled.unsetLEDs();
        RGBled.setLEDColor(YELLOW);
        break;

      case 9:
        #ifdef DEBUG_SERIAL
          Serial.println("DTNF Code: 9");
        #endif
        RGBled.setLEDColor(GREEN);
        setPTT(true);
        delay(1500);
        voice.say(spTHIS1);
        voice.say(spIS);
        spellString(&voice,callsign);
        setPTT(false);
        RGBled.unsetLEDs();
        break;

      case 5:
        #ifdef DEBUG_SERIAL
          Serial.println("DTNF Code: 5");
        #endif
        RGBled.setLEDColor(BLUE);
        if(receiveDCF77(bitArray,DCF77_STRING_SIZE)==ERROR_TIMEOUT)
        {
          setPTT(true);
          delay(1300);
          voice.say(spTIME);
          voice.say(spFAIL);
          setPTT(false);
          RGBled.setLEDColor(CYAN);
          break;
        }
        if(decodeDCF77(bitArray,DCF77_STRING_SIZE,&time)==ERROR_INVALID_VALUE)
        {
          setPTT(true);
          delay(1300);
          voice.say(spTIME);
          voice.say(spFAIL);
          setPTT(false);
          RGBled.setLEDColor(CYAN);
          break;
        }
        setPTT(true);
        delay(1300);
        voice.say(spCHECK);
        setPTT(false);
        RGBled.unsetLEDs();
        break;

      case 6:
        #ifdef DEBUG_SERIAL
          Serial.println("DTNF Code: 6");
        #endif
        RGBled.setLEDColor(BLUE);
        setPTT(true);
        delay(1300);
        voice.say(spIT);
        voice.say(spIS);
        intToCharArray(time.hour, TmpDCF77Num);
        spellString(&voice,TmpDCF77Num);
        voice.say(spPOINT);
        intToCharArray(time.minute, TmpDCF77Num);
        spellString(&voice,TmpDCF77Num);
        voice.say(spA1);
        voice.say(spCLOCK);
        setPTT(false);
        RGBled.unsetLEDs();
        break;        

      default:
      	RGBled.setLEDColor(GREEN);
      	delay(1500);
      	RGBled.unsetLEDs();
      	break;
    }

    inputIndex = 0; // after valid DTMF code the index is reset.
  } 
  else if (inputIndex < (sizeof(inputString)-1)) 
  {
    inputString[inputIndex] = DTMFtone;
    inputIndex++;
  } else 
  {
    inputIndex = 0; // reset index if more than 4 characters were received
    inputString[0] = '\0';
  }
}

// intnerrupt function to receive new DTMF characters via radio waves
void handleInterrupt() 
{
  ch = DTMF.read();
  currentState = PROCESS_INPUT;
}

// activate PTT, warning this key puts the station on the air
void setPTT(bool state) 
{
  digitalWrite(PTTpin, state ? HIGH : LOW);
}

void PowerSaver(bool status) 
{
  if(status == true)
  {
    power_spi_disable();
    power_usart0_disable();
    power_flashctrl_disable();
    power_wdtcc_disable();
    power_timer3_disable();
    power2_adc_disable();
    power2_ac0_disable();
    power2_ac1_disable();
    power2_dac_disable();
    power2_dap_disable();
  }
  if(status == false)
  {
    power_spi_enable();
    power_usart0_enable();
    power_flashctrl_enable();
    power_wdtcc_enable();
    power_timer3_enable();
    power2_adc_enable();
    power2_ac0_enable();
    power2_ac1_enable();
    power2_dac_enable();
    power2_dap_enable();
  }
}

//      VDTCR |= (1 << 7); // Set the Software-Reset-Bit
//      VDTCR &= ~(1 << 6); // Delete the software reset bit to trigger the reset
