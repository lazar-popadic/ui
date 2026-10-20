/*
 * signal.h
 *
 *  Created on: Oct 17, 2026
 *      Author: lazar
 */

#ifndef LIB_SIGNAL_H_
#define LIB_SIGNAL_H_

int8_t sign(float x);
void debounce(volatile uint8_t *debounced, uint8_t raw, float *time_accumulated,
		float time, float dt);

#endif /* LIB_SIGNAL_H_ */
