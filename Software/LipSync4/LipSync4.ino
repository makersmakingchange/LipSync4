#include <Wire.h>
#include "LSTimer.h"
#include "LSUtils.h"
#include <ArduinoJson.h>
#include "LSConfig.h"
#include "LSUSB.h"
#include "LSBLE.h"
#include "LSCircularBuffer.h"
#include "LSInput.h"
#include "LSPressure.h"
#include "LSJoystick.h"
#include "LSOutput.h"
#include "LSMemory.h"

//Firmware : LipSync4
//Developed by : MakersMakingChange
//VERSION: Alpha 2 (06 April 2022) 
//Copyright Neil Squire Society 2022. 
//LICENSE: This work is licensed under the CC BY SA 4.0 License: http://creativecommons.org/licenses/by-sa/4.0 .


int comMode; //0 = None , 1 = USB , 2 = Wireless  
int debugMode;
bool ledActionEnabled = false;

//LED module variables
const ledActionStruct ledActionProperty[]{
    {CONF_ACTION_NOTHING,            1,LED_CLR_NONE,  LED_CLR_NONE,   LED_ACTION_NONE},
    {CONF_ACTION_LEFT_CLICK,         1,LED_CLR_NONE,  LED_CLR_MAGENTA,LED_ACTION_BLINK},
    {CONF_ACTION_RIGHT_CLICK,        3,LED_CLR_NONE,  LED_CLR_BLUE,   LED_ACTION_BLINK},
    {CONF_ACTION_DRAG,               1,LED_CLR_PURPLE,LED_CLR_MAGENTA,LED_ACTION_ON},
    {CONF_ACTION_SCROLL,             3,LED_CLR_PURPLE,LED_CLR_BLUE,   LED_ACTION_ON},
    {CONF_ACTION_CURSOR_CENTER,      2,LED_CLR_NONE,  LED_CLR_ORANGE, LED_ACTION_BLINK},
    {CONF_ACTION_CURSOR_CALIBRATION, 4,LED_CLR_NONE,  LED_CLR_ORANGE, LED_ACTION_BLINK},
    {CONF_ACTION_MIDDLE_CLICK,       2,LED_CLR_NONE,  LED_CLR_PURPLE, LED_ACTION_BLINK},
    {CONF_ACTION_DEC_SPEED,          1,LED_CLR_NONE,  LED_CLR_MAGENTA,LED_ACTION_NONE},
    {CONF_ACTION_INC_SPEED,          3,LED_CLR_NONE,  LED_CLR_BLUE,   LED_ACTION_NONE},
    {CONF_ACTION_CHANGE_MODE,        2,LED_CLR_NONE,  LED_CLR_BLUE,   LED_ACTION_BLINK}
};

ledStateStruct* ledCurrentState = new ledStateStruct;

int ledTimerId;

LSTimer<ledStateStruct> ledStateTimer;

//Input module variables

int buttonActionSize, switchActionSize;
inputStateStruct buttonState, switchState;

const inputActionStruct switchActionProperty[]{
    {CONF_ACTION_NOTHING,            INPUT_MAIN_STATE_NONE,       0,0},
    {CONF_ACTION_LEFT_CLICK,         INPUT_MAIN_STATE_S1_PRESSED, 0,1000},
    {CONF_ACTION_MIDDLE_CLICK,       INPUT_MAIN_STATE_S2_PRESSED, 0,1000},
    {CONF_ACTION_RIGHT_CLICK,        INPUT_MAIN_STATE_S3_PRESSED, 0,1000},
    {CONF_ACTION_DRAG,               INPUT_MAIN_STATE_S1_PRESSED, 1000,3000},
    {CONF_ACTION_CHANGE_MODE,        INPUT_MAIN_STATE_S2_PRESSED, 1000,3000},
    {CONF_ACTION_SCROLL,             INPUT_MAIN_STATE_S3_PRESSED, 1000,3000},
    {CONF_ACTION_CURSOR_CALIBRATION, INPUT_MAIN_STATE_S1_PRESSED, 3000,10000},
    {CONF_ACTION_NOTHING,            INPUT_MAIN_STATE_S2_PRESSED, 3000,10000},
    {CONF_ACTION_MIDDLE_CLICK,       INPUT_MAIN_STATE_S3_PRESSED, 3000,10000},
};

const inputActionStruct buttonActionProperty[]{
    {CONF_ACTION_NOTHING,            INPUT_MAIN_STATE_NONE,        0,0},
    {CONF_ACTION_DEC_SPEED,          INPUT_MAIN_STATE_S1_PRESSED,  0,1000},
    {CONF_ACTION_INC_SPEED,          INPUT_MAIN_STATE_S3_PRESSED,  0,1000},
    {CONF_ACTION_CURSOR_CENTER,      INPUT_MAIN_STATE_S2_PRESSED,  0,1000},
    {CONF_ACTION_CHANGE_MODE,        INPUT_MAIN_STATE_S2_PRESSED,  1000,3000},
    {CONF_ACTION_CURSOR_CALIBRATION, INPUT_MAIN_STATE_S13_PRESSED, 0,1000}
};

int inputButtonPinArray[] = { CONF_BUTTON1_PIN, CONF_BUTTON2_PIN, CONF_BUTTON3_PIN };
int inputSwitchPinArray[] = { CONF_SWITCH1_PIN, CONF_SWITCH2_PIN, CONF_SWITCH3_PIN };

//Pressure module variables

pressureStruct pressureValues = { 0.0, 0.0, 0.0 };

inputStateStruct sapActionState;

int sapActionSize;

const inputActionStruct sapActionProperty[]{
    {CONF_ACTION_NOTHING,            PRESS_SAP_MAIN_STATE_NONE,  0,0},
    {CONF_ACTION_LEFT_CLICK,         PRESS_SAP_MAIN_STATE_PUFF,  0,1000},
    {CONF_ACTION_RIGHT_CLICK,        PRESS_SAP_MAIN_STATE_SIP,   0,1000},
    {CONF_ACTION_DRAG,               PRESS_SAP_MAIN_STATE_PUFF,  1000,3000},
    {CONF_ACTION_SCROLL,             PRESS_SAP_MAIN_STATE_SIP,   1000,3000},
    {CONF_ACTION_CURSOR_CENTER,      PRESS_SAP_MAIN_STATE_PUFF,  3000,5000},
    {CONF_ACTION_MIDDLE_CLICK,       PRESS_SAP_MAIN_STATE_SIP,   3000,5000}
};

//Joystick module variables and structures

int xVal;
int yVal;

int calibTimerId[2];

LSTimer<int> calibTimer;

//Timer related variables

int pollTimerId[5];

LSTimer<void> pollTimer;

//General

int outputAction;
bool canOutputAction = true;

bool settingsEnabled = false;                        //Serial input settings command mode enabled or disabled

//Create instances of classes

LSMemory mem;

LSInput ib(inputButtonPinArray, CONF_BUTTON_NUMBER);
LSInput is(inputSwitchPinArray, CONF_SWITCH_NUMBER); //Starts an instance of the object

LSJoystick js;                                       //Starts an instance of the LSJoystick object

LSPressure ps;                                       //Starts an instance of the LSPressure object

LSOutput led;                                        //Starts an instance of the LSOutput LED object

LSUSBMouse mouse;                                    //Starts an instance of the USB mouse object
LSBLEMouse btmouse;                                  //Starts an instance of the BLE mouse object


//***MICROCONTROLLER AND PERIPHERAL CONFIGURATION***//
// Function   : setup 
// 
// Description: This function handles the initialization of variables, pins, methods, libraries. This function only runs once at powerup or reset.
// 
// Parameters :  void
// 
// Return     : void
//*********************************//
void setup()
{
  // Begin HID mouse 
  mouse.begin();
  btmouse.begin();

  Serial.begin(115200);
  //while (!TinyUSBDevice.mounted())
  //while (!Serial) { delay(10); }

  delay(1000);                                                  //TO BE REMOVED 
  
  initMemory();                                                 //Initialize Memory 

  initLed();                                                    //Initialize LED Feedback 

  initSipAndPuff();                                             //Initialize Sip And Puff 

  initInput();                                                  //Initialize input buttons and input switches 

  initJoystick();                                               //Initialize Joystick 

  initCommunicationMode();                                      //Initialize Communication Mode

  initDebug();                                                  //Initialize Debug Mode operation 

  startupFeedback();                                            //Startup IBM LED Feedback 

  //Configure poll timer to perform each feature as a separate loop
  pollTimerId[0] = pollTimer.setInterval(CONF_JOYSTICK_POLL_RATE, 0, joystickLoop);
  pollTimerId[1] = pollTimer.setInterval(CONF_PRESSURE_POLL_RATE, 0, pressureLoop);
  pollTimerId[2] = pollTimer.setInterval(CONF_INPUT_POLL_RATE, 0, inputLoop);
  pollTimerId[3] = pollTimer.setInterval(CONF_BLE_FEEDBACK_POLL_RATE, 0, bleFeedbackLoop);
  pollTimerId[4] = pollTimer.setInterval(CONF_DEBUG_POLL_RATE, 0, debugLoop);

  
  enablePoll(false);                              //Enable it when the led IBM effect is complete 

} //end setup


//***START OF MAIN LOOP***//
// Function   : loop 
// 
// Description: This function loops consecutively and responses to changes.
// 
// Parameters :  void
// 
// Return     : void
//*********************************//
void loop()
{
  ledStateTimer.run();
  calibTimer.run();
  pollTimer.run();
  settingsEnabled=serialSettings(settingsEnabled); //Check to see if setting option is enabled in Lipsync
}

//***ENABLE POLL FUNCTION***//
// Function   : enablePoll 
// 
// Description: This function enables polling by enabling poll timers. It can be used to disable main features 
//              during initialization.
//
// Parameters : isEnabled : bool : enable polling if it's True
// 
// Return     : void 
//****************************************//
void enablePoll(bool isEnabled){
  if(isEnabled){
    getDebugMode(false,false);
    pollTimer.enable(1); 
    pollTimer.enable(2);    
    pollTimer.enable(3);  
    //setLedDefault();
  } else {
    pollTimer.disable(0);    
    pollTimer.disable(1);
    pollTimer.disable(2);
    pollTimer.disable(3);  
    pollTimer.disable(4); 
  }
}

//*********************************//
// Memory Functions
//*********************************//

//***INITIALIZE MEMORY FUNCTION***//
// Function   : initMemory 
// 
// Description: This function initializes flash memory to store settings 
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initMemory()
{
  mem.begin();                                                      //Begin memory 
  //mem.format();                                                   //DON'T UNCOMMENT
  mem.initialize(CONF_SETTINGS_FILE, CONF_SETTINGS_JSON);           //Initialize flash memory to store settings 
}

//***RESET MEMORY FUNCTION***//
// Function   : resetMemory 
// 
// Description: This function formats and removes existing text files in flash memory.
//              It initializes flash memory to store settings after formatting.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void resetMemory()
{
  mem.format();                                                    //Format and remove existing text files in flash memory 
  mem.initialize(CONF_SETTINGS_FILE, CONF_SETTINGS_JSON);          //Initialize flash memory to store settings 
}


//*********************************//
// Communication Mode Functions
//*********************************//

//***INITIALIZE COMMUNICATION FUNCTION***//
// Function   : initCommunicationMode 
// 
// Description: This function initializes communication mode or configures communication mode
//              based on stored settings in the flash memory (0 = None , 1 = USB , 2 = Wireless)
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initCommunicationMode()
{
  comMode = getCommunicationMode(false,false);
}


//*********************************//
// Input Functions
//*********************************//


//***INITIALIZE INPUTS FUNCTION***//
// Function   : initInput 
// 
// Description: This function initializes inputs including input buttons and input switches.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initInput()
{

  ib.begin();                                                                     //Begin input buttons    
  is.begin();                                                                     //Begin input switches  
  buttonActionSize = sizeof(buttonActionProperty) / sizeof(inputActionStruct);    //Size of total available input button actions
  switchActionSize = sizeof(switchActionProperty) / sizeof(inputActionStruct);    //Size of total available input switch actions
}

//***INPUT LOOP FUNCTION***//
// Function   : inputLoop 
// 
// Description: This function handles input button and input switch actions.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void inputLoop()
{
  //Request new values
  ib.update();
  is.update(); 

  //Get the last state change
  buttonState = ib.getInputState();
  switchState = is.getInputState();

  //Evaluate Output Actions
  evaluateOutputAction(buttonState, buttonActionSize,buttonActionProperty);
  evaluateOutputAction(switchState, switchActionSize,switchActionProperty);
}

//*********************************//
// Sip and Puff Functions
//*********************************//


//***INITIALIZE SIP AND PUFF FUNCTION***//
// Function   : initSipAndPuff 
// 
// Description: This function initializes sip and puff as inputs.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initSipAndPuff()
{
  ps.begin();                                                                     //Begin sip and puff 
  getPressureMode(true,false);                                                    //Get the pressure mode stored in flash memory ( 1 = Absolute , 2 = Differential )
  getPressureThreshold(true,false);                                               //Get sip and puff pressure thresholds stored in flash memory 
  sapActionSize = sizeof(sapActionProperty) / sizeof(inputActionStruct);          //Size of total available sip and puff actions
}


//***PRESSURE LOOP FUNCTION***//
// Function   : inputLoop 
// 
// Description: This function handles pressure polling, sip and puff state evaluation.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void pressureLoop()
{

  ps.update(); //Request new pressure difference from sensor and push it to array

  pressureValues = ps.getAllPressure(); //Read the pressure object (can be last value from array, average or other algorithms)

  //Get the last state change
  sapActionState = ps.getState();

  //Output action logic
  evaluateOutputAction(sapActionState, sapActionSize,sapActionProperty);
}

//***RELEASE OUTPUT FUNCTION***//
// Function   : releaseOutputAction 
// 
// Description: This function handles release of mouse hold actions.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void releaseOutputAction()
{
  setLedDefault();                                                            //Set default led feedback 
  //Release left click if it's in drag mode and left mouse button is pressed.
  if (outputAction == CONF_ACTION_DRAG && (mouse.isPressed(MOUSE_LEFT) || btmouse.isPressed(MOUSE_LEFT)))
  {
    mouse.release(MOUSE_LEFT);
    btmouse.release(MOUSE_LEFT);
  }
  //Set new state of current output action 
  outputAction = CONF_ACTION_NOTHING;
  canOutputAction = true;
}

//***EVALUATE OUTPUT ACTION FUNCTION***//
// Function   : evaluateOutputAction 
// 
// Description: This function evaluates and performs output action
//
// Parameters : actionState : inputStateStruct : Current input state 
//              actionSize : int : size of available actions
//              actionProperty : const inputActionStruct : array of all possible actions
// 
// Return     : void 
//****************************************//
void evaluateOutputAction(inputStateStruct actionState, int actionSize,const inputActionStruct actionProperty[])
{
  bool canEvaluateAction = true;
  //Output action logic
  int tempActionIndex = 0;
  //Handle input action when it's in hold state
  if ((
    actionState.secondaryState == INPUT_SEC_STATE_RELEASED) &&
    (outputAction == CONF_ACTION_SCROLL ||
      outputAction == CONF_ACTION_DRAG))
  {
    //Set new state of current output action 
    releaseOutputAction();
    canEvaluateAction = false;
  }

  //Loop over all possible outputs
  for (int actionIndex = 0; actionIndex < actionSize && canEvaluateAction && canOutputAction; actionIndex++)
  {
    //Detected input release in defined time limits. Perform output action based on action index
    if (actionState.mainState == actionProperty[actionIndex].inputActionState &&
      actionState.secondaryState == INPUT_SEC_STATE_RELEASED &&
      actionState.elapsedTime >= actionProperty[actionIndex].inputActionStartTime &&
      actionState.elapsedTime < actionProperty[actionIndex].inputActionEndTime)
    {
      //Get action index 
      tempActionIndex = actionProperty[actionIndex].inputActionNumber;

      //Set Led state 
      setLedState(ledActionProperty[tempActionIndex].ledEndAction, 
      ledActionProperty[tempActionIndex].ledEndColor, 
      ledActionProperty[tempActionIndex].ledNumber, 
      1, 
      CONF_LED_REACTION_TIME, 
      CONF_LED_BRIGHTNESS);
      outputAction = tempActionIndex;

      //Perform output action and led action 
      performOutputAction(tempActionIndex);

      break;
    } //Detected input start in defined time limits. Perform led action based on action index
    else if (actionState.mainState == actionProperty[actionIndex].inputActionState &&
      actionState.secondaryState == INPUT_SEC_STATE_STARTED &&
      actionState.elapsedTime >= actionProperty[actionIndex].inputActionStartTime &&
      actionState.elapsedTime < actionProperty[actionIndex].inputActionEndTime)
    {
      //Get action index 
      tempActionIndex = actionProperty[actionIndex].inputActionNumber; 

      //Set Led state 
      setLedState(ledActionProperty[tempActionIndex].ledEndAction, 
      ledActionProperty[tempActionIndex].ledStartColor, 
      ledActionProperty[tempActionIndex].ledNumber, 
      0, 
      0, 
      CONF_LED_BRIGHTNESS);
      //Perform led action 
      performLedAction(ledCurrentState);

      break;
    }
  }  
}

//***PERFORM OUTPUT ACTION FUNCTION***//
// Function   : performOutputAction 
// 
// Description: This function performs output action
//
// Parameters : action : int : action index number 
// 
// Return     : void 
//****************************************//
void performOutputAction(int action)
{
  //Perform led action 
  performLedAction(ledCurrentState);
  switch (action)
  {
    case CONF_ACTION_NOTHING:
    {
      break;
    }
    case CONF_ACTION_LEFT_CLICK:
    {
      cursorLeftClick();
      break;
    }
    case CONF_ACTION_RIGHT_CLICK:
    {
      cursorRightClick();
      break;
    }
    case CONF_ACTION_DRAG:
    {
      cursorDrag();
      break;
    }
    case CONF_ACTION_SCROLL:
    {
      cursorScroll(); //Enter Scroll mode
      break;
    }
    case CONF_ACTION_CURSOR_CENTER:
    {
      //Perform cursor center
      setJoystickInitialization(true,false);
      break;
    }
    case CONF_ACTION_CURSOR_CALIBRATION:
    {
      setJoystickCalibration(true,false);
      break;
    }
    case CONF_ACTION_MIDDLE_CLICK:
    {
      //Perform cursor middle click
      cursorMiddleClick();
      break;
    }
    case CONF_ACTION_DEC_SPEED:
    {
      //Decrease cursor speed
      decreaseJoystickSpeed(true,false);
      break;
    }
    case CONF_ACTION_INC_SPEED:
    {
      //Increase cursor speed
      increaseJoystickSpeed(true,false);
      break;
    }
    case CONF_ACTION_CHANGE_MODE:
    {
      //Change communication mode
      toggleCommunicationMode(true,false);
      break;
    }
  }
  if(action==CONF_ACTION_DRAG || action==CONF_ACTION_SCROLL){
    canOutputAction = false;
  }
  else {
    outputAction=CONF_ACTION_NOTHING;
    canOutputAction = true;
  }
}

//***CURSOR LEFT CLICK FUNCTION***//
// Function   : cursorLeftClick 
// 
// Description: This function performs cursor left click action.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void cursorLeftClick(void)
{
  //Serial.println("Left Click");
  if (comMode == CONF_COM_MODE_USB)
  {
    mouse.click(MOUSE_LEFT);
  }
  else if (comMode == CONF_COM_MODE_BLE)
  {
    btmouse.click(MOUSE_LEFT);
  }
}

//***CURSOR RIGHT CLICK FUNCTION***//
// Function   : cursorRightClick 
// 
// Description: This function performs cursor right click action.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void cursorRightClick(void)
{
  //Serial.println("Right Click");
  if (comMode == CONF_COM_MODE_USB)
  {
    mouse.click(MOUSE_RIGHT);
  }
  else if (comMode == CONF_COM_MODE_BLE)
  {
    btmouse.click(MOUSE_RIGHT);
  }
}

//***CURSOR MIDDLE CLICK FUNCTION***//
// Function   : cursorMiddleClick 
// 
// Description: This function performs cursor middle click action.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void cursorMiddleClick(void)
{
  //Serial.println("Middle Click");
  if (comMode == CONF_COM_MODE_USB)
  {
    mouse.click(MOUSE_MIDDLE);
  }
  else if (comMode == CONF_COM_MODE_BLE)
  {
    btmouse.click(MOUSE_MIDDLE);
  }
}

//***DRAG FUNCTION***//
// Function   : cursorDrag 
// 
// Description: This function performs cursor drag action.
//
// Parameters : void
// 
// Return     : void 
//********************//
void cursorDrag(void)
{
  //Serial.println("Drag");
  if (comMode == CONF_COM_MODE_USB)
  {
    mouse.press(MOUSE_LEFT);
  }
  else if (comMode == CONF_COM_MODE_BLE)
  {
    btmouse.press(MOUSE_LEFT);
  }
}
//***CURSOR SCROLL FUNCTION***//
// Function   : cursorScroll 
// 
// Description: This function is an operating mode that enables scrolling action.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void cursorScroll(void)
{
  //Serial.println("Scroll");
}


//*********************************//
// Joystick Functions
//*********************************//

//***INITIALIZE JOYSTICK FUNCTION***//
// Function   : initJoystick 
// 
// Description: This function initializes joystick as input.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initJoystick()
{
  js.begin();                                                               //Begin joystick 
  js.setMagnetDirection(JOY_DIRECTION_DEFAULT,JOY_DIRECTION_INVERSE);       //Set x and y magnet direction 
  getJoystickDeadZone(true,false);                                          //Get joystick deadzone stored in flash memory 
  getJoystickSpeed(true,false);                                             //Get joystick cursor speed stored in flash memory 
  setJoystickInitialization(true,false);                                    //Perform joystick center initialization
  getJoystickCalibration(true,false);                                       //Get joystick calibration points stored in flash memory 
}

//***PERFORM JOYSTICK CENTER FUNCTION***//
// Function   : performJoystickCenter 
// 
// Description: This function performs joystick center point initialization.
//
// Parameters : * args : int : pointer of step number 
// 
// Return     : void 
//****************************************//
void performJoystickCenter(int* args)
{
  int stepNumber = (int)args;
  unsigned long readingDuration = CONF_JOY_INIT_READING_DELAY*CONF_JOY_INIT_READING_NUMBER; //Duration of the center point reading s
  unsigned long currentReadingStart = CONF_JOY_INIT_START_TIME;                             //Time until start of current reading.
  unsigned long nextStepStart = currentReadingStart+readingDuration;                        //Time until start of next step.
  pointFloatType centerPoint;

  if (stepNumber == 0)  //STEP 0: Joystick Compensation Center Point
  {
    //Start timer to get 5 reading every 100ms
    calibTimerId[1] = calibTimer.setTimer(CONF_JOY_INIT_READING_DELAY, currentReadingStart, CONF_JOY_INIT_READING_NUMBER, performJoystickCenterStep, (int *)stepNumber);
    ++stepNumber;                                  
    ///Start exit step                                                     
    calibTimerId[0] = calibTimer.setTimeout(nextStepStart, performJoystickCenter, (int *)stepNumber);      
  } else
  {
    js.evaluateInputCenter();                                                      //Evaluate the center point using values in the buffer 
    js.setMinimumRadius();                                                         //Update minimum radius of operation            
    centerPoint = js.getInputCenter();                                             //Get the new center for API output  
    printResponseFloatPoint(true,true,true,0,"IN,1",true,centerPoint);
    calibTimer.deleteTimer(0);                                                     //Delete timer
    canOutputAction = true;
  }

}

//***PERFORM JOYSTICK CENTER STEP FUNCTION***//
// Function   : performJoystickCenterStep 
// 
// Description: This function performs the actual joystick center point initialization step.
//
// Parameters : * args : int : pointer of step number 
// 
// Return     : void 
//****************************************//
void performJoystickCenterStep(int* args)
{
  //Turn on and set the second led to orange to indicate start of the process 
  if(calibTimer.getNumRuns(1)==1 && ledActionEnabled){                                                                    //Turn Led's ON when timer is running for first time
    setLedState(LED_ACTION_ON, LED_CLR_ORANGE, 2, 0, 0,CONF_LED_BRIGHTNESS);
    performLedAction(ledCurrentState);   
  }
  //Push new center values to be evaluated at the end of the process 
  js.updateInputCenterBuffer();

  //Turn off the second led to orange to indicate end of the process 
  if(calibTimer.getNumRuns(1)==CONF_JOY_INIT_READING_NUMBER && ledActionEnabled){                                        //Turn Led's OFF when timer is running for last time
    setLedState(LED_ACTION_OFF, LED_CLR_NONE, 2, 0, 0,CONF_LED_BRIGHTNESS);                           
    performLedAction(ledCurrentState);      
  }
}

//***PERFORM JOYSTICK CALIBRATION FUNCTION***//
// Function   : performJoystickCalibration 
// 
// Description: This function performs joystick maximum point calibration in a recursive fashion.
//
// Parameters : * args : int : pointer of step number 
// 
// Return     : void 
//****************************************//
void performJoystickCalibration(int* args)
{
  int stepNumber = (int)args;
  unsigned long readingDuration = CONF_JOY_CALIB_READING_DELAY*CONF_JOY_CALIB_READING_NUMBER; //Duration of the max corner reading 
  unsigned long currentReadingStart = CONF_JOY_CALIB_BLINK_TIME*((stepNumber*2)+1);           //Time until start of current reading.
  unsigned long nextStepStart = currentReadingStart+readingDuration+CONF_JOY_CALIB_STEP_DELAY; //Time until start of next reading.

  if (stepNumber == 0)  //STEP 0
  {
    ++stepNumber;
    calibTimerId[0] = calibTimer.setTimeout(nextStepStart, performJoystickCalibration, (int *)stepNumber);      // Start next step
  }
  else if (stepNumber < 5) //STEP 1-4: Joystick Calibration Corner Points 
  {
    setLedState(LED_ACTION_BLINK, LED_CLR_PURPLE, 4, stepNumber, CONF_JOY_CALIB_BLINK_TIME,CONF_LED_BRIGHTNESS);    
    performLedAction(ledCurrentState);                                                                  // LED Feedback to show start of performJoystickCalibrationStep
    js.zeroInputMax(stepNumber);                                                                        //Clear the existing calibration value 
    calibTimerId[1] = calibTimer.setTimer(CONF_JOY_CALIB_READING_DELAY, currentReadingStart, CONF_JOY_CALIB_READING_NUMBER, performJoystickCalibrationStep, (int *)stepNumber);
    ++stepNumber;                                                                                       //Set LED's feedback to show step is already started and get the max reading for the quadrant/step
    calibTimerId[0] = calibTimer.setTimeout(nextStepStart, performJoystickCalibration, (int *)stepNumber);      //Start next step
  } 
  else if (stepNumber == 5)
  {
    setLedState(LED_ACTION_NONE, LED_CLR_NONE, 4, 0, 0,CONF_LED_BRIGHTNESS_LOW);                        //Turn off Led's
    performLedAction(ledCurrentState);
    setJoystickInitialization(false, false); 
    js.setMinimumRadius();                                                                              //Update the minimum cursor operating radius 
    canOutputAction = true;
  }

}
//***PERFORM JOYSTICK CALIBRATION STEP FUNCTION***//
// Function   : performJoystickCalibrationStep 
// 
// Description: This function performs the actual joystick maximum point calibration step in a recursive fashion.
//
// Parameters : * args : int : pointer of step number 
// 
// Return     : void 
//****************************************//
void performJoystickCalibrationStep(int* args)
{
  int stepNumber = (int)args;
  String stepCommand = "CA"+String(stepNumber);                                                       //Command to write new calibration point to Flash memory 
  pointFloatType maxPoint;

  //Turn on and set the all leds to orange to indicate start of the process 
  if(calibTimer.getNumRuns(0)==1){                                                                    //Turn Led's ON when timer is running for first time
    setLedState(LED_ACTION_ON, LED_CLR_ORANGE, 4, 0, 0,CONF_LED_BRIGHTNESS);
    performLedAction(ledCurrentState);   
  }
  
  maxPoint=js.getInputMax(stepNumber);                                                                //Get maximum x and y for the step number 

  //Turn off all the leds to orange to indicate end of the process 
  if(calibTimer.getNumRuns(0)==CONF_JOY_CALIB_READING_NUMBER){                                        //Turn Led's OFF when timer is running for last time
    mem.writePoint(CONF_SETTINGS_FILE,stepCommand,maxPoint);                                          //Store the point in Flash Memory 
    setLedState(LED_ACTION_OFF, LED_CLR_NONE, 4, 0, 0,CONF_LED_BRIGHTNESS);                           
    performLedAction(ledCurrentState);      
    printResponseFloatPoint(true,true,true,0,"CA,1",true,maxPoint);
  }
}

//***JOYSTICK LOOP FUNCTION***//
// Function   : joystickLoop 
// 
// Description: This function handles joystick move and scroll actions.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void joystickLoop()
{

  js.update();                                                        //Request new values

  pointIntType joyOutPoint = js.getXYOut();                           //Read the filtered values

  performJoystick(joyOutPoint);                                       //Perform joystick move action
}

//***PERFORM JOYSTICK FUNCTION***//
// Function   : performJoystick 
// 
// Description: This function performs joystick move and scroll actions.
//
// Parameters : inputPoint : pointIntType : The output cursor x and y
// 
// Return     : void 
//****************************************//
void performJoystick(pointIntType inputPoint)
{
  //0 = None , 1 = USB , 2 = Wireless  
  if (comMode == CONF_COM_MODE_USB)
  {
    (outputAction == CONF_ACTION_SCROLL) ? mouse.scroll(round(inputPoint.y / 5)) : mouse.move(inputPoint.x, -inputPoint.y);
  }
  else if (comMode == CONF_COM_MODE_BLE)
  {
    (outputAction == CONF_ACTION_SCROLL) ? btmouse.scroll(round(inputPoint.y / 5)) : btmouse.move(inputPoint.x, -inputPoint.y);
  }
}


//*********************************//
// Debug Functions
//*********************************//

//***INITIALIZE DEBUG FUNCTION***//
// Function   : initDebug 
// 
// Description: This function initializes debug mode and debug state.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initDebug()
{
  debugMode = getDebugMode(false, false);                                   //Get debug mode number stored in flash memory 
  setDebugState(debugMode);                                                 //Set debug operation state based on the debug mode
}

//***DEBUG LOOP FUNCTION***//
// Function   : debugLoop 
// 
// Description: This function outputs debug data based on debug mode.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void debugLoop(){
  if(debugMode==CONF_DEBUG_MODE_JOYSTICK){
    js.update(); //Request new values from joystick class
    pointFloatType debugJoystickArray[3];
    debugJoystickArray[0] = js.getXYRaw();                                                          //Read the raw values
    debugJoystickArray[1] = {(float)js.getXYIn().x,(float)js.getXYIn().y};                          //Read the filtered values
    debugJoystickArray[2] = {(float)js.getXYOut().x,(float)js.getXYOut().y};                        //Read the output values
    printResponseFloatPointArray(true,true,true,0,"DEBUG,1",true,"", 3, ',', debugJoystickArray);    
  }
  else if(debugMode==CONF_DEBUG_MODE_PRESSURE){  //Use update values from pressureLoop()
    //ps.update(); //Request new pressure difference from sensor and push it to array
    float debugPressureArray[3];
    debugPressureArray[0] = ps.getMainPressure();  //Read the main pressure 
    debugPressureArray[1] = ps.getRefPressure();   //Read the ref pressure
    debugPressureArray[2] = ps.getDiffPressure();  //Read the diff pressure
    printResponseFloatArray(true,true,true,0,"DEBUG,2",true,"", 3, ',', debugPressureArray);    
  }
  else if(debugMode==CONF_DEBUG_MODE_BUTTON){  
    int debugButtonArray[3];
    debugButtonArray[0] = buttonState.mainState;             //Read the main state 
    debugButtonArray[1] = buttonState.secondaryState;        //Read the secondary state
    debugButtonArray[2] = (int) buttonState.elapsedTime;     //Read the Elapsed Time     
    printResponseIntArray(true,true,true,0,"DEBUG,3",true,"", 3, ',', debugButtonArray);    
  }
  else if(debugMode==CONF_DEBUG_MODE_SWITCH){  
    int debugSwitchArray[3];
    debugSwitchArray[0] = switchState.mainState;             //Read the main state 
    debugSwitchArray[1] = switchState.secondaryState;        //Read the secondary state 
    debugSwitchArray[2] = (int) switchState.elapsedTime;     //Read the Elapsed Time    
    printResponseIntArray(true,true,true,0,"DEBUG,4",true,"", 3, ',', debugSwitchArray);    
  }
  else if(debugMode==CONF_DEBUG_MODE_SAP){  
    int debugSapArray[3];
    debugSapArray[0] = sapActionState.mainState;             //Read the main state 
    debugSapArray[1] = sapActionState.secondaryState;        //Read the secondary state 
    debugSapArray[2] = (int) sapActionState.elapsedTime;     //Read the Elapsed Time    
    printResponseIntArray(true,true,true,0,"DEBUG,5",true,"", 3, ',', debugSapArray);    
  }
}

//***SET DEBUG STATE FUNCTION***//
// Function   : setDebugState 
// 
// Description: This function configures the state of operation based on the debug mode
//
// Parameters : inputDebugMode : int : The current debug mode
// 
// Return     : void 
//****************************************//
void setDebugState(int inputDebugMode) {
  if (inputDebugMode==CONF_DEBUG_MODE_NONE) {
    pollTimer.enable(0);                      //Enable joystick data polling 
    pollTimer.disable(4);                     //Disable debug data polling 
  } 
  else if (inputDebugMode==CONF_DEBUG_MODE_JOYSTICK) {
    pollTimer.disable(0);                     //Disable joystick data polling 
    pollTimer.enable(4);                      //Enable debug data polling 
  }
  else {
    pollTimer.enable(4);                      //Enable debug data polling 
  }
}


//*********************************//
// LED Functions
//*********************************//

//***INITIALIZE LED FUNCTION***//
// Function   : initLed 
// 
// Description: This function initializes LED output as feedback method.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void initLed()
{
  led.begin();
  ledTimerId=0;
  *ledCurrentState = { 0, 0, 0, 0, 0, 0 };
}

//***STARTUP INITIALIZATION LED FUNCTION***//
// Function   : startupFeedback 
// 
// Description: This function performs the startup initialization feedback.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void startupFeedback()
{
  setLedState(LED_ACTION_BLINK, 1, 4, 4, CONF_LED_STARTUP_COLOR_TIME, CONF_LED_BRIGHTNESS);
  ledTimerId = ledStateTimer.setTimeout(ledCurrentState->ledBlinkTime, ledIBMEffect, ledCurrentState);

}

//***SET LED STATE FUNCTION***//
// Function   : setLedState 
// 
// Description: This function sets LED states based on led number, blink number, color number, blink delay , and brightness.
//
// Parameters : ledAction      : int : led action ( NOTHING , ON , OFF , BLINK )
//              ledColorNumber : int : led color number 
//              ledNumber      : int : led number ( 1, 2, 3, 4 or all )
//              ledBlinkNumber : int : led blink number 
//              ledBlinkTime : unsigned long : led blink delay time 
//              ledBrightness : int : led brightness
// 
// Return     : void 
//****************************************//
void setLedState(int ledAction, int ledColorNumber, int ledNumber, int ledBlinkNumber, unsigned long ledBlinkTime, int ledBrightness)
{ //Set led state after output action is performed
  if (ledNumber <= OUTPUT_RGB_LED_NUM + 1)
  {
    
    ledCurrentState->ledAction = ledAction;
    ledCurrentState->ledColorNumber = ledColorNumber;
    ledCurrentState->ledNumber = ledNumber;
    ledCurrentState->ledBlinkNumber = ledBlinkNumber;
    ledCurrentState->ledBlinkTime = ledBlinkTime;
    ledCurrentState->ledBrightness = ledBrightness;
  }
}



//***LED IBM EFFECT FUNCTION***//
// Function   : ledIBMEffect 
// 
// Description: This function performs the IBM LED effect based on step number.
//
// Parameters : args : ledStateStruct* : It includes step number.
// 
// Return     : void 
//****************************************//
void ledIBMEffect(ledStateStruct* args)
{
  if (args->ledColorNumber == 0)
  {
    ledActionEnabled = true;
    enablePoll(true);
  }
  else if (args->ledColorNumber < 7)
  {
    led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
    setLedState(args->ledAction, (args->ledColorNumber)+1, args->ledNumber, args->ledBlinkNumber, (args->ledBlinkTime),args->ledBrightness);
    ledTimerId = ledStateTimer.setTimeout(ledCurrentState->ledBlinkTime, ledIBMEffect, ledCurrentState);
  } 
  else if (args->ledColorNumber == 7)
  {
    setLedState(LED_ACTION_OFF, 0, args->ledNumber, args->ledBlinkNumber, (args->ledBlinkTime),args->ledBrightness);
    performLedAction(ledCurrentState);
    ledTimerId = ledStateTimer.setTimeout(ledCurrentState->ledBlinkTime, ledIBMEffect, ledCurrentState);
  }

}

//***LED BLINK EFFECT FUNCTION***//
// Function   : ledBlinkEffect 
// 
// Description: This function performs the blink LED effect based on step number, led number passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes step number, led number.
// 
// Return     : void 
//****************************************//
void ledBlinkEffect(ledStateStruct* args){
  if(ledStateTimer.getNumRuns(0) % 2){
     led.setLedColor(args->ledNumber, 0, args->ledBrightness);
  } 
  else{
    led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
  }

  if(ledStateTimer.getNumRuns(0)==((args->ledBlinkNumber)*2)+1){
    
     setLedState(LED_ACTION_NONE, LED_CLR_NONE, 0, 0, 0,CONF_LED_BRIGHTNESS_LOW);
     
  }   
}

//***TURN ALL LEDS OFF FUNCTION***//
// Function   : turnLedAllOff 
// 
// Description: This function turns off all the LEDs.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void turnLedAllOff()
{
  led.clearLedAll();
}

//***TURN LED OFF FUNCTION***//
// Function   : turnLedOff 
// 
// Description: This function turns off the specified LED based on led number passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number.
// 
// Return     : void 
//****************************************//
void turnLedOff(ledStateStruct* args)
{
  led.clearLed(args->ledNumber);
}

//***TURN LED ON FUNCTION***//
// Function   : turnLedOff 
// 
// Description: This function turns on the specified LED based on led number passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number.
// 
// Return     : void 
//****************************************//
void turnLedOn(ledStateStruct* args)
{
  led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
}


//***BLINK LED FUNCTION***//
// Function   : blinkLed 
// 
// Description: This function blinks the specified LED based on led number and number of blinks passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number and number of blinks.
// 
// Return     : void 
//****************************************//
void blinkLed(ledStateStruct* args)
{
  ledTimerId = ledStateTimer.setTimer(args->ledBlinkTime, 0, ((args->ledBlinkNumber)*2)+1, ledBlinkEffect, ledCurrentState);
}

//***SET DEFAULT LED EFFECT FUNCTION***//
// Function   : setLedDefault 
// 
// Description: This function sets the LED to default operating color based on led number and color passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number and color.
// 
// Return     : void 
//****************************************//
void setLedDefault(){
  //Clear if it's in USB MODE
  if (comMode == CONF_COM_MODE_USB)
  {
    led.clearLedAll();
  }
  else if (comMode == CONF_COM_MODE_BLE && btmouse.isConnected())
  { //Clear and set second LED to blue if it's in BLE MODE
    led.clearLedAll();
    led.setLedColor(2, LED_CLR_BLUE, CONF_LED_BRIGHTNESS);
  }
}

//***BLE FEEDBACK LOOP FUNCTION***//
// Function   : bleFeedbackLoop 
// 
// Description: This function performs the default LED effect to indicate the device is in BLE mode.
//
// Parameters : void
// 
// Return     : void 
//****************************************//
void bleFeedbackLoop()
{
  //Set second LED to blue if it's in BLE MODE and connected 
  if (comMode == CONF_COM_MODE_BLE && btmouse.isConnected())
  {
    led.setLedColor(2, LED_CLR_BLUE, CONF_LED_BRIGHTNESS);
  } //Clear second LED if it's in BLE MODE, but it's NOT connected 
  else if (comMode == CONF_COM_MODE_BLE && btmouse.isConnected()==false)
  {
    led.clearLed(2);
  }
}

//***PERFORM LED ACTION FUNCTION***//
// Function   : performLedAction 
// 
// Description: This function performs LED actions based on led number, blink number, color number, blink delay , and brightness.
//
// Parameters : args : ledStateStruct* : It includes led number, blink number, color number, blink delay , and brightness.
// 
// Return     : void 
//****************************************//
void performLedAction(ledStateStruct* args)
{
  switch (args->ledAction)                                            //Get led action 
  {
    case LED_ACTION_NONE:
    {
      setLedDefault();
      break;
    }
    case LED_ACTION_OFF:
    {
      turnLedOff(args);
      break;
    }
    case LED_ACTION_ON:
    {
      turnLedOn(args);
      break;
    }
    case LED_ACTION_BLINK:
    {
      blinkLed(args);
      break;
    }
  }
}
