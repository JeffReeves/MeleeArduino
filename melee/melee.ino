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
*       - Dedicated shorthop (X) and fullhop (Y) buttons
*       - Always jumpcancel grab with Z button
*       - Always jumpcancel upsmash with C-Stick Up
*       - Dedicated light shield (L) button
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
const unsigned char PRE_JUMPSQUAT_TIME = 2;   // 2 ms works for all characters
const unsigned char JUMPSQUAT_TIME = 3;       // 3 ms works for all characters
const unsigned char POST_JUMPSQUAT_TIME = 25; // 25 ms works for all characters 

// CONTROLS
// using 0-255 true value
const unsigned char JOYSTICK_MEDIAN    = 128;   // middle position of joystick
const unsigned char JOYSTICK_MAX_LEFT  = 0;     // maximum X position left
const unsigned char JOYSTICK_MAX_DOWN  = 0;     // maximum Y position down
const unsigned char JOYSTICK_MAX_RIGHT = 255;   // maximum X position right
const unsigned char JOYSTICK_MAX_UP    = 255;   // maximum Y position up

const unsigned char SMASH_START_LEFT  = 73;     // 73/255  = 0.2863 left
const unsigned char SMASH_START_DOWN  = 73;     // 73/255  = 0.2863 down
const unsigned char SMASH_START_RIGHT = 204;    // 204/255 = 0.8000 right
const unsigned char SMASH_START_UP    = 204;    // 204/255 = 0.8000 up

const unsigned char SMASH_ALMOST_MAX_UP  = 245;    // 245/255 = 0.9608 up

const unsigned char LIGHT_SHIELD = 85;             // 85/255 = 33% pressed

// using -128 to +127 absolute value
const unsigned char DEAD_ZONE_END    = 36;      // 0.2875, closest: 36 = 0.28125
const unsigned char WALK_END         = 102;     // 0.7999, closest: 102 = 0.7969
const unsigned char SMASH_TURN_START = 103;     // 0.8000, closest: 103 = 0.8047

//--[ GLOBALS ]-----------------------------------------------------------------

// TIMING
unsigned char cycles = 3;           // 3 = GC/Wii, 9 = Dolphin
unsigned char buffer = 0;           // buffer window

unsigned long timer_X = 0;          // keep time when X is pressed
unsigned long timer_Y = 0;          // keep time when Y is pressed
unsigned long timer_Z = 0;          // keep time when Z is pressed
unsigned long timer_cstick_up = 0;  // keep time when cstick is moved up

// STATES
unsigned char jumping = 0;            // character jumping
unsigned char shielding = 0;          // character shielding
unsigned char jc_grab_active = 0;     // character jumpcancel grab
unsigned char jc_upsmash_active = 0;  // character jumpcancel upsmash

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
                    
                    if(positive){
                        // set x value to max right
                        gcc.xAxis = JOYSTICK_MAX_RIGHT;
                    }
                    else {
                        // set x value to max left
                        gcc.xAxis = JOYSTICK_MAX_LEFT;
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
        if(timer_X == 0){
            // set counter to current milliseconds
            timer_X = millis();
        }

        // while still in jumpsquat (3 frames)
        if(millis() - timer_X > JUMPSQUAT_TIME){ 
            // stop pressing X
            gcc.x = 0; 
        }
    }
    else {
        // if button is released, reset counter
        timer_X = 0;
    }
}

//--[ FULLHOP ]-----------------------------------------------------------------
// PURPOSE: Always fullhops when Y is pressed.
// ACTION:  When the Y button is pressed a "jumping" state is activated. While
//              in the "jumping" state, the Y button is held down until out of
//              jumpsquat (currently 25 ms).
// REASON:  To fullhop, a player must press and release a jump button after 
//              the character exits the jumpsquat state. Forcing a fullhop 
//              on a single button press ensures consistency for the player -- 
//              especially when paired with the shorthop button.

void fullhop(){

    if(gcc.y | jumping){

        // if counter is zero
        if(timer_Y == 0){
            // set counter to current milliseconds
            timer_Y = millis();
            // toggle jumping status to on
            jumping = 1;
        }

        // before exiting jumpsquat (6 frames)
        if(millis() - timer_Y <= POST_JUMPSQUAT_TIME){ 
            // keep pressing Y
            gcc.y = 1;
        }
        else if(millis() - timer_Y > POST_JUMPSQUAT_TIME){
            gcc.y = 0;
            // stop jumping status
            jumping = 0;
        }
    }
    else {
        // if button is released, reset counter
        timer_Y = 0;
    }
}

//--[ LIGHTSHIELD ONLY ]--------------------------------------------------------
// PURPOSE: Always light shield when L is pressed.
// ACTION:  Converts any digital L press to a % of analog L pressing.
// REASON:  A digital shield press (aka "hard shielding") locks the player out 
//              of "teching" for 40 frames, even when used for "L canceling". 
//              An analog shield press (aka "light shielding") does not cause 
//              this same 40 frame lockout. In addition, there is almost no need
//              for two seperate shield triggers.

void lightshield_only(){

    if(gcc.left >= 1 | gcc.l | shielding){

        // disable the digital L press
        gcc.l = 0;

        // toggle shielding status to on
        shielding = 1;

        // lightshield with analog L press at 33%
        gcc.left = LIGHT_SHIELD;
        
        // stop shielding status
        shielding = 0;
    }
}

//--[ JUMPCANCEL GRAB ]---------------------------------------------------------
// PURPOSE: Always jumpcancel grab when Z is pressed.
// ACTION:  If the player is holding the X axis of the analog stick in a 
//              position outside of the deadzone and walk sections, and the Z 
//              button is pressed, start a jump and initate a grab before 
//              jumpsquat is finished. This results in a jumpcancelled grab.
// REASON:  Grabs can be performed standing still or while running/dashing. The
//              standing grab comes out faster; and if the opponent is missed, 
//              it has less ending lag than a dash grab. A jumpcancelled grab 
//              allows the player to get both of these benefits but while 
//              running/dashing. The inputs required for a human to perform a 
//              jumpcancelled grab (jump -> grab, within a window as short as
//              50 ms) can increase the possibility of a repetitive stress 
//              injury.

void jumpcancel_grab(){

    // if analog stick is outside of the deadzone and walk range
    if(analog_x_abs > WALK_END) {

        if(gcc.z | jc_grab_active){

            // disable Z for duration of macro
            gcc.z = 0;

            // set status of jumpcancelled grab to active
            jc_grab_active = 1;

            if(timer_Z == 0){
                timer_Z = millis();
            }

            // until frame 2
            if(millis() - timer_Z <= PRE_JUMPSQUAT_TIME){ 
                // hold Y to start jump
                gcc.y = 1;
            }
            // after frame 2
            else if(millis() - timer_Z > PRE_JUMPSQUAT_TIME){
                // release Y to stop jump
                gcc.y = 0;
                // press Z to initiate grab
                gcc.z = 1;
                // disable jumpcancel status
                jc_grab_active = 0;
            }
        }
        else {
            timer_Z = 0;
            jc_grab_active = 0;
        }
    }
    else {
        timer_Z = 0;
        jc_grab_active = 0;
    }
}

//--[ JUMPCANCEL UPSMASH ]------------------------------------------------------
// PURPOSE: Always jumpcancel upsmash when the C stick is moved up.
// ACTION:  If the player is holding the X axis of the analog stick in a 
//              position outside of the deadzone and walk sections, and the
//              C Stick's Y axis is in the up smash position, start a jump and 
//              initate an upsmash attack before jumpsquat is finished. This 
//              results in a jumpcancelled upsmash.
// REASON:  A jumpcancelled upsmash allows the player to insert an upsmash 
//              attack while dashing / running, as if they were standing still.
//              Therefore, this macro exists for the same reason the jumpcancel
//              grab does.

void jumpcancel_upsmash(){

    // if analog stick is outside of the deadzone and walk range
    if(analog_x_abs > WALK_END) {

        // if cstick Y axis is greater or equal to the start of the upsmash 
        //      range (204), or a jumpcancelled upsmash is already started
        if((gcc.cyAxis >= SMASH_START_UP) | jc_upsmash_active){

            // set status to active
            jc_upsmash_active = 1;

            // set cstick Y axis to the middle position for now
            gcc.cyAxis = JOYSTICK_MEDIAN;

            // start a timer if one isn't already running
            if(timer_cstick_up == 0){
                timer_cstick_up = millis();
            }

            // until frame 2
            if(millis() - timer_cstick_up <= PRE_JUMPSQUAT_TIME){ 
                // hold Y to start jump
                gcc.y = 1;
            }
            // after frame 2
            else if(millis() - timer_cstick_up > PRE_JUMPSQUAT_TIME){
                // release Y to stop jump
                gcc.y = 0;
                // set cstick Y axis in max value for upsmash
                gcc.cyAxis = JOYSTICK_MAX_UP;
                // stop upsmash status
                jc_upsmash_active = 0;
            }
        }
        else {
            timer_cstick_up = 0;
            jc_upsmash_active = 0;
        }
    }
    else {
        timer_cstick_up = 0;
        jc_upsmash_active = 0;
    }
}

//--[ L+R+A+START ]-------------------------------------------------------------
// PURPOSE: Enter L+R+A+Start for quitting a game when paused.
// ACTION:  Directional pad (dpad) up is converted to L+R+A+Start.
// REASON:  Any light shield only trigger buttons prevents pressing L+R+A+Start
//              to quit out of an active game when in the pause menu. In 
//              addition, the button combination can be difficult for those with
//              disabilities.

void lra_start(){

    if(gcc.dup){

        // press L+R+A+Start
        gcc.l = 1;
        gcc.r = 1;
        gcc.a = 1;
        gcc.start = 1;
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
    fullhop();
    lightshield_only();
    jumpcancel_grab();
    jumpcancel_upsmash();
    lra_start();
}

void profile_right(){
    backdash();
    shorthop();
    fullhop();
}

void profile_down(){
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
    analog_x = gcc.xAxis - JOYSTICK_MEDIAN; 
    analog_y = gcc.yAxis - JOYSTICK_MEDIAN; 
    cstick_x = gcc.cxAxis - JOYSTICK_MEDIAN; 
    cstick_y = gcc.cyAxis - JOYSTICK_MEDIAN;

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
