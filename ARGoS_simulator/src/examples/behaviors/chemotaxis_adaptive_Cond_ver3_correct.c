#include "kilolib.h"
#include <stdio.h>
#include <math.h>


FILE *f;

message_t message;
// Flag to keep track of message transmission.
int message_sent = 0;
// Flag to keep track of new messages.
int new_message = 0;

long double z_col = 0.0;
long double z_m = 0;
long double z_s = 0;
int N = 0;
double alpha = 0.1;

double beta = 0.08;
double g = 0.9;
double lamda = 0.02;// it is reinitialized later on in the setup

int counter;
int wn = 500;

int ids[21];
double vals[21];

uint32_t tick;
uint16_t get_averaged_ambient_light();

int current_motion;
int last_light = 0;
int last_dist2Ref = 0;
int ref_light = 0;
int ambient = 0;
int state = 0; // 0: measure, 1: interact/communicate with others ,  2: do chemotaxis!
int dist2Ref = 0;
float diff_dist2Ref = 0;
int diffThresh = 20;

// Constants for motion handling function.
#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3

// Function to handle motion.
void set_motion(int new_motion)
{
	// We only need to take an action if the motion is being changed.
	if (current_motion != new_motion)
	{
		current_motion = new_motion;

		if (current_motion == STOP)
		{
			set_motors(0, 0);
			//			set_color(RGB(1, 1, 1)); // White
		}
		else if (current_motion == FORWARD)
		{
			spinup_motors();
			set_motors(kilo_straight_left, kilo_straight_right);
			//			set_color(RGB(0, 1, 0)); // Green
		}
		else if (current_motion == LEFT)
		{
			spinup_motors();
			set_motors(kilo_turn_left, 0);
			set_color(RGB(1, 0, 0)); // Red
		}
		else if (current_motion == RIGHT)
		{
			spinup_motors();
			set_motors(0, kilo_turn_right);
			//			set_color(RGB(0, 0, 1)); // Blue
		}
	}
}


int contains_id(int id) {
	for (int i = 0; i < N; i++) {
		if (ids[i] == id)
			return i;
	}
	return -1;
}

void send_light() {

	uint32_t x = z_m * 1000; // send z_m or z_s
	message.data[0] = kilo_uid;
	message.data[1] = (x >> 24) & 0xFF;
	message.data[2] = (x >> 16) & 0xFF;
	message.data[3] = (x >> 8) & 0xFF;
	message.data[4] = (x) & 0xFF;

	//convert_message(1,4,x);

	unsigned long light = (int) message.data[1];
	light = light << 8;
	light |= message.data[2];
	light = light << 8;
	light |= message.data[3];
	light = light << 8;
	light |= message.data[4];

	message.crc = message_crc(&message);

	message_sent = 0;

}

void setup() {


	// Initialize message:
	// The type is always NORMAL.
	message.type = NORMAL;
	// Some dummy data as an example.
	message.data[0] = 0;
	// It's important that the CRC is computed after the data has been set;
	// otherwise it would be wrong and the message would be dropped by the
	// receiver.
	message.crc = message_crc(&message);
	tick = kilo_ticks;
	counter = 1;

	//int sign_param = kilo_uid%2;

	lamda = 1.0 - beta - g; // just to get sure the sum of parameters is equal to 1.0

	set_color(RGB(0,3,3));
	delay(1000);
	ref_light = 1024;
	diffThresh = 20; // the threshold for the difference between two samples of light, if it is above this value it will make decision

	// printf("HELLO WORLD\n");
}

void loop() {

	// printf("Hey 00!\n");

	int timeDuration = 200;

	ambient = get_averaged_ambient_light(); // current ambient light

	// printf("Hey 00-1!\n");
	dist2Ref = fabs(ambient-ref_light); // the distance  to the ref. light (unsigned)
	int checkVal = fabs(last_light-ambient);

	// f= fopen("light.txt","a");
	// fprintf(f,"%d %d %d \n",kilo_uid, ambient, ref_light);
	// fclose(f);

	// printf("Hey 01!\n");

	if(dist2Ref<2)//  if it is close enough to the consensus light! if(fabs(ambient-ref_light)<70)
	{
		set_motion(STOP);
		set_color(RGB(1, 1, 1)); // White
		delay(100);
		set_color(RGB(0,0,0)); // OFF
		delay(200);
		set_color(RGB(1, 1, 1)); // White
		delay(100);
		set_color(RGB(0,0,0)); // OFF
		delay(200);
		delay(timeDuration);
	}
	else // still needs to do the chemotaxis
	{
		// printf("Hey 02!\n");
		if( (checkVal>diffThresh) ) // || (ambient > 1000) ) // if you could sense some difference in the light, compared to the last time you measured it
		{
			// printf("Hey 00!\n");
			set_color(RGB(0,3,3));//Cyan RGB LED
			delay(200);

			last_light = ambient; // update the last light, for the next steps ...

			diff_dist2Ref = dist2Ref - last_dist2Ref;

			last_dist2Ref = dist2Ref;//fabs(last_light-ref_light); // diff2Ref // update the difference to the ref. light for the next steps


//			dist2Ref = dist2Ref; //- dist2Ref + last_dist2Ref; // update it. how the difference to the ref. light has changed since the last update

			//		timeDuration = (int) ((0.75/450)*(double)diff2Ref + 0.25)*timeDuration;
			//		if(timeDuration>1000)
			//			timeDuration = 1000;
			//		else if(timeDuration<250)
			//			timeDuration = 250;

			z_s = ambient;

			//			if(diff2Ref>30) // if the different to the desired light is great enough!
			//			{
			if(diff_dist2Ref<0)
			{
				set_motion(FORWARD);
				set_color(RGB(0, 1, 0)); // Green
				delay(100);
				set_color(RGB(0, 0, 0)); // off
				//		delay(1000);
				delay(timeDuration);
			}
			else if(diff_dist2Ref>1)
			{
				int rnd = rand_soft();
				//				printf("random No.: %3d\n",rnd);

				set_motion(STOP);
				set_color(RGB(1, 1, 1)); // White
				delay(100);

				if(1)//rnd<80)//1)// 
				{
					set_motion(RIGHT);
					set_color(RGB(0, 0, 1)); // Blue
				}
				else
				{
					set_motion(LEFT);
					set_color(RGB(1, 0, 0)); // RED
				}

				delay(100);
				set_color(RGB(0, 0, 0)); // off
				delay(2000);
				set_motion(FORWARD);
				//				set_color(RGB(0,0,0));
				//				delay(100);
				set_color(RGB(3,0,3));//turn RGB LED Violet
				delay(50);
				set_color(RGB(0, 0, 0)); // off
				delay(7*timeDuration);

			}
			//			}
			//			else //  if it is close enough to the consensus light! if(fabs(ambient-ref_light)<70)
			//			{
			//				set_motion(STOP);
			//				set_color(RGB(1, 1, 1)); // White
			//				delay(100);
			//				set_color(RGB(0,0,0)); // OFF
			//				delay(200);
			//				set_color(RGB(1, 1, 1)); // White
			//				delay(100);
			//				set_color(RGB(0,0,0)); // OFF
			//				delay(200);
			//				delay(timeDuration);
			//			}
		}
		else
		{
			set_motion(FORWARD);
			set_color(RGB(1,1,0));
			delay(timeDuration);
		}
	}

	// printf("Hey 03!\n");
}

message_t* message_tx() {
	if (message_sent == 0) {
		return &message;
	}
	return 0;
}

void message_tx_success() {
	// Set the flag on message transmission.
	message_sent = 1;
}

void message_rx(message_t *message_r, distance_measurement_t *distance) {
	// Reset the flag so the LED is only blinked once per message.
	int id = message_r->data[0];
	uint32_t light = (unsigned long) message_r->data[1];
	light = light << 8;
	light |= message_r->data[2];
	light = light << 8;
	light |= message_r->data[3];
	light = light << 8;
	light |= message_r->data[4];

	double l = light / (double) 1000;
	int ind = contains_id(id);
	if (ind != -1) {
		vals[ind] = l;
	} else {
		ids[N] = id;
		vals[N] = l;
		N++;
	}

	send_light();
}

int main() {
	// initialize hardware
	kilo_init();

	// Register the message_tx callback function.
	kilo_message_tx = message_tx;
	// Register the message_tx_success callback function.
	kilo_message_tx_success = message_tx_success;
	// Register the message_rx callback function.
	kilo_message_rx = message_rx;
	// start program
	kilo_start(setup, loop);



	return 0;
}

//get light intensity

uint16_t get_averaged_ambient_light() {
	static const uint16_t NUM_SAMPLES = 10; //400
	uint32_t sum = 0;
	uint16_t sample_counter = 0;

	while (sample_counter < NUM_SAMPLES) {
		int16_t sample = get_ambientlight();
		if (sample != -1) {
			sum += sample;
			sample_counter++;
		}
	}
	return (sum / NUM_SAMPLES);
}
