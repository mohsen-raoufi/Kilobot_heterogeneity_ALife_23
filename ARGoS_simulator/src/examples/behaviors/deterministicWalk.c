#include "kilolib.h"

#include <math.h>

#include <stdlib.h>

message_t message;
// Flag to keep track of message transmission.
int message_sent = 0;
// Flag to keep track of new messages.
int new_message = 0;
int distance = 0;
int state = 0;
int actions[20];

#define STOP 3
#define FORWARD 2
#define LEFT 0
#define RIGHT 1


int current_motion = STOP;

uint32_t counter;

int ids[11];
double vals[11];

uint32_t tick;

void blink() {
	set_color(RGB(3, 0, 3));
	delay(50);
	set_color(RGB(0, 0, 0));
	delay(50);
	set_color(RGB(3, 0, 3));
	delay(50);
	set_color(RGB(0, 0, 0));

}

void convert_message(int s, int e, uint32_t x) {
	for (int i = (e - s); i <= 0; i--) {
		message.data[s] = (x >> (i * 8)) & 0xFF;
		s++;
	}
}


void set_motion(int new_motion) {
	// Only take an action if the motion is being changed.
	if (current_motion != new_motion) {
		current_motion = new_motion;

		if (current_motion == STOP) {
			set_motors(0, 0);
		} else if (current_motion == FORWARD) {
			spinup_motors();
			set_motors(kilo_straight_left, kilo_straight_right);
		} else if (current_motion == LEFT) {
			spinup_motors();
			set_motors(kilo_turn_left, 0);
		} else if (current_motion == RIGHT) {
			spinup_motors();
			set_motors(0, kilo_turn_right);
		}
	}
}

void setup() {

	if(kilo_uid < 5)
	{
	 kilo_straight_left = kilo_straight_left + kilo_uid;	
	 kilo_turn_left = kilo_turn_left + kilo_uid;
	}
	else
	{
	   kilo_straight_right = kilo_straight_right + kilo_uid -4;	
	   kilo_turn_right = kilo_turn_right + kilo_uid - 4;
	}
	
	tick = kilo_ticks;
	counter = 0;
	for (int i=0; i< 20; i++) {
		actions[i] = 2;
	}
	actions[1] = 0;
	actions[3] = 1;
	actions[5] = 1;
	actions[7] = 0;
	actions[9] = 0;
	actions[11] = 0;
	actions[13] = 1;
	actions[15] = 0;
	actions[17] = 1;
	actions[19] = 1;
	//	actions = [2,0,2,1,2,1,2,0,2,0,2,0,2,1,2,0,2,1,2,1];

}

void loop() {

	int m = actions[counter%20];
	counter++;
	set_motion(STOP);
	delay(500);
	set_motion(m);

	if( m == FORWARD) set_color(RGB(0,1,0));
	else if (m == LEFT) set_color(RGB(1,0,0));
	else set_color(RGB(0,0,1));

	delay(2000);

	if(counter==30){
		set_motion(STOP);
		for(int j=0; j<100; j++){
			set_motion(STOP);
			blink();
			delay(100);
		}
	}


}



int main() {
	// initialize hardware
	kilo_init();


	kilo_start(setup, loop);

	return 0;
}
