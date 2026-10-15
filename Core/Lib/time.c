/*
 * time.c
 *
 *  Created on: Oct 15, 2026
 *      Author: lazar
 */

#include "main.h"
#include "tim.h"

static uint32_t sys_time_ms = 0;
static float sys_time_s = 0.0f;
static const float dt_ = 0.001;

void time_ISR(void) {
	sys_time_ms++;
	sys_time_s += dt_;
	chinch_read(dt_);
}

void time_start(void) {
	HAL_TIM_Base_Start_IT(&htim10);
}

uint8_t delay_nb_2(uint32_t *start_time, uint32_t delay_ms) {
	if (*start_time > sys_time_ms)
		*start_time = sys_time_ms;
	if (sys_time_ms <= *start_time + delay_ms)
		return 0;
	*start_time = 0xffff;
	return 1;
}
