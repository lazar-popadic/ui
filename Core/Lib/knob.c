/*
 * knob.c
 *
 *  Created on: Oct 15, 2026
 *      Author: lazar
 */

#include "main.h"
#include "gpio.h"
#include "tim.h"

static int16_t count_diff(TIM_HandleTypeDef *htim);

static volatile uint8_t knob_button_debounced_;
static float knob_button_time_ = 0.02f;
static float knob_button_time_accumulated_ = 0.0f;

static volatile int16_t knob_position_ = 0;
static volatile int16_t previous_knob_position_ = 0;

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

void knob_init(void) {
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

void knob_position_read(void) {
	knob_position_ += count_diff(&htim3);
}

int16_t get_knob_position(void) {
	return knob_position_;
}

static int16_t count_diff(TIM_HandleTypeDef *htim) {
	int16_t current = (int16_t) (htim->Instance->CNT);
	int16_t diff = current - previous_knob_position_;
	previous_knob_position_ = current;
	return diff;
}
