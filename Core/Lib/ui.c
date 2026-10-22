/*
 * ui.c
 *
 *  Created on: Oct 20, 2026
 *      Author: lazar
 */

#define BG_COLOR	DARKBLUE
#define FG_COLOR	PEACH

#include "main.h"

// TODO: izbaci magicne brojeve i sve lepo izbaci u promenljive
// TODO: dodaj i full screen refresh funkciju

uint8_t chinch_ = 0, button_ = 0;
uint16_t knob_pos_ = 8192;
uint8_t pc_ = 0, bt_ = 0, drive_ = 0;

void top_panel_init() {
	ST7789_DrawFilledRect(0, 0, 239, 39, BG_COLOR);
	draw_circle_button(159, 19, 16, "Knob", Font_7x10, FG_COLOR, BG_COLOR, 0);
	draw_rect_button(79, 19, 51, 21, "Chinch", Font_7x10, FG_COLOR, BG_COLOR, 0);
	draw_knob_dot(159, 19, 16, -M_PI*0.5, 4, FG_COLOR, BG_COLOR);
}

void top_panel_loop(uint8_t chinch, uint8_t button, uint16_t knob_pos) {
	if (chinch_ != chinch) {
		chinch_ = chinch;
		draw_rect_button(79, 19, 51, 21, "Chinch", Font_7x10, FG_COLOR, BG_COLOR, chinch);
	}
	if (button_ != button) {
		button_ = button;
		draw_circle_button(159, 19, 16, "Knob", Font_7x10, FG_COLOR, BG_COLOR, button);
		// Reset knob dot
		draw_knob_dot(159, 19, 16, (knob_pos_ - 8192) * M_PI / 10 - M_PI * 0.5f,
				4, FG_COLOR, BG_COLOR);
	}
	if (knob_pos_ != knob_pos) {
		// Reset previous dot (knob_pos_ still holds its position)
		erase_knob_dot(159, 19, 16, (knob_pos_ - 8192) * M_PI / 10 - M_PI * 0.5f,
				4, BG_COLOR);
		knob_pos_ = knob_pos;
		// Reset knob outline, text and fill
		draw_circle_button(159, 19, 16, "Knob", Font_7x10, FG_COLOR, BG_COLOR, button);

		draw_knob_dot(159, 19, 16, (knob_pos_ - 8192) * M_PI / 10 - M_PI * 0.5f,
				4, FG_COLOR, BG_COLOR);
	}
}

void bottom_panel_init() {
	ST7789_DrawFilledRect(0, 240, 239, 279, BG_COLOR);
	draw_circle_button(59, 260, 16, "PC", Font_7x10, FG_COLOR, BG_COLOR, 0);
	draw_circle_button(119, 260, 16, "BT", Font_7x10, FG_COLOR, BG_COLOR, 0);
	draw_circle_button(179, 260, 16, "STM", Font_7x10, FG_COLOR, BG_COLOR, 0);
}

void bottom_panel_loop(uint8_t pc, uint8_t bt, uint8_t drive) {
	if (pc_ != pc) {
		pc_ = pc;
		draw_circle_button(59, 260, 16, "PC", Font_7x10, FG_COLOR, BG_COLOR, pc);
	}
	if (bt_ != bt) {
		bt_ = bt;
		draw_circle_button(119, 260, 16, "BT", Font_7x10, FG_COLOR, BG_COLOR, bt);
	}
	if (drive_ != drive) {
		drive_ = drive;
		draw_circle_button(179, 260, 16, "STM", Font_7x10, FG_COLOR, BG_COLOR, drive);
	}
}
