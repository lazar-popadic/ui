/*
 * knob.c
 *
 *  Created on: Oct 15, 2026
 *      Author: lazar
 */

#include "main.h"
#include "gpio.h"
#include "tim.h"

static volatile uint8_t knob_button_debounced_;
static float knob_button_time_ = 0.02f;
static float knob_button_time_accumulated_ = 0.0f;

void knob_button_read(float dt) {
	uint8_t knob_button_raw = !!(Knob_button_GPIO_Port->IDR & Knob_button_Pin);
	if (knob_button_raw != knob_button_debounced_) {
		knob_button_time_accumulated_ += dt;
		if (knob_button_time_accumulated_ >= knob_button_time_) {
			knob_button_time_accumulated_ = 0.0f;
			knob_button_debounced_ = knob_button_raw;
		}
	} else {
		knob_button_time_accumulated_ = 0.0f;
	}
}

uint8_t get_knob_button(void) {
	return knob_button_debounced_;
}
