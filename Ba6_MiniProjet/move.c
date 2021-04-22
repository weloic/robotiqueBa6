#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <sensors/VL53L0X/VL53L0X.h>

#include <main.h>
#include <motors.h>
#include <move.h>

static BSEMAPHORE_DECL(start_pi_reg, FALSE); // @suppress("Field cannot be resolved")

static int16_t speed = 600;
static bool move_on = true;
static bool sleep_mode = false;

void move(uint16_t rotation) {
	if (move_on) {
		left_motor_set_speed(speed + rotation);
		right_motor_set_speed(speed - rotation);
	}
}

void move_stop(void) {
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

void set_speed(int16_t new_speed) {
	speed = new_speed;
}

void activate_motors(void) {
	move_on = true;
}

void deactivate_motors(void) {
	move_on = false;
}

//Régulateur PI afin d'approcher un code barre
int16_t pi_regulator(float distance, float goal) {
	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = distance - goal;

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and
	//the camera is a bit noisy
	if (fabs(error) < ERROR_THRESHOLD) {
		return 0;
	}

	sum_error += error;

	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth
	if (sum_error > MAX_SUM_ERROR) {
		sum_error = MAX_SUM_ERROR;
	} else if (sum_error < -MAX_SUM_ERROR) {
		sum_error = -MAX_SUM_ERROR;
	}

	speed = KP * error + KI * sum_error;

	return (int16_t) speed;
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void) arg;

	systime_t time;

	int16_t speed = 0;

	while (1) {
		if (!sleep_mode) {

			time = chVTGetSystemTime();

			//computes the speed to give to the motors
			//distance_cm is modified by the image processing thread
			speed = pi_regulator(VL53L0X_get_dist_mm(), GOAL_DISTANCE);

			//applies the speed from the PI regulator
			right_motor_set_speed(speed);
			left_motor_set_speed(speed);

			//100Hz
			chThdSleepUntilWindowed(time, time + MS2ST(10));
		} else {
			chBSemWait(&start_pi_reg);
		}
	}
}

void init_pi_regulator(void) {
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

void start_pi_regulator(void) {
	if (sleep_mode) chBSemSignal(&start_pi_reg);
	sleep_mode = false;
}

void stop_pi_regulator(void){
	sleep_mode = true;
}
