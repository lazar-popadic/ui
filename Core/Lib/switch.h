/*
 * switch.h
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#ifndef LIB_SWITCH_H_
#define LIB_SWITCH_H_

uint8_t pc_read(float dt);
uint8_t bt_read(float dt);
uint8_t drive_read(float dt);
uint8_t get_pc_switch(void);
uint8_t get_bt_switch(void);
uint8_t get_drive_switch(void);

#endif /* LIB_SWITCH_H_ */
