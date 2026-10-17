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
