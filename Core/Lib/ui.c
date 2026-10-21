/*
 * ui.c
 *
 *  Created on: Oct 20, 2026
 *      Author: lazar
 */

#define BG_COLOR	DARKBLUE
#define FG_COLOR	PEACH

#include "main.h"

static void knob_dot(uint16_t *x, uint16_t *y, uint16_t x_c, uint16_t y_c,
		uint8_t r, float angle);

// TODO: izbaci magicne brojeve i sve lepo izbaci u promenljive
// TODO: dodaj i full screen refresh funkciju

uint8_t chinch_ = 0, button_ = 0;
uint16_t knob_pos_ = 0, knob_x_ = 0, knob_y_ = 0;
uint8_t pc_ = 0, bt_ = 0, drive_ = 0;

void top_panel_init() {
	ST7789_DrawFilledRectangle(0, 0, 240, 40, BG_COLOR);
	ST7789_DrawCircle(159, 19, 16, FG_COLOR);
	ST7789_DrawRectangle(54, 9, 104, 29, FG_COLOR);
	ST7789_WriteString(145, 16, "Knob", Font_7x10, FG_COLOR, BG_COLOR);
	ST7789_WriteString(58, 16, "Chinch", Font_7x10, FG_COLOR, BG_COLOR);
	knob_dot(&knob_x_, &knob_y_, 159, 19, 15, -M_PI*0.5);
	ST7789_DrawFilledCircle(knob_x_, knob_y_, 4, BG_COLOR);
	ST7789_DrawCircle(knob_x_, knob_y_, 3, FG_COLOR);
	ST7789_DrawCircle(knob_x_, knob_y_, 2, FG_COLOR);
}

void top_panel_loop(uint8_t chinch, uint8_t button, uint16_t knob_pos) {
	if (chinch_ != chinch) {
		chinch_ = chinch;
		if (chinch) {
			ST7789_DrawFilledRectangle(55, 10, 48, 18, FG_COLOR);
			ST7789_WriteString(58, 16, "Chinch", Font_7x10, BG_COLOR, FG_COLOR);
		} else {
			ST7789_DrawFilledRectangle(55, 10, 48, 18, BG_COLOR);
			ST7789_WriteString(58, 16, "Chinch", Font_7x10, FG_COLOR, BG_COLOR);
		}
	}
	if (button_ != button) {
		button_ = button;
		if (button) {
			ST7789_DrawFilledCircle(159, 19, 15, FG_COLOR);
			ST7789_WriteString(145, 16, "Knob", Font_7x10, BG_COLOR, FG_COLOR);
		} else {
			ST7789_DrawFilledCircle(159, 19, 15, BG_COLOR);
			ST7789_WriteString(145, 16, "Knob", Font_7x10, FG_COLOR, BG_COLOR);
		}
		// Reset knob dot
		ST7789_DrawFilledCircle(knob_x_, knob_y_, 4, BG_COLOR);
		ST7789_DrawCircle(knob_x_, knob_y_, 3, FG_COLOR);
		ST7789_DrawCircle(knob_x_, knob_y_, 2, FG_COLOR);
	}
	if (knob_pos_ != knob_pos) {
		knob_pos_ = knob_pos;
		// Reset previous dot
		ST7789_DrawFilledCircle(knob_x_, knob_y_, 3, BG_COLOR);
		ST7789_DrawCircle(159, 19, 16, FG_COLOR);
		// Reset knob text and fill
		if (button) {
			ST7789_DrawFilledCircle(159, 19, 15, FG_COLOR);
			ST7789_WriteString(145, 16, "Knob", Font_7x10, BG_COLOR, FG_COLOR);
		} else {
			ST7789_DrawFilledCircle(159, 19, 15, BG_COLOR);
			ST7789_WriteString(145, 16, "Knob", Font_7x10, FG_COLOR, BG_COLOR);
		}

		knob_dot(&knob_x_, &knob_y_, 159, 19, 16,
				(knob_pos_ - 8192) * M_PI / 10 - M_PI * 0.5f);
		ST7789_DrawFilledCircle(knob_x_, knob_y_, 4, BG_COLOR);
		ST7789_DrawCircle(knob_x_, knob_y_, 3, FG_COLOR);
		ST7789_DrawCircle(knob_x_, knob_y_, 2, FG_COLOR);
	}
}

static void knob_dot(uint16_t *x, uint16_t *y, uint16_t x_c, uint16_t y_c,
		uint8_t r, float angle) {
	*x = x_c + r * cos(angle);
	*y = y_c + r * sin(angle);
}

void bottom_panel_init() {
	ST7789_DrawFilledRectangle(0, 240, 240, 40, BG_COLOR);
	ST7789_WriteString(52, 257, "PC", Font_7x10, FG_COLOR, BG_COLOR);
	ST7789_WriteString(112, 257, "BT", Font_7x10, FG_COLOR, BG_COLOR);
	ST7789_WriteString(169, 257, "STM", Font_7x10, FG_COLOR, BG_COLOR);
	ST7789_DrawCircle(59, 260, 16, FG_COLOR);
	ST7789_DrawCircle(119, 260, 16, FG_COLOR);
	ST7789_DrawCircle(179, 260, 16, FG_COLOR);
}

void bottom_panel_loop(uint8_t pc, uint8_t bt, uint8_t drive) {
	if (pc_ != pc) {
		pc_ = pc;
		if (pc) {
			ST7789_DrawFilledCircle(59, 260, 15, FG_COLOR);
			ST7789_WriteString(52, 257, "PC", Font_7x10, BG_COLOR, FG_COLOR);
		} else {
			ST7789_DrawFilledCircle(59, 260, 15, BG_COLOR);
			ST7789_WriteString(52, 257, "PC", Font_7x10, FG_COLOR, BG_COLOR);
		}
	}
	if (bt_ != bt) {
		bt_ = bt;
		if (bt) {
			ST7789_DrawFilledCircle(119, 260, 15, FG_COLOR);
			ST7789_WriteString(112, 257, "BT", Font_7x10, BG_COLOR, FG_COLOR);
		} else {
			ST7789_DrawFilledCircle(119, 260, 15, BG_COLOR);
			ST7789_WriteString(112, 257, "BT", Font_7x10, FG_COLOR, BG_COLOR);
		}
	}
	if (drive_ != drive) {
		drive_ = drive;
		if (drive) {
			ST7789_DrawFilledCircle(179, 260, 15, FG_COLOR);
			ST7789_WriteString(169, 257, "STM", Font_7x10, BG_COLOR, FG_COLOR);
		} else {
			ST7789_DrawFilledCircle(179, 260, 15, BG_COLOR);
			ST7789_WriteString(169, 257, "STM", Font_7x10, FG_COLOR, BG_COLOR);
		}
	}
}
