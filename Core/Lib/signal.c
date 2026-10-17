/*
 * signal.c
 *
 *  Created on: Oct 17, 2026
 *      Author: lazar
 */

#include "main.h"

int8_t sign(float x) {
	if (x > 0)
		return 1;
	if (x < 0)
		return -1;
	return 0;
}

void debounce(uint8_t *debounced, uint8_t raw, float *time_accumulated,
		float time, float dt) {
	if (raw != *debounced) {
		*time_accumulated += dt;
		if (*time_accumulated >= time) {
			*time_accumulated = 0.0f;
			*debounced = raw;
		}
	} else {
		*time_accumulated = 0.0f;
	}
}
