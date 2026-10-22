/*
 * ui_helpers.c
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#include "main.h"

// TODO: rounded i chamfered rect funkcije za crtanje robota

static void knob_dot_pos(int16_t x_c, int16_t y_c, uint8_t orbit,
		uint16_t pos, uint16_t pos_zero, float step, float offset, int16_t *x,
		int16_t *y);


void draw_circle_button(int16_t x, int16_t y, uint16_t r, const char *text,
		FontDef font, uint16_t fg, uint16_t bg, uint8_t on) {
	uint16_t fill = on ? fg : bg;
	uint16_t ink = on ? bg : fg;

	ST7789_DrawCircle(x, y, r, fg);
	ST7789_DrawFilledCircle(x, y, r - 1, fill);
	ST7789_DrawText(x, y, text, font, ink, fill);
}

void draw_rect_button(int16_t x, int16_t y, uint16_t w, uint16_t h,
		const char *text, FontDef font, uint16_t fg, uint16_t bg, uint8_t on) {
	uint16_t fill = on ? fg : bg;
	uint16_t ink = on ? bg : fg;

	ST7789_DrawRectCentered(x, y, w, h, fg);
	ST7789_DrawFilledRectCentered(x, y, w - 2, h - 2, fill);
	ST7789_DrawText(x, y, text, font, ink, fill);
}

void draw_knob_dot(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, uint16_t r, uint16_t fg,
		uint16_t bg) {
	int16_t x, y;
	knob_dot_pos(x_c, y_c, orbit, pos, pos_zero, step, offset, &x, &y);

	ST7789_DrawFilledCircle(x, y, r, bg);
	ST7789_DrawCircle(x, y, r - 1, fg);
	ST7789_DrawCircle(x, y, r - 2, fg);
}

void erase_knob_dot(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, uint16_t r, uint16_t bg) {
	int16_t x, y;
	knob_dot_pos(x_c, y_c, orbit, pos, pos_zero, step, offset, &x, &y);

	ST7789_DrawFilledCircle(x, y, r, bg);
}

static void knob_dot_pos(int16_t x_c, int16_t y_c, uint8_t orbit,
		uint16_t pos, uint16_t pos_zero, float step, float offset, int16_t *x,
		int16_t *y) {
	float angle = ((int32_t) pos - pos_zero) * step - offset;

	*x = x_c + orbit * cos(angle);
	*y = y_c + orbit * sin(angle);
}