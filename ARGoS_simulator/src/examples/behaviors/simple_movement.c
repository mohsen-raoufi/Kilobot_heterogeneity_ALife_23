#include <kilolib.h>

void setup()
{
        int bias = 0 ;

	 kilo_straight_right = kilo_straight_right + bias;
	 kilo_turn_right = kilo_turn_right + bias ;

	 kilo_straight_left = kilo_straight_left - bias;
	 kilo_turn_left = kilo_turn_left - bias;
}

void loop()
{
    // Set the LED green.
    set_color(RGB(0, 1, 0));
    // Spinup the motors to overcome friction.
    spinup_motors();
    // Move straight for 2 seconds (2000 ms).

    set_motors(kilo_straight_left, kilo_straight_right);

    //set_motors_dbl((double)kilo_straight_left + 0.1, (double)kilo_straight_right);
    delay(20);

}

int main()
{
    kilo_init();
    kilo_start(setup, loop);

    return 0;
}
