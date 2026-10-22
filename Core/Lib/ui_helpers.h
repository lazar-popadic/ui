/*
 * ui_helpers.h
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#ifndef LIB_UI_HELPERS_H_
#define LIB_UI_HELPERS_H_

/* Buttons: 1 px fg outline, filled inside, label centered on (x, y).
 * Off: bg fill with fg label. On: fg fill with bg label. */
void draw_circle_button(int16_t x, int16_t y, uint16_t r, const char *text,
		FontDef font, uint16_t fg, uint16_t bg, uint8_t on);
void draw_rect_button(int16_t x, int16_t y, uint16_t w, uint16_t h,
		const char *text, FontDef font, uint16_t fg, uint16_t bg, uint8_t on);

/* Knob position dot on a circle of radius orbit around (x_c, y_c), drawn as
 * a bg disc of radius r with two fg rings (r-1, r-2).
 * Its angle is (pos - pos_zero) * step - offset: step is the angle per knob
 * increment, offset M_PI * 0.5f puts pos_zero at 12 o'clock.
 * erase_knob_dot paints the dot at the same place over with bg. */
void draw_knob_dot(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, uint16_t r, uint16_t fg,
		uint16_t bg);
void erase_knob_dot(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, uint16_t r, uint16_t bg);

#endif /* LIB_UI_HELPERS_H_ */
