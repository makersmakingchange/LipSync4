/* 
* File: LSOutput.h
* Firmware: LipSync
* Developed by: MakersMakingChange
* Version: v4.1rc (20 January 2025)
  License: GPL v3.0 or later

  Copyright (C) 2024 - 2025 Neil Squire Society
  This program is free software: you can redistribute it and/or modify it under the terms of
  the GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with this program.
  If not, see <http://www.gnu.org/licenses/>
*/

//Header definition
#ifndef _LSOUTPUT_H
#define _LSOUTPUT_H

#define OUTPUT_LED_NUM 4   // Total number of leds
#define OUTPUT_MONO_LED_NUM 3  // Number of monocolor leds
#define OUTPUT_RGB_LED_NUM 1   //Number of RGB leds


struct rgbStruct {
  int r;  // red value 0 to 255
  int g;  // green value
  int b;  // blue value
};

//Color structure
typedef struct {
  uint8_t colorNumber;
  String colorName;
  rgbStruct colorCode;
} colorStruct;

//Color properties
/*
const colorStruct colorProperty[]{
  { LED_CLR_NONE, "None", { 0, 0, 0 } },
  { LED_CLR_BLUE, "Blue", { 0, 0, 60 } },
  { LED_CLR_PURPLE, "Purple", { 50, 0, 128 } },
  { LED_CLR_MAGENTA, "Magenta", { 255, 0, 255 } },
  { LED_CLR_PINK, "Pink", { 60, 0, 50 } },
  { LED_CLR_RED, "Red", { 60, 0, 0 } },
  { LED_CLR_ORANGE, "Orange", { 60, 20, 0 } },
  { LED_CLR_YELLOW, "Yellow", { 50, 60, 0 } },
  { LED_CLR_GREEN, "Green", { 0, 60, 0 } },
  { LED_CLR_TEAL, "Teal", { 0, 128, 128 } },
  { LED_CLR_WHITE, "White", { 255, 255, 255 } }
};
*/

/*
//Color properties - IBM COLORBLIND FRIENDLY PALETTE
const colorStruct colorProperty[]{
  { LED_CLR_NONE,   "None",     {   0,   0,   0 } },
  { LED_CLR_BLUE,   "Blue",     { 100, 143, 255 } },
  { LED_CLR_PURPLE, "Purple",   {  50,   0, 128 } },
  { LED_CLR_RED,    "Red",      { 220,  38, 127 } },
  { LED_CLR_ORANGE, "Orange",   { 254,  97,   0 } },
  { LED_CLR_YELLOW, "Yellow",   { 255, 176,   0 } },
  { LED_CLR_WHITE,  "White",    { 255, 255, 255 } }
};
*/

//Color properties
const colorStruct colorProperty[]{
  { LED_CLR_NONE,   "None",     {   0,   0,   0 } },
  { LED_CLR_BLUE,   "Blue",     {   0,   0, 255 } },
  { LED_CLR_PURPLE, "Purple",   { 170,   0, 255 } },
  { LED_CLR_MAGENTA, "Magenta", { 255,   0, 255 } },
  { LED_CLR_PINK,   "Pink",     { 255, 127, 127 } },
  { LED_CLR_RED,    "Red",      { 230,   0,   0 } },
  { LED_CLR_ORANGE, "Orange",   { 254,  97,   0 } },
  { LED_CLR_YELLOW, "Yellow",   { 255, 255,   0 } },
  { LED_CLR_GREEN,  "Green",    { 0,   255,   0 } },
  { LED_CLR_TEAL,   "Teal",     { 0,  255,  255 } },  
  { LED_CLR_WHITE,  "White",    { 255, 255, 255 } }
};


class LSOutput {
private:
  int _ledBrightness = 255; //TODO Jake 2024-01-24. This currently doesn't do anything without Neopixels
  int _lightModeLevel = 1;  

public:
  LSOutput();
  void begin();
  void clearLedAll();
  void clearLed(int ledNumber);
  uint32_t getLedColor(int ledNumber);
  uint8_t getLedBrightness();
  void setLedBrightness(int ledBrightness);
  void setLedColor(int ledNumber, int ledColorNumber, int ledBrightness);
  void show();
  void setLightModeLevel(int lightModeLevel);
  int getLightModeLevel();

private:
  int _ledBrightness = 255;
  int _lightModeLevel = 1;  

};

//*********************************//
// Function   : LSOutput 
// 
// Description: LSOutput constructor
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
LSOutput::LSOutput() {
}

//*********************************//
// Function   : begin 
// 
// Description: Initialize LSOutput 
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSOutput::begin() {

  // Hub LEDs
  pinMode(CONF_LED_LEFT_PIN, OUTPUT);
  pinMode(CONF_LED_MIDDLE_PIN, OUTPUT);
  pinMode(CONF_LED_RIGHT_PIN, OUTPUT);

  // Microcontroller LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  //ledPixels.begin();
  clearLedAll();
}


//***CLEAR ALL RGB LED FUNCTION***//
//*********************************//
// Function   : clearLedAll 
// 
// Description: Turn off all LEDs
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSOutput::clearLedAll() {
  // Turn off all LEDs
  clearLed(CONF_LED_ALL);
}

//***CLEAR RGB LED FUNCTION***//
//*********************************//
// Function   : clearLed 
// 
// Description: Turn off a particular LED based on the input
// 
// Arguments :  int : ledNumber : Index of the LED to turn off
// 
// Return     : void
//*********************************//
void LSOutput::clearLed(int ledNumber) {
  setLedColor(ledNumber, LED_CLR_NONE, 0);
}


//***GET RGB LED COLOR FUNCTION***//
//*********************************//
// Function   : getLedColor 
// 
// Description: Get the color or a particular LED
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
uint32_t LSOutput::getLedColor(int ledNumber) {

  //uint32_t colorValue = ledPixels.getPixelColor(ledNumber-1);

  return 0;
  //return colorValue;
}


//***GET RGB LED BRIGHTNESS FUNCTION***//
//*********************************//
// Function   : getLedBrightness 
// 
// Description: Get the brightness of the LEDs
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
uint8_t LSOutput::getLedBrightness() {
  return _ledBrightness;
  //  return (ledPixels.getBrightness());
}



//***SET RGB LED BRIGHTNESS FUNCTION***//
//*********************************//
// Function   : setLedBrightness 
// 
// Description: Set the brightness of the LEDs
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSOutput::setLedBrightness(int ledBrightness) {
  _ledBrightness = ledBrightness;
  //show();

}

//*********************************//
// Function   : setLightModeLevel 
// 
// Description: Set light mode level
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSOutput::setLightModeLevel(int lightModeLevel)
{
  _lightModeLevel = lightModeLevel;
}

//*********************************//
// Function   : getLightModeLevel
// 
// Description: Return Light Mode Level
//              
// Arguments :  void
// 
// Return     : int _lightModeLevel : 
//*********************************//
int LSOutput::getLightModeLevel(void)
{
  return _lightModeLevel;
}


//***SET RGB LED COLOR FUNCTION***//
//*********************************//
// Function   : setLedColor 
// 
// Description: LSOutput constructor
// 
// Arguments :  int : ledNumber : index of LED (CONF_LED_LEFT, CONF_LED_RIGHT, CONF_LED_MIDDLE, CONF_LED_MICRO, CONF_LED_ALL)
//              int : ledColorNumber : color index
//              int : ledBrightness : 
// 
// Return     : void
//*********************************//
void LSOutput::setLedColor(int ledNumber, int ledColorNumber, int ledBrightness) {

  // Extract color components from color
  int r = colorProperty[ledColorNumber].colorCode.r;
  int g = colorProperty[ledColorNumber].colorCode.g;
  int b = colorProperty[ledColorNumber].colorCode.b;

  bool someLight = (r > 0 || g > 0 || b > 0);

  switch (ledNumber) {
    case CONF_LED_LEFT:
      {
        if (someLight && _lightModeLevel > 0) 
        {
          analogWrite(CONF_LED_LEFT_PIN, ledBrightness);
        } else {
          analogWrite(CONF_LED_LEFT_PIN, 0);
        }
        break;
      }
    case CONF_LED_MIDDLE:
      {
        if (someLight && _lightModeLevel > 0) 
        {
          analogWrite(CONF_LED_MIDDLE_PIN, ledBrightness);
        } else {
          analogWrite(CONF_LED_MIDDLE_PIN, 0);
        }
        break;
      }
    case CONF_LED_RIGHT:
      {
        if (someLight && _lightModeLevel > 0) 
        {
          analogWrite(CONF_LED_RIGHT_PIN, ledBrightness);
        } else {
          analogWrite(CONF_LED_RIGHT_PIN, 0);
        }
        break;
      }
    case CONF_LED_MICRO:
      {
        analogWrite(LED_RED, 255 - r);
        analogWrite(LED_GREEN, 255 - g);
        analogWrite(LED_BLUE, 255 - b);
        break;
      }

      case CONF_LED_ALL:
      {
       setLedColor(CONF_LED_LEFT, ledColorNumber, ledBrightness);
       setLedColor(CONF_LED_MIDDLE, ledColorNumber, ledBrightness);
       setLedColor(CONF_LED_RIGHT, ledColorNumber, ledBrightness);
       setLedColor(CONF_LED_MICRO, ledColorNumber, ledBrightness);
       break;
      }

  }  //end switch


} //end LSOutput::setLedColor

#endif
