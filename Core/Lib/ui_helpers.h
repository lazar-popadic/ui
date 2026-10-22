/*
 * ui_helpers.h
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#ifndef LIB_UI_HELPERS_H_
#define LIB_UI_HELPERS_H_

void draw_circle_button(int16_t x, int16_t y, uint16_t r, const char *text,
		FontDef font, uint16_t fg, uint16_t bg, uint8_t on);
void draw_rect_button(int16_t x, int16_t y, uint16_t w, uint16_t h,
		const char *text, FontDef font, uint16_t fg, uint16_t bg, uint8_t on);
void draw_knob_dot(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, uint16_t r, uint16_t fg,
		uint16_t bg);
void erase_knob_dot(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, uint16_t r, uint16_t bg);
uint8_t gray_fade(uint8_t *i, uint16_t *color, int8_t increment);
uint8_t gray_fade_in_out(uint8_t *i, uint16_t *color, uint8_t increment);
void draw_init_text(uint8_t repeat, uint8_t delay_ms, uint8_t dots_psc);

#endif /* LIB_UI_HELPERS_H_ */
