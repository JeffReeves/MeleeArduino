// this is a quick test to see if we can calculate frames and then run code only
//      once on each new frame

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


int main()
{
    // initialize random numbers with a number between 0 - 8888
    srand(time(NULL)); 
    int random_number = rand() % 8888;
    
    // generate new random microseconds to make a psuedo arduino micros()
    unsigned long micro = 0;
    unsigned long micros(){
        micro += random_number;
        return micro;
    }
    
    // constant representing number of microseconds in a single frame of Melee
    const double MICROSECOND_PER_FRAME = 16666.66666666667; // 1000000 microsecond / 60 fps
    unsigned long frame = 0;
    
    printf("micro: %u\n", micro);
    
    // pretend loop in 
    for(char i = 0; i < 20; i++){
        
        // get new micros
        unsigned long time_from_start = micros();
        printf("time_from_start: %u\n", time_from_start);
        
        // calculate current frame 
        unsigned long current_frame = ceil(time_from_start / MICROSECOND_PER_FRAME);
        
        //printf("current_frame: %u\n", frame);
        
        // if current_frame is greater than last frame, run code
        if(current_frame > frame){
            frame = current_frame;
            printf("RUN CODE FOR NEW FRAME: %u\n", frame);
        }
    }

    return 0;
}