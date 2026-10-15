/*
 * knob.h
 *
 *  Created on: Oct 15, 2026
 *      Author: lazar
 */

#ifndef LIB_KNOB_H_
#define LIB_KNOB_H_

void knob_button_read(float dt);
uint8_t get_knob_button(void);
void knob_init(void);
void knob_position_read(void);
int16_t get_knob_position(void);

#endif /* LIB_KNOB_H_ */
