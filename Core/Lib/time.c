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

static int8_t knob_button_edge_;
static uint8_t chinch_, pc_, bt_, base_;

void time_ISR(void) {
	sys_time_ms++;
	sys_time_s += dt_;
// User inputs
	chinch_ = chinch_read(dt_);
	knob_button_edge_ = knob_button_read(dt_);
	knob_position_read();
	pc_ = pc_read(dt_);
	bt_ = bt_read(dt_);
	base_ = base_read(dt_);

	if (knob_button_edge_ == 1)
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void time_init(void) {
	HAL_TIM_Base_Start_IT(&htim10);
}

uint8_t time_delay(uint32_t *start_time, uint32_t delay_ms) {
	if (*start_time > sys_time_ms)
		*start_time = sys_time_ms;
	if (sys_time_ms <= *start_time + delay_ms)
		return 0;
	*start_time = 0xffff;
	return 1;
}
