/*
*   AUTHOR:  Jeffrey Reeves
*   WEBSITE: https://alchemist.digital/
*   GITHUB:  https://github.com/JeffReeves
*   LICENSE: GNU General Public License v3.0
*
*   PURPOSE: Improves accessibility and consistency for human players in 
*               Super Smash Brothers Melee for the Nintendo GameCube.
*
*   FEATURES:
*       - Perfect dashback that prevents slower tilt turn state
*       - Dedicated shorthop button (X)
*       - Always jumpcancel grab with Z button
*       - Three profile toggle using D-pad (Left, Right, Down)
*
*   PREREQUISITES:
*       - Arduino (UNO, Nano, etc.)
*       - Arduino IDE
*       - Logic Level Converter (3.3V to 5V)
*       - Controller (GameCube, extension cable, adapter, etc.)
*       - Game Console (GameCube, Wii, etc.)
*       - NicoHood's Nintendo Library for Arduino
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
*   NOTE: Unless approved by a Tournament Organizer (TO), do **NOT** use this 
*           Arduino assistant during competitive play.
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

// TIMING
const unsigned char JUMPSQUAT_TIME = 3;     // 3 ms works for all characters

// CONTROLS
const unsigned char ANALOG_MAX_LEFT = 0;    // maximum X position left
const unsigned char ANALOG_MAX_RIGHT = 255; // maximum X position right
const unsigned char ANALOG_MEDIAN = 128;    // middle of joystick
const unsigned char DEAD_ZONE_END = 36;     // 0.2875, closest: 36 = 0.28125
const unsigned char SMASH_TURN_START = 103; // 0.8000, closest: 103 = 0.8046875

//--[ GLOBALS ]-----------------------------------------------------------------

// TIMING
unsigned char cycles = 3;   // 3 = GC/Wii, 9 = Dolphin
unsigned char buffer = 0;   // buffer window
unsigned long timer_X = 0;  // used for keeping time when X is pressed
unsigned long timer_Z = 0;  // used for keeping time when Z is pressed

// CONTROLS
int analog_x, analog_y, cstick_x, cstick_y; // analog and cstick positions
unsigned char analog_x_abs, analog_y_abs;   // absolute analog values
unsigned char cstick_x_abs, cstick_y_abs;   // absolute cstick values 

// PROFILES
char active_profile = 'L';


//==[ FUNCTIONS ]===============================================================

//--[ BACKDASH ]----------------------------------------------------------------
// PURPOSE: Prevents slow turn around (tilt turn) when attempting to backdash.
// ACTION:  Sets the analog stick value to the maximum left/right coordinate 
//              when the stick is within the tilt turn range during a three
//              cycle buffer.
// REASON:  The console can sometimes poll the controller in the middle of a 
//              backdash. When this happens, an incorrect analog stick 
//              coordinate value is used (at no fault of the player). Instead 
//              of getting a fast dash, they get a slow turn around (tilt turn).

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

//--[ SHORTHOP ]----------------------------------------------------------------
// PURPOSE: Always shorthops when X is pressed.
// ACTION:  Disconnects X button 3 ms after it has been pressed.
// REASON:  To shorthop, a player must press and release a jump button before 
//              the character exits the jumpsquat state. On characters like Fox,
//              the jumpsquat is 3 frames, or approximately 50 ms. This makes 
//              shorthopping very difficult to consistently perform for most 
//              players -- especially those with disabilites and/or repetitive 
//              stress injuries.

void shorthop(){

    if(gcc.x){

        // if counter is zero
        if(timer_X == 0) {
            // set counter to current milliseconds
            timer_X = millis();
        }

        // while still in jumpsquat (3 frames)
        if(millis() - timer_X > JUMPSQUAT_TIME) { 
            // stop pressing X
            gcc.x = 0; 
        }
    }
    else {
        // if button is released, reset counter
        timer_X = 0;
    }
}

//--[ JUMPCANCEL GRAB ]---------------------------------------------------------
// PURPOSE: Always jumpcancel grab when Z is pressed.
// ACTION:  Starts a jump and initates a grab after jumpsquat is started,
//              resulting in a jumpcancelled grab.
// REASON:  Grabs can be performed standing still or while running/dashing. The
//              standing grab comes out faster; and if the opponent is missed, 
//              it has less ending lag than a dash grab. A jumpcancelled grab 
//              allows the player to get both of these benefits but while 
//              running/dashing. The inputs required for a human to perform a 
//              jumpcancelled grab (jump -> grab, within a window as short as
//              50 ms) can increase the possibility of a repetitive stress 
//              injury.

void jumpcancel_grab(){

    if(gcc.z){

        // disables Z for duration of macro
        gcc.z = 0;

        if(timer_Z == 0) {
            timer_Z = millis();
        }

        // until frame 2
        if(millis() - timer_Z <= 2) { 
            // hold Y to start jump
            gcc.y = 1;
        }
        // after frame 2
        else if(millis() - timer_Z > 2){
            // release Y to stop jump
            gcc.y = 0;
            // press Z to initiate grab
            gcc.z = 1;
        }
    }
    else {
        timer_Z = 0;
    }
}

//--[ PROFILES ]----------------------------------------------------------------
// PURPOSE: Allows the user to select which functions they want active by 
//              toggling between three profiles that they can customize.
// ACTION:  Selects a profile based upon a directional pad (dpad) direction. 
//              The available profiles are located on left, down, and up.
// REASON:  By having different profiles available to the player, they have the
//              freedom to choose which functions they want at any given time.

void profile_left(){
    backdash();
    shorthop();
    jumpcancel_grab();
    test();
}

void profile_right(){
    backdash();
    shorthop();
    test();
}

void profile_down(){
    test();
}

void toggle_profiles(){

    if(gcc.dleft){
        active_profile = 'L';
    }
    else if(gcc.dright){
        active_profile = 'R';
    }
    else if(gcc.ddown){
        active_profile = 'D';
    }
}

//--[ TESTING ]-----------------------------------------------------------------
// PURPOSE: Testing new ideas and functions.
// ACTION:  Whatever is necessary.
// REASON:  New ideas are always coming.

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
    // NOTE: joysticks go from 0 to 255 from left to right and bottom to top,
    //  subtracting 128 from these values shifts the origin to the middle.
    analog_x = gcc.xAxis - ANALOG_MEDIAN; 
    analog_y = gcc.yAxis - ANALOG_MEDIAN; 
    cstick_x = gcc.cxAxis - ANALOG_MEDIAN; 
    cstick_y = gcc.cyAxis - ANALOG_MEDIAN;

    // convert coordinates to an absolute value (i.e. -125 becomes 125)
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
            profile_right();
            break;
        case 'D':
            profile_down();
            break;
    }

    // write data to console
    console.write(gcc);
}