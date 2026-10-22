/*
 * switch.c
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#include "main.h"
#include "gpio.h"

static volatile uint8_t pc_debounced_, bt_debounced_, base_debounced_;
static float switch_time_ = 0.1f;
static float pc_time_accumulated_ = 0.0f, bt_time_accumulated_ = 0.0f,
		base_time_accumulated_ = 0.0f;

uint8_t pc_read(float dt) {
	uint8_t pc_raw = !!(PC_Switch_GPIO_Port->IDR & PC_Switch_Pin);
	debounce(&pc_debounced_, pc_raw, &pc_time_accumulated_, switch_time_, dt);
	return pc_debounced_;
}

uint8_t bt_read(float dt) {
	uint8_t bt_raw = !!(BT_Switch_GPIO_Port->IDR & BT_Switch_Pin);
	debounce(&bt_debounced_, bt_raw, &bt_time_accumulated_, switch_time_, dt);
	return bt_debounced_;
}



uint8_t base_read(float dt) {
	uint8_t base_raw = !!(Base_Switch_GPIO_Port->IDR & Base_Switch_Pin);
	debounce(&base_debounced_, base_raw, &base_time_accumulated_, switch_time_, dt);
	return base_debounced_;
}

uint8_t get_pc_switch(void) {
	return pc_debounced_;
}

uint8_t get_bt_switch(void) {
	return bt_debounced_;
}

uint8_t get_base_switch(void) {
	return base_debounced_;
}
