/*
 * chinch.c
 *
 *  Created on: Oct 15, 2026
 *      Author: lazar
 */

#include "main.h"
#include "gpio.h"

static volatile uint8_t chinch_debounced_;
static float chinch_time_ = 0.02f;
static float chinch_time_accumulated_ = 0.0f;

uint8_t chinch_read(float dt) {
	uint8_t chinch_raw = !!(Chinch_GPIO_Port->IDR & Chinch_Pin);
	debounce(&chinch_debounced_, chinch_raw, &chinch_time_accumulated_, chinch_time_, dt);
	return chinch_debounced_;
}

uint8_t get_chinch(void) {
	return chinch_debounced_;
}
