/*
 * ui.h
 *
 *  Created on: Oct 20, 2026
 *      Author: lazar
 */

#ifndef LIB_UI_H_
#define LIB_UI_H_

void ui_init();
void top_panel_init();
void top_panel_loop(uint8_t chinch, uint8_t button, uint16_t knob_pos);
void bottom_panel_init();
void bottom_panel_loop(uint8_t pc, uint8_t bt, uint8_t drive);

#endif /* LIB_UI_H_ */
