/*
 * switch.c
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#include "main.h"
#include "gpio.h"

static volatile uint8_t pc_debounced_, bt_debounced_, drive_debounced_;
static float switch_time_ = 0.1f;
static float pc_time_accumulated_ = 0.0f, bt_time_accumulated_ = 0.0f,
		drive_time_accumulated_ = 0.0f;

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



uint8_t drive_read(float dt) {
	uint8_t drive_raw = !!(Drive_Switch_GPIO_Port->IDR & Drive_Switch_Pin);
	debounce(&drive_debounced_, drive_raw, &drive_time_accumulated_, switch_time_, dt);
	return drive_debounced_;
}

uint8_t get_pc_switch(void) {
	return pc_debounced_;
}

uint8_t get_bt_switch(void) {
	return bt_debounced_;
}

uint8_t get_drive_switch(void) {
	return drive_debounced_;
}
