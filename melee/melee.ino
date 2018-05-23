/*
*   AUTHOR:  Jeffrey Reeves
*   WEBSITE: https://alchemist.digital/
*   GITHUB:  https://github.com/JeffReeves
*   PURPOSE: Improves consistency in Super Smash Brothers Melee for the 
*               Nintendo GameCube.
*
*
*   FEATURES:
*       - Three profile toggle using D-pad (Left, Right, Down)
*       - Perfect dashback that prevents tilt turn
*       - Dedicated shorthop button (X)
*
*
*   PREREQUISITES:
*       - Arduino (UNO, Nano, etc.)
*       - Arduino IDE
*       - Logic Level Converter (3.3V to 5V)
*       - Controller (GameCube, extension cable, adapter, etc.)
*       - Game Console (GameCube, Wii, etc.)
*       - NicoHood's Nintendo Library for Arduino
*
*
*   HOW TO INSTALL:
*       1. Connect the Controller to the Arduino and the Logic Level Converter 
*           (see diagram on main github readme).
*       2. Connect Arduino to PC and open the Arduino IDE.
*       3. Select the correct Board and Port under the Tools menu.
*       4. Copy this entire file and paste it over the current Arduino Sketch.
*       5. Select the Upload option to upload this code to the Arduino.
*       6. Disconnect the Arduino from the PC.
*       7. Plug in the Controller into the Game Console and start it up.
*       8. Enjoy playing Melee!
*
*
*/


/*==[ INIT ]==================================================================*/

//--[ NICOHOOD'S NINTENDO LIBRARY ]---------------------------------------------

#include <Nintendo.h>

// arduino ports for data wire
CGamecubeController controller(2);  // reads from controller on D2
CGamecubeConsole console(3);        // writes to console on D3

// define object to store controller data
Gamecube_Report_t gcc = defaultGamecubeData.report;


//--[ CONSTANTS ]---------------------------------------------------------------

// timing
const unsigned char PROFILE_CHANGE_TIME = 100; // 100 ms

// controls
const unsigned char ANALOG_MAX_LEFT = 0;     // maximum X position left
const unsigned char ANALOG_MAX_RIGHT = 255;  // maximum X position right
const unsigned char ANALOG_MEDIAN = 128;     // middle of joystick
const unsigned char DEAD_ZONE_END = 36;      // 0.2875, closest: 36 = 0.28125
const unsigned char SMASH_TURN_START = 103;  // 0.8000, closest: 103 = 0.8046875


//--[ GLOBALS ]-----------------------------------------------------------------

// timing
unsigned long counter_X = 0;    // used for keeping time during macros
unsigned char cycles = 3;       // 3 = GC/Wii, 9 = Dolphin
unsigned char buffer = 0; 
unsigned long loop_time, current_time; 

// controls
int analog_x, analog_y, cstick_x, cstick_y; 
unsigned char analog_x_abs, analog_y_abs, cstick_x_abs, cstick_y_abs;

// profiles
char active_profile = 'L';


//==[ FUNCTIONS ]===============================================================

//--[ SUPPORT ]-----------------------------------------------------------------


//--[ PROFILES ]----------------------------------------------------------------

void profile_left(){
    backdash();
    shorthop();
    test();
}

void profile_right(){
    test();
}

void profile_down(){
    // everything disabled
}

// checks if a duration has passed
bool time_elapsed(int duration){

    // sets the current time
    if(current_time == 0){
        current_time = millis();
    }

    // returns true if duration has passed
    if(loop_time - current_time > duration){
        current_time = 0;
        return true;
    }

    return false;
}

// toggles between different profiles using the dpad
void toggle_profiles(){

    // dpad left
    if(gcc.dleft){

        if(time_elapsed(PROFILE_CHANGE_TIME)){
          active_profile = 'L';
        }
    }
    // dpad right
    else if(gcc.dright){

        if(time_elapsed(PROFILE_CHANGE_TIME)){
          active_profile = 'R';
        }
    }
    // dpad down
    else if(gcc.ddown){

        if(time_elapsed(PROFILE_CHANGE_TIME)){
          active_profile = 'D';
        }
    }
}


//--[ CONSISTENCY ]-------------------------------------------------------------

// prevents tilt turn when attempting to backdash
// sets the analog stick to max left/right when in tilt turn range
void backdash(){

    // if analog stick Y is within deadzone
    if(analog_y_abs <= DEAD_ZONE_END){

        // if analog stick X is within deadzone
        if(analog_x_abs <= DEAD_ZONE_END){

            // refresh buffer (default: 3)
            buffer = cycles;
        }
        // else analog stick X is outside of deadzone
        else {

            // if a buffer exists
            if(buffer > 0){
                
                // decrement buffer
                buffer--; 
                
                // if the analog stick X is in tilt turn position 
                if(analog_x_abs < SMASH_TURN_START){
                    
                    // check if the x position is positive or negative
                    bool positive = analog_x > 0;
                    
                    if(positive) {
                        // set x value to max right
                        gcc.xAxis = ANALOG_MAX_RIGHT; // 255
                    }
                    else {
                        // set x value to max left
                        gcc.xAxis = ANALOG_MAX_LEFT; // 0
                    }
                } 
                // if analog stick is within smash turn start
                else {
                    // don't change x value 
                    // clear buffer 
                    buffer = 0;
                }
            }
        }
    }
    // analog stick exited Y deadzone
    else {
        // clear buffer
        buffer = 0;
    }
}


//--[ ACCESSIBILITY ]-----------------------------------------------------------

// shorthop only button on X
void shorthop(){

    if(gcc.x){

        // if counter is zero
        if(counter_X == 0) {
            // set counter to current milliseconds
            counter_X = millis();
        }

        // if current milliseconds is 
        if(millis() - counter_X > 3) {
            gcc.x = 0; 
        }
    }
    else {
        counter_X = 0;
    }
}


//--[ TESTING ]-----------------------------------------------------------------

void test(){

}

/*==[ SETUP ]=================================================================*/

void setup(){

    // calibrates initial state of controller
    gcc.origin = 0;
    gcc.errlatch = 0; 
    gcc.high1 = 0; 
    gcc.errstat = 0;
}


/*==[ LOOP ]==================================================================*/

void loop(){

    // read from the controller
    controller.read(); 

    // store the current state
    gcc = controller.getReport();

    // calculates the x and y coordinates of the joysticks
    // NOTE: joysticks go from 0 to 255, 
    //  subtracting 128 shifts the centerpoint to the middle.
    analog_x = gcc.xAxis - ANALOG_MEDIAN; 
    analog_y = gcc.yAxis - ANALOG_MEDIAN; 
    cstick_x = gcc.cxAxis - ANALOG_MEDIAN; 
    cstick_y = gcc.cyAxis - ANALOG_MEDIAN;

    // convert coordinates to an absolute value (i.e. -125 = 125)
    analog_x_abs = abs(analog_x);
    analog_y_abs = abs(analog_y);
    cstick_x_abs = abs(cstick_x);
    cstick_y_abs = abs(cstick_y);

    // enables profile toggle
    toggle_profiles();

    // activate profile based on selection with dpad
    switch(active_profile){
        case 'L':
            profile_left();
            break;
        case 'R':
            profile_left();
            break;
        case 'D':
            profile_down();
            break;
    }

    // write data to console
    console.write(gcc);
}