#include <kilolib.h>
#include <math.h>

#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3


#define DISPERSION 0
#define CONSENSUS 1
#define CHEMOTAXIS 2

#define NULL 0 // still dispersing
#define WAIT_FOR_OTHERS 1
#define DISCONNECTED 2
#define READY_TO_SHARE 3
#define MEASURE 4
#define SHARING 5

#define DISPERSING_RX 0xAA
#define WAITING4OTHERS_RX 0xAB
#define CONSNSS_SHARING_RX 0xCA


//static const int timeToDisperse = 9600;

static const uint8_t MAX_DISTANCE = 60; // 60 mm
int current_motion = STOP;
//int too_far = 0;
message_t message;
uint32_t last_motion_update = 0;
uint8_t cur_distance = 0;
distance_measurement_t dist;

// ---------------------
// Flag to keep track of message transmission.
int message_sent = 0;
// Flag to keep track of new messages.
int new_message = 0;

long double z_col = 0.0;
long double z_m = 0;
long double z_s = 0;
int N = 0;

double beta = 0.01;
double g = 0.6;
double lamda = 0.4;// it is reinitialized later on in the setup

int counter;

int ids[21];
double vals[21];

uint32_t tick;
uint16_t get_averaged_ambient_light();
void send_light();
void send_subState(int subState);
int contains_id(int id);
void do_random_walk();
void chemotaxis();

int last_light = 0;
int last_dist2Ref = 0;
int ref_light = 0;
int ambient = 0;

int dist2Ref = 0;
float diff_dist2Ref = 0;
int diffThresh = 10;
int switch2Exploit = 100;
int counterConstEnv = 0;
// ----------------------------------------

int state = DISPERSION; // 0: dispersion, 1: consensus, 2: chemotaxis
// Dispersion : AA: Dispersing 		AB: Waiting for other disperse		CC: Consensus SHARING		DD: Chemotaxis

//int state = 0; // 0: measure, 1: interact/communicate with others ,  2: do chemotaxis!
int subState = NULL;

// Function to handle motion.
void set_motion(int new_motion)
{
	// Only take an an action if the motion is being changed.
	if (current_motion != new_motion)
	{
		current_motion = new_motion;

		if (current_motion == STOP)
		{
			set_motors(0, 0);
		}
		else if (current_motion == FORWARD)
		{
			spinup_motors();
			set_motors(kilo_straight_left, kilo_straight_right);
		}
		else if (current_motion == LEFT)
		{
			spinup_motors();
			set_motors(kilo_turn_left, 0);
		}
		else if (current_motion == RIGHT)
		{
			spinup_motors();
			set_motors(0, kilo_turn_right);
		}
	}
}

void do_random_walk()
{
	// Generate an 8-bit random number (between 0 and 2^8 - 1 = 255).
	int random_number = rand_soft();

	// Compute the remainder of random_number when divided by 4.
	// This gives a new random number in the set {0, 1, 2, 3}.
	int random_direction = (random_number % 4);


	// There is a 50% chance of random_direction being 0 OR 1, in which
	// case set the LED green and move forward.
	if ((random_direction == 0) || (random_direction == 1))
		//            if ((random_direction == 0) || (random_direction == 1))
	{
		//                set_color(RGB(0, 1, 0));
		set_motion(FORWARD);
	}
	// There is a 25% chance of random_direction being 2, in which case
	// set the LED red and move left.
	else if (random_direction == 2)
	{
		//                set_color(RGB(1, 0, 0));
		set_motion(LEFT);
	}
	// There is a 25% chance of random_direction being 3, in which case
	// set the LED blue and move right.
	else if (random_direction == 3)
	{
		//                set_color(RGB(0, 0, 1));
		set_motion(RIGHT);
	}
}

void dispersion()
{
	// printf("%d",cur_distance);
	
	if (kilo_ticks > tick + 32)
	{
		tick = kilo_ticks;

		// If a message was received within the last second
		if (new_message == 1)//  && kilo_ticks < timeToDisperse)
		{
			new_message = 0;
			subState = NULL;
			cur_distance = estimate_distance(&dist);

			// If the distance between the kilobot and its neighbour is too
			// large, stop its motion
			if (cur_distance > MAX_DISTANCE){
				set_motion(STOP);

				subState = WAIT_FOR_OTHERS;

				// check if all the other neighbors finished their dispersion state
				int othersDone = 1;
				for (int i = 0; i < N; i++) {
					if(vals[i]==DISPERSING_RX) // if there is any one that is still dispersing, then I am not ready to share!
					{
						othersDone = 0;
						break;
					}
				}
				if(othersDone==1)
					subState = READY_TO_SHARE;
			}

			else // if the distance is close enough >> do random walk
				do_random_walk();


			if(subState == READY_TO_SHARE) //kilo_ticks > timeToDisperse && )
			{
				set_motion(STOP);
				state = CONSENSUS;
				subState = MEASURE;//READY_TO_SHARE;
			}

		}

		//		else if(new_message == 1 && subState == READY_TO_SHARE) //kilo_ticks > timeToDisperse && )
		//		{
		//			set_motion(STOP);
		//			state = CONSENSUS;
		//			subState = NULL;
		//		}
		// If no messages were received within the last second, set the LED red
		// and stop moving.
		else if(new_message == 0)
		{
			set_motion(STOP);
			subState = DISCONNECTED;
		}

		N = 0;
	}
}

void consensus()
{
	int switch2Exploit = 100; //50;//100;

	if (subState== MEASURE) { // z_m == 0) { // for initial measurement!
		counter = 0;
		int NFirstSamples = 20;
		z_m = 0;
		for (int i = 0; i < NFirstSamples; i++)
		{
			z_m += (double) get_averaged_ambient_light();
			//			set_color(RGB(0, 0, 0));
			//			delay(100);
		}
		z_m /= (double) NFirstSamples;

		delay(1500);
		set_color(RGB(0,0,0));
		delay(100);
		subState = SHARING;
		// if(kilo_uid==0)
		// {
		// 	printf("hey hey");//cout << "z_m: " << z_m << endl;
		// }
	}

	send_light();

	if(subState == SHARING) { // communicate
		if (kilo_ticks > tick + 40) { // do we need this? (now that we have couple of delays)
			tick = kilo_ticks;
			z_col = 0;
			int t = 1;
			for (int i = 0; i < N; i++) {
				z_col += (vals[i] - z_col) / t;
				++t;
				//			counter++; // we can also put it here! so that, those who have more neighbors switch to exploitation earlier
			}

			counter++;

			z_s = (double) get_averaged_ambient_light();

			if(N == 0) z_col = z_s;

			double updatedG;

			if(counter<20)
			{
				updatedG= (double) (counter/switch2Exploit);
				updatedG = g + lamda*updatedG;
			}
			else
			{
				updatedG = 0.99;
			}

			z_m = beta*z_m + updatedG*z_col + (1.0-beta-updatedG)*z_s;

			N = 0;
		}
		//
		//		phase = ((int) z_m>>5)%8;
		//		set_color(RGB(phase>>2, phase&0x02, phase&0x01));

		//		phase = ((int) z_m/40)%27; // make the value to be between 0 and 27
		//		set_color(RGB(phase/3, (phase/3)%3, phase%3));
		int phase;
		phase = ((int) z_m/125)%8; // make the value to be between 0 and 8
		set_color(RGB(phase>>2, phase&0b010, phase&0b001));
		//		if (z_m < 200)
		//			set_color(RGB(3, 3, 3));	//turn RGB LED White
		//		else if (z_m < 350)
		//			set_color(RGB(3, 0, 0));	//turn RGB LED Red
		//		else if (z_m < 500)
		//			set_color(RGB(3, 3, 0));	//turn RGB LED Orange
		//		else if (z_m < 650)
		//			set_color(RGB(0, 3, 0));	//turn RGB LED Green
		//		else if (z_m < 800)
		//			set_color(RGB(0, 3, 1));	//turn RGB LED Turquoise
		//		else if (z_m < 950)
		//			set_color(RGB(0, 0, 3));	//turn RGB LED Blue
		//		else
		//			set_color(RGB(3, 0, 3));	//turn RGB LED Violet

		if(counter==switch2Exploit)
		{

			ref_light = z_m;
			state = CHEMOTAXIS;
			for(int phase=0; phase<8; phase++)
			{
				set_color(RGB(phase>>2, phase&0b010, phase&0b001));
				delay(100);
			}
			counter ++;
		}
	}
}

void chemotaxis()
{
	if (kilo_ticks > tick + 40) { // do we need this? (now that we have couple of delays)
		tick = kilo_ticks;

		z_col = 0;
		if(N == 0) z_col = z_m;
		int t = 1;
		for (int i = 0; i < N; i++) {
			z_col += (vals[i] - z_col) / t;
			++t;
			//			counter++; // we can also put it here! so that, those who have more neighbors switch to exploitation earlier
		}
		z_m = beta*z_m + (1.0-beta)*z_col; // stil update the collective estimation, but no input from the environment!
		N = 0;
	}

	ref_light = z_m;

	

	int timeDuration = 200;

	ambient = get_averaged_ambient_light(); // current ambient light
	dist2Ref = fabs(ambient-ref_light); // the distance  to the ref. light (unsigned)
	int checkVal = fabs(last_light-ambient);

	// if(kilo_uid==0)
	// 	printf("%i\n", ambient);

	//printf("Hey");//"ref_light: " << ref_light << ", current: " << ambient << endl;

	if(dist2Ref<10)//  if it is close enough to the consensus light! if(fabs(ambient-ref_light)<70)
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
		delay(10*timeDuration);
	}
	else // still needs to do the chemotaxis
	{
		if( (checkVal>diffThresh) ) // || (ambient > 1000) ) // if you could sense some difference in the light, compared to the last time you measured it
		{
			set_color(RGB(0,3,3));//Cyan RGB LED
			delay(100);

			last_light = ambient; // update the last light, for the next steps ...

			diff_dist2Ref = dist2Ref - last_dist2Ref;

			last_dist2Ref = dist2Ref;//fabs(last_light-ref_light); // diff2Ref // update the difference to the ref. light for the next steps

			z_s = ambient;

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

				if(1)//rnd<180)//128)
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
				delay(2*timeDuration);
			}
		}
		else
		{
			set_motion(FORWARD);
			set_color(RGB(1,1,0));
			delay(100);
			set_color(RGB(0, 0, 0)); // off
			delay(timeDuration);
			counterConstEnv++;
		}

		if(counterConstEnv > 40) // if the robot is measuring the same value for a while
		{
			counterConstEnv = 0; // reset the counter
			spinup_motors();
			set_motors(180, 180);
			delay(400);
			int rnd = rand_soft();
			if(rnd<180)//128)
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
		}
	}
}

void setup()
{
	
	int sign_param = kilo_uid%2;
	int bias = (1-2*sign_param)*((kilo_uid+1)/2%5);
	if(kilo_uid < 20)
	{
	 kilo_straight_left = kilo_straight_left + bias;	
	 kilo_turn_left = kilo_turn_left + bias;
	}
	else
	{
	   kilo_straight_right = kilo_straight_right + bias;	
	   kilo_turn_right = kilo_turn_right + bias;
	}

	// Initialize message:
	// The type is always NORMAL.
	message.type = NORMAL;
	// Some dummy data as an example.
	message.data[0] = 0;
	// It's important that the CRC is computed after the data has been set;
	// otherwise it would be wrong and the message would be dropped by the
	// receiver.
	message.crc = message_crc(&message);

	kilo_ticks += (rand_hard())<<2;

	rand_seed(rand_hard()); // randomly seed the random for soft_rand

	tick = kilo_ticks;
	counter = 1;

	lamda = 1.0 - beta - g; // just to get sure the sum of parameters is equal to 1.0

	delay(2000);
}

void loop()
{
	// if(kilo_uid==0)
	// 	printf("apple");//"%i",kilo_uid);
	switch (state) {
	case CONSENSUS:
		consensus();

		delay(50);
		set_color(RGB(0,0,0));
		delay(50);
		break;

	case CHEMOTAXIS:
		chemotaxis();
		break;

	case DISPERSION: // dispersion
		dispersion();

		switch (subState) {
		case NULL:
			set_color(RGB(0, 1, 0));
			break;
		case DISCONNECTED:
			set_color(RGB(1, 0, 0));
			break;
		case WAIT_FOR_OTHERS:
			set_color(RGB(0, 0, 1));
			break;
		case READY_TO_SHARE:
			set_color(RGB(0, 1, 1));
			break;
		}

		break;



			//		default:
			//			set_color(RGB(1,0,0));
			//			delay(100);
			//			set_color(RGB(0,0,0));
			//			delay(100);
			//			break;
	}


}

//void message_rx(message_t *m, distance_measurement_t *d)
void message_rx(message_t *message_r, distance_measurement_t *distance)
{
	new_message = 1;
	dist = *distance;

	if(state==DISPERSION)
	{
		send_subState(subState);
		//		send_subState(NULL);
		if(subState != NULL)
		{

			{
				int id = message_r->data[0];
				int ind = contains_id(id);
				int checkStateOthers = (int) message_r->data[1];
				if (ind != -1) {
					vals[ind] = checkStateOthers;
				}
				else {
					ids[N] = id;
					vals[N] = checkStateOthers;
					N++;
				}
			}
		}
	}

	else //if(state==CONSENSUS)
	{
		//		int id = message_r->data[0];
		int checkStateOthers = (int) message_r->data[1];
		if(checkStateOthers == DISPERSING_RX)// || checkStateOthers == WAITING4OTHERS_RX)
		{
			state = DISPERSION;
			subState = WAIT_FOR_OTHERS;
			send_subState(subState);
		}
		else // others are also in consensus state
		{
			//
			//		}
			//
			//	}
			//
			//	else if(0)//state!=DISPERSION)
			//	{
			// Reset the flag so the LED is only blinked once per message.
			int id = message_r->data[0];
			int st = message_r->data[1];
			if(st==CONSNSS_SHARING_RX)
			{
				uint32_t light = (unsigned long) message_r->data[2];
				light = light << 8;
				light |= message_r->data[3];
				light = light << 8;
				light |= message_r->data[4];
				light = light << 8;
				light |= message_r->data[5];

				double l = light / (double) 1000;
				int ind = contains_id(id);
				if (ind != -1) {
					vals[ind] = l;
				} else {
					ids[N] = id;
					vals[N] = l;
					N++;
				}
			}

			send_light();
		}
	}
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


int main()
{
	// initialize hardware
	kilo_init();
	//	debug_init();
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

void send_subState(int subState)
{
	uint8_t st;
	//	AA: Dispersing 		BB: Waiting for other disperse		CC: Consensus		DD: Chemotaxis
	switch (subState) {
	case NULL:
		st = DISPERSING_RX;
		break;
	case WAIT_FOR_OTHERS:
		st = WAITING4OTHERS_RX;
		break;
	case READY_TO_SHARE:
		st = CONSNSS_SHARING_RX;
		break;
	case MEASURE:
		st = CONSNSS_SHARING_RX;
		break;
	default:
		st = 0xBB;
		break;
	}
	message.data[0] = kilo_uid;
	message.data[1] = st; // imDispersing


	message.crc = message_crc(&message);
	message_sent = 0;
}

void send_light() {

	uint32_t x = z_m * 1000; // send z_m or z_s
	message.data[0] = kilo_uid;
	message.data[1] = CONSNSS_SHARING_RX;
	message.data[2] = (x >> 24) & 0xFF;
	message.data[3] = (x >> 16) & 0xFF;
	message.data[4] = (x >> 8) & 0xFF;
	message.data[5] = (x) & 0xFF;

	message.crc = message_crc(&message);
	message_sent = 0;

}

int contains_id(int id) {
	for (int i = 0; i < N; i++) {
		if (ids[i] == id)
			return i;
	}
	return -1;
}

//get light intensity
uint16_t get_averaged_ambient_light() {
	static const uint16_t NUM_SAMPLES = 400;
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
