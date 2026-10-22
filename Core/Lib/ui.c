/*
 * ui.c
 *
 *  Created on: Oct 20, 2026
 *      Author: lazar
 */

#include "main.h"
#include "tim.h"

// TODO: dodaj i full screen refresh funkciju, na neku promenljivu

/* Panels */
FontDef *panel_font = &Font_7x10;
uint16_t panel_height = 40;
uint16_t panel_foreground_color = PEACH;
uint16_t panel_background_color = DARKBLUE;
uint16_t button_radius = 16;

/* Top panel */
uint16_t knob_x = 159;
uint16_t chinch_x = 79;
uint16_t chinch_width = 51;
uint16_t chinch_height = 21;

/* Bottom panel */
uint16_t pc_x = 59;
uint16_t bt_x = 119;
uint16_t base_x = 179;

/* Knob dot */
uint16_t knob_dot_radius = 4;
uint16_t knob_pos_zero = 8192;
float knob_step = M_PI / 10;
float knob_offset = M_PI * 0.5f;

/* Calculated in the panel init functions */
uint16_t top_panel_y, bottom_panel_y;

uint8_t chinch_ = 0, button_ = 0;
uint16_t knob_pos_ = 0;
uint8_t pc_ = 0, bt_ = 0, base_ = 0;

void ui_update(void) {
	top_panel_loop(get_chinch(), get_knob_button(), get_knob_position());
	bottom_panel_loop(get_pc_switch(), get_bt_switch(), get_base_switch());
}

void ui_init(void) {
	// Reset variables
	chinch_ = 0;
	button_ = 0;
	knob_pos_ = 0;
	pc_ = 0;
	bt_ = 0;
	base_ = 0;
	// Black screen
	ST7789_Fill_Color(BLACK);
	HAL_Delay(250);
	// Initializing text fade
	draw_init_text(1, 0, 32);
	HAL_Delay(250);
	// Bottom panel: switches
	bottom_panel_init();
	// Top panel: Knob position, push button, chinch
	top_panel_init();
	// Panel separators
	ST7789_DrawRect(1, panel_height - 1, ST7789_WIDTH - 1,
	ST7789_HEIGHT - panel_height, panel_foreground_color);
}

void top_panel_init(void) {
	top_panel_y = (panel_height - 1) / 2;
	knob_pos_ = knob_pos_zero;

	ST7789_DrawFilledRect(0, 0, ST7789_WIDTH - 1, panel_height - 1,
			panel_background_color);
	draw_circle_button(knob_x, top_panel_y, button_radius, "Knob", *panel_font,
			panel_foreground_color, panel_background_color, 0);
	draw_rect_button(chinch_x, top_panel_y, chinch_width, chinch_height,
			"Chinch", *panel_font, panel_foreground_color,
			panel_background_color, 0);
	draw_knob_dot(knob_x, top_panel_y, button_radius, knob_pos_, knob_pos_zero,
			knob_step, knob_offset, knob_dot_radius, panel_foreground_color,
			panel_background_color);
}

void top_panel_loop(uint8_t chinch, uint8_t button, uint16_t knob_pos) {
	if (chinch_ != chinch) {
		chinch_ = chinch;
		draw_rect_button(chinch_x, top_panel_y, chinch_width, chinch_height,
				"Chinch", *panel_font, panel_foreground_color,
				panel_background_color, chinch);
	}
	if (button_ != button) {
		button_ = button;
		draw_circle_button(knob_x, top_panel_y, button_radius, "Knob",
				*panel_font, panel_foreground_color, panel_background_color,
				button);
		draw_knob_dot(knob_x, top_panel_y, button_radius, knob_pos_,
				knob_pos_zero, knob_step, knob_offset, knob_dot_radius,
				panel_foreground_color, panel_background_color);
	}
	if (knob_pos_ != knob_pos) {
		erase_knob_dot(knob_x, top_panel_y, button_radius, knob_pos_,
				knob_pos_zero, knob_step, knob_offset, knob_dot_radius,
				panel_background_color);
		knob_pos_ = knob_pos;
		draw_circle_button(knob_x, top_panel_y, button_radius, "Knob",
				*panel_font, panel_foreground_color, panel_background_color,
				button);
		draw_knob_dot(knob_x, top_panel_y, button_radius, knob_pos_,
				knob_pos_zero, knob_step, knob_offset, knob_dot_radius,
				panel_foreground_color, panel_background_color);
	}
}

void bottom_panel_init(void) {
	bottom_panel_y = ST7789_HEIGHT - 1 - (panel_height - 1) / 2;

	ST7789_DrawFilledRect(0, ST7789_HEIGHT - panel_height, ST7789_WIDTH - 1,
	ST7789_HEIGHT - 1, panel_background_color);
	draw_circle_button(pc_x, bottom_panel_y, button_radius, "PC", *panel_font,
			panel_foreground_color, panel_background_color, 0);
	draw_circle_button(bt_x, bottom_panel_y, button_radius, "BT", *panel_font,
			panel_foreground_color, panel_background_color, 0);
	draw_circle_button(base_x, bottom_panel_y, button_radius, "base",
			*panel_font, panel_foreground_color, panel_background_color, 0);
}

void bottom_panel_loop(uint8_t pc, uint8_t bt, uint8_t base) {
	if (pc_ != pc) {
		pc_ = pc;
		draw_circle_button(pc_x, bottom_panel_y, button_radius, "PC",
				*panel_font, panel_foreground_color, panel_background_color,
				pc);
	}
	if (bt_ != bt) {
		bt_ = bt;
		draw_circle_button(bt_x, bottom_panel_y, button_radius, "BT",
				*panel_font, panel_foreground_color, panel_background_color,
				bt);
	}
	if (base_ != base) {
		base_ = base;
		draw_circle_button(base_x, bottom_panel_y, button_radius, "base",
				*panel_font, panel_foreground_color, panel_background_color,
				base);
	}
}
