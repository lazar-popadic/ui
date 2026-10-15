/*
 * time.h
 *
 *  Created on: Oct 15, 2026
 *      Author: lazar
 */

#ifndef LIB_TIMER_H_
#define LIB_TIMER_H_

void time_ISR(void);
void time_start(void);
uint8_t delay_nb_2(uint32_t *start_time, uint32_t delay_ms);

#endif /* LIB_TIMER_H_ */
