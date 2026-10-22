/*
 * ui_helpers.c
 *
 *  Created on: Oct 22, 2026
 *      Author: lazar
 */

#include "main.h"

// TODO: rounded i chamfered rect funkcije za crtanje robota

static void knob_dot_pos(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, int16_t *x, int16_t *y);

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

static void knob_dot_pos(int16_t x_c, int16_t y_c, uint8_t orbit, uint16_t pos,
		uint16_t pos_zero, float step, float offset, int16_t *x, int16_t *y) {
	float angle = ((int32_t) pos - pos_zero) * step - offset;

	*x = x_c + orbit * cos(angle);
	*y = y_c + orbit * sin(angle);
}

uint8_t gray_fade(uint8_t *i, uint16_t *color, int8_t increment) {
	int16_t next = *i + increment;
	if (next >= 255) {
		*i = 255;
		*color = WHITE;
		return 0;
	}
	if (next <= 0) {
		*i = 0;
		*color = BLACK;
		return 0;
	}
	*i = next;
	*color = ((*i >> 3) << 11) | ((*i >> 2) << 5) | (*i >> 3);
	return 1;
}

uint8_t gray_fade_in_out(uint8_t *i, uint16_t *color, uint8_t increment) {
	static uint8_t phase = 0;
	uint8_t fade_output = gray_fade(i, color, phase ? -increment : increment);

	if (!fade_output) {
		if (phase) {
			phase = 0;
			return 0;
		}
		phase = 1;
	}
	return 1;
}

const char *init_text_[] = { "Initializing   ", "Initializing.  ",
		"Initializing.. ", "Initializing..." };

void draw_init_text(uint8_t repeat, uint8_t delay_ms, uint8_t dots_psc) {
	uint16_t fade_color = 0;
	uint8_t fade_output;
	uint8_t i = 0;
	uint8_t text_i = 0;

	for (uint8_t j = 0; j < repeat; j++) {
		fade_output = 1;
		while (fade_output) {
			fade_output = gray_fade_in_out(&i, &fade_color, 1);
			if (i > 0 && !(i % dots_psc)) {
				ST7789_DrawText(120, 140, init_text_[text_i], Font_11x18, BLACK,
				BLACK);
				text_i++;
				text_i %= sizeof(init_text_) / sizeof(init_text_[0]);
			}
			ST7789_DrawText(120, 140, init_text_[text_i], Font_11x18,
					fade_color, BLACK);
			if (delay_ms)
				HAL_Delay(delay_ms);
		}
	}
}
