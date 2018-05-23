//==[ ANALOG STICKS ]===========================================================

const unsigned char ANALOG_MAX_LEFT = 0;     // maximum X position left
const unsigned char ANALOG_MAX_RIGHT = 255;  // maximum X position right
const unsigned char ANALOG_MEDIAN = 128;     // middle of joystick

// using 128
// joystick coordinates 0.0000 to 1.0000
// each 1 of 128 is equal to 0.0078125
// 36.8 = 0.2875 , 36 = 0.28125
// tilt turn is 0.2875 to 0.7875 
// smash turn is 0.8000 to 1.0000
// 102.4 = 0.8000 , 103 = 0.8046875

// using 255
// each 1 of 255 is equal to 0.003921568627451
// 0.2875 = 73.31249999999963, 73 = 0.286274509803923  DEAD ZONE END
// 0.8000 = 203.99999999999, 203 = 0.80000000000004  SMASH TURN START

const unsigned char DEAD_ZONE_END = 36;      // 36 ON 128, 73 on 255
const unsigned char SMASH_TURN_START = 103;  // 103 ON 128, 203 on 255


//function to return angle in degrees when given x and y components
float ang(float xval, float yval){
    return atan(yval / xval) * 57.2958;
}

//function to return vector magnitude when given x and y components
float mag(char xval, char yval){
    return sqrt(sq(xval) + sq(yval));
} 

//==[ POLLING ]=================================================================

unsigned char cycles = 3; // number of cycles to use (3 = GC/Wii, 9 = Dolphin)


//==[ CHEATS ]==================================================================


//--[ PEACH FLOAT CANCEL NAIRS ]------------------------------------------------

unsigned long macro_state, delta_t, last_t; // Peach float cancel

fc_nair_macro(); // peach float cancel nair with X

void fc_nair_macro(){
    if(gcc.x || macro_state){
        if(macro_state < 25){
            gcc.y = 1;
            gcc.yAxis = 0;
        } 
        else if(macro_state < 35){
            gcc.y = 0;
            gcc.x = 0;
            gcc.xAxis = ANALOG_MEDIAN;
            gcc.yAxis = ANALOG_MEDIAN;
            gcc.a = 1;
        } 
        else if(macro_state < 50){
            gcc.yAxis = 0;
        } 
        else {
            macro_state = 0 - delta_t;
        }
        macro_state += delta_t;
    }
}


// Timing Code in main loop
delta_t = millis() - last_t;
last_t = millis();


//==[ TIME AND FRAMES ]=========================================================

/* for some reason, each macro needs its own variable to store millis(), 
*     if they share a common variable it fails 
*/

//--[ FRAME COUNTER USING MICROSECONDS ]----------------------------------------

const double MICROSECOND_PER_FRAME = 16666.66666666667; // 1000000 μs / 60 fps
unsigned long frame = 0;  // used to count frames since start

// counts the number of frames since the start
void count_frames(){

    // calculate current frame
    unsigned long current_frame = floor(micros() / MICROSECOND_PER_FRAME);

    // if current_frame is greater than the last time we checked
    if(current_frame > frame){
        frame = current_frame;
    }
}

void test(){

    if(gcc.y){
        Serial.print("FRAME: ");
        Serial.print(frame);
        Serial.println();
    }
}

void loop(){
    // determine the current frame
    count_frames();
}