#include "st7789.h"

/* ========================================================================== */
/*  Private definitions                                                       */
/* ========================================================================== */

#ifdef USE_DMA
/* Transfers shorter than this (in bytes) are sent by polling the SPI registers,
 * because setting up a DMA transfer through HAL costs more than it saves. */
#define DMA_MIN_SIZE	64
#endif

/* Pixel buffer used for text rendering.
 * Pixels are stored already byte-swapped (big-endian, as the controller expects)
 * so the buffer can be streamed out as plain bytes.
 * Solid fills don't use it (see ST7789_WriteColor).
 */
#define HOR_LEN 		20	//	Also mind the resolution of your screen!
#define DISP_BUF_LEN	(ST7789_WIDTH * HOR_LEN)
static uint16_t disp_buf[DISP_BUF_LEN];

#ifdef USE_DMA
/* Set while a solid fill DMA transfer may still be running. CS stays low and
 * SPI/DMA stay in 16-bit mode until ST7789_WaitIdle() cleans up. */
static volatile uint8_t fill_pending;
/* Source of the fill DMA: must outlive the call that starts the transfer */
static uint16_t fill_color;
#endif

static inline uint16_t ST7789_Swap16(uint16_t v) {
	return (v >> 8) | (v << 8);
}

/* ========================================================================== */
/*  SPI transport                                                             */
/* ========================================================================== */

#ifdef USE_DMA
/**
 * @brief Switch SPI to 16-bit frames and the TX DMA to a fixed 16-bit source,
 * so one color value can be repeated for a whole fill.
 * SPI must be idle.
 */
static void ST7789_SPI_Mode16Fixed(void) {
	SPI_TypeDef *spi = ST7789_SPI_PORT.Instance;
	DMA_Stream_TypeDef *dma = ST7789_SPI_PORT.hdmatx->Instance;

	CLEAR_BIT(spi->CR1, SPI_CR1_SPE); // DFF may only change while SPI is disabled
	SET_BIT(spi->CR1, SPI_CR1_DFF);
	SET_BIT(spi->CR1, SPI_CR1_SPE);
	MODIFY_REG(dma->CR, DMA_SxCR_MINC | DMA_SxCR_PSIZE | DMA_SxCR_MSIZE,
			DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0);
}

/**
 * @brief Restore 8-bit SPI frames and the incrementing 8-bit TX DMA set up by CubeMX.
 * SPI must be idle.
 */
static void ST7789_SPI_Mode8(void) {
	SPI_TypeDef *spi = ST7789_SPI_PORT.Instance;
	DMA_Stream_TypeDef *dma = ST7789_SPI_PORT.hdmatx->Instance;

	CLEAR_BIT(spi->CR1, SPI_CR1_SPE);
	CLEAR_BIT(spi->CR1, SPI_CR1_DFF);
	SET_BIT(spi->CR1, SPI_CR1_SPE);
	MODIFY_REG(dma->CR, DMA_SxCR_MINC | DMA_SxCR_PSIZE | DMA_SxCR_MSIZE,
			DMA_SxCR_MINC);
}
#endif

/**
 * @brief Wait for a background fill to finish and release the bus.
 * Every driver call does this itself; call it directly only if you need to
 * know the display is done (e.g. before sharing SPI1 or entering sleep).
 * @return none
 */
void ST7789_WaitIdle(void) {
#ifdef USE_DMA
	if (!fill_pending)
		return;
	while (HAL_SPI_GetState(&ST7789_SPI_PORT) != HAL_SPI_STATE_READY) {
	}
	ST7789_SPI_Mode8();
	ST7789_UnSelect();
	fill_pending = 0;
#endif
}

/* Start a transaction: finish any background fill, then select the display */
static void ST7789_Begin(void) {
	ST7789_WaitIdle();
	ST7789_Select();
}

/* End a transaction; a background fill keeps CS low until it completes */
static void ST7789_End(void) {
#ifdef USE_DMA
	if (fill_pending)
		return;
#endif
	ST7789_UnSelect();
}

/**
 * @brief Send raw bytes over SPI, using DMA for long transfers and register
 * polling for short ones. Returns only once the last bit has left the shifter,
 * so DC/CS may be changed right after.
 * @param buff -> pointer of data buffer
 * @param buff_size -> size of the data buffer
 * @return none
 */
static void ST7789_SPI_Write(const uint8_t *buff, size_t buff_size) {
#ifdef USE_DMA
	if (buff_size >= DMA_MIN_SIZE) {
		// split data in small chunks because HAL can't send more than 64K at once
		while (buff_size > 0) {
			uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
			HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, (uint8_t*) buff, chunk_size);
			/* Wait on the SPI state, not the DMA state: the DMA finishes
			 * before the last byte has been shifted out. */
			while (HAL_SPI_GetState(&ST7789_SPI_PORT) != HAL_SPI_STATE_READY) {
			}
			buff += chunk_size;
			buff_size -= chunk_size;
		}
		return;
	}
#endif
	SPI_TypeDef *spi = ST7789_SPI_PORT.Instance;
	while (buff_size--) {
		while (!(spi->SR & SPI_SR_TXE)) {
		}
		*(__IO uint8_t*) &spi->DR = *buff++;
	}
	while (!(spi->SR & SPI_SR_TXE)) {
	}
	while (spi->SR & SPI_SR_BSY) {
	}
	/* Discard what was clocked in on MISO, clearing the overrun flag */
	(void) spi->DR;
	(void) spi->SR;
}

/* ========================================================================== */
/*  Controller commands                                                       */
/*  These do not touch CS: the public function that starts a transaction     */
/*  selects the display once and releases it at the end.                      */
/* ========================================================================== */

/**
 * @brief Write command to ST7789 controller
 * @param cmd -> command to write
 * @return none
 */
static void ST7789_WriteCommand(uint8_t cmd) {
	ST7789_DC_Clr();
	ST7789_SPI_Write(&cmd, sizeof(cmd));
}

/**
 * @brief Write data to ST7789 controller
 * @param buff -> pointer of data buffer
 * @param buff_size -> size of the data buffer
 * @return none
 */
static void ST7789_WriteData(const uint8_t *buff, size_t buff_size) {
	ST7789_DC_Set();
	ST7789_SPI_Write(buff, buff_size);
}

/**
 * @brief Write data to ST7789 controller, simplify for 8bit data.
 * data -> data to write
 * @return none
 */
static void ST7789_WriteSmallData(uint8_t data) {
	ST7789_WriteData(&data, sizeof(data));
}

/**
 * @brief Send the MADCTL command for a rotation
 * @param m -> rotation parameter(please refer it in st7789.h)
 * @return none
 */
static void ST7789_SendRotation(uint8_t m) {
	ST7789_WriteCommand(ST7789_MADCTL);	// MADCTL
	switch (m) {
	case 0:
		ST7789_WriteSmallData(
		ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
		break;
	case 1:
		ST7789_WriteSmallData(
		ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	case 2:
		ST7789_WriteSmallData(ST7789_MADCTL_RGB);
		break;
	case 3:
		ST7789_WriteSmallData(
		ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	default:
		break;
	}
}

/**
 * @brief Set address of DisplayWindow and start a RAM write
 * @param xi&yi -> coordinates of window
 * @return none
 */
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1,
		uint16_t y1) {
	uint16_t x_start = x0 + X_SHIFT, x_end = x1 + X_SHIFT;
	uint16_t y_start = y0 + Y_SHIFT, y_end = y1 + Y_SHIFT;

	/* Column Address set */
	ST7789_WriteCommand(ST7789_CASET);
	{
		uint8_t data[] = { x_start >> 8, x_start & 0xFF, x_end >> 8, x_end
				& 0xFF };
		ST7789_WriteData(data, sizeof(data));
	}

	/* Row Address set */
	ST7789_WriteCommand(ST7789_RASET);
	{
		uint8_t data[] = { y_start >> 8, y_start & 0xFF, y_end >> 8, y_end
				& 0xFF };
		ST7789_WriteData(data, sizeof(data));
	}
	/* Write to RAM */
	ST7789_WriteCommand(ST7789_RAMWR);
}

/**
 * @brief Stream count pixels of a single color into the current window
 * With DMA, large fills repeat a single 16-bit value without a buffer and
 * are left running in the background: the function returns as soon as the
 * last chunk has started.
 * @param color -> color to write
 * @param count -> number of pixels
 * @return none
 */
static void ST7789_WriteColor(uint16_t color, uint32_t count) {
	ST7789_DC_Set();

#ifdef USE_DMA
	if (count * sizeof(uint16_t) >= DMA_MIN_SIZE) {
		fill_color = color;	// 16-bit frames go out MSB first, no swap needed
		ST7789_SPI_Mode16Fixed();
		fill_pending = 1;
		while (count > 0) {
			uint16_t chunk = count > 65535 ? 65535 : count;
			while (HAL_SPI_GetState(&ST7789_SPI_PORT) != HAL_SPI_STATE_READY) {
			}
			HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, (uint8_t*) &fill_color,
					chunk);
			count -= chunk;
		}
		return;
	}
#endif

	/* Short fills: a few pixels by polling */
	uint16_t px[32];
	uint16_t swapped = ST7789_Swap16(color);
	uint32_t i, n = count < 32 ? count : 32;
	for (i = 0; i < n; i++)
		px[i] = swapped;
	while (count > 0) {
		uint32_t chunk = count < 32 ? count : 32;
		ST7789_SPI_Write((uint8_t*) px, chunk * sizeof(uint16_t));
		count -= chunk;
	}
}

/* ========================================================================== */
/*  Drawing primitives                                                        */
/* ========================================================================== */

/**
 * @brief Clip a box (x0 <= x1, y0 <= y1, inclusive) to the screen
 * @return 0 if nothing of it is visible
 */
static int ST7789_Clip(int32_t *x0, int32_t *y0, int32_t *x1, int32_t *y1) {
	if (*x1 < 0 || *y1 < 0 || *x0 >= ST7789_WIDTH || *y0 >= ST7789_HEIGHT
			|| *x0 > *x1 || *y0 > *y1)
		return 0;
	if (*x0 < 0)
		*x0 = 0;
	if (*y0 < 0)
		*y0 = 0;
	if (*x1 >= ST7789_WIDTH)
		*x1 = ST7789_WIDTH - 1;
	if (*y1 >= ST7789_HEIGHT)
		*y1 = ST7789_HEIGHT - 1;
	return 1;
}

/**
 * @brief Fill an area with a single color in one window, clipped to the screen
 * @param x0&y0 -> one corner (inclusive)
 * @param x1&y1 -> opposite corner (inclusive)
 * @param color -> color to Fill with
 * @return none
 */
static void ST7789_FillArea(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
		uint16_t color) {
	int32_t swap;
	if (x0 > x1) {
		swap = x0;
		x0 = x1;
		x1 = swap;
	}
	if (y0 > y1) {
		swap = y0;
		y0 = y1;
		y1 = swap;
	}
	if (!ST7789_Clip(&x0, &y0, &x1, &y1))
		return;

	ST7789_Begin();
	ST7789_SetAddressWindow(x0, y0, x1, y1);
	ST7789_WriteColor(color,
			(uint32_t) (x1 - x0 + 1) * (uint32_t) (y1 - y0 + 1));
	ST7789_End();
}

/* Receives one straight run of line pixels, from (x0, y0) to (x1, y1) inclusive */
typedef void (*ST7789_RunFn)(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
		void *ctx);

/**
 * @brief Walk a line with Bresenham's algorithm and hand every run of
 * consecutive pixels along the major axis to run(), so a line is drawn as a
 * few fills instead of one per pixel.
 */
static void ST7789_LineRuns(int32_t x0, int32_t y0, int32_t xe, int32_t ye,
		ST7789_RunFn run, void *ctx) {
	int32_t swap;
	int32_t steep = ABS(ye - y0) > ABS(xe - x0);
	if (steep) {
		swap = x0;
		x0 = y0;
		y0 = swap;

		swap = xe;
		xe = ye;
		ye = swap;
	}

	if (x0 > xe) {
		swap = x0;
		x0 = xe;
		xe = swap;

		swap = y0;
		y0 = ye;
		ye = swap;
	}

	int32_t dx = xe - x0;
	int32_t dy = ABS(ye - y0);
	int32_t err = dx / 2;
	int32_t ystep = (y0 < ye) ? 1 : -1;
	int32_t run_start = x0;

	for (; x0 <= xe; x0++) {
		err -= dy;
		/* The minor coordinate changes after this pixel (or the line ends): flush the run */
		if (err < 0 || x0 == xe) {
			if (steep)
				run(y0, run_start, y0, x0, ctx);
			else
				run(run_start, y0, x0, y0, ctx);
			y0 += ystep;
			err += dx;
			run_start = x0 + 1;
		}
	}
}

/* ST7789_RunFn for ST7789_DrawLine: fill the run, ctx points to the color */
static void ST7789_LineRunFill(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
		void *ctx) {
	ST7789_FillArea(x0, y0, x1, y1, *(uint16_t*) ctx);
}

/* Per-row horizontal extent of a triangle's outline */
typedef struct {
	int16_t xmin[ST7789_HEIGHT];
	int16_t xmax[ST7789_HEIGHT];
} ST7789_Spans;

/**
 * @brief ST7789_RunFn for ST7789_DrawFilledTriangle: widen the span of every
 * on-screen row the run touches. ctx points to an ST7789_Spans.
 * x is stored clamped to -1..ST7789_WIDTH (anything past an edge is clipped anyway).
 */
static void ST7789_LineRunSpan(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
		void *ctx) {
	ST7789_Spans *s = ctx;
	int32_t y;

	if (x0 < -1)
		x0 = -1;
	if (x1 > ST7789_WIDTH)
		x1 = ST7789_WIDTH;
	if (y0 < 0)
		y0 = 0;
	if (y1 >= ST7789_HEIGHT)
		y1 = ST7789_HEIGHT - 1;

	for (y = y0; y <= y1; y++) {
		if (x0 < s->xmin[y])
			s->xmin[y] = x0;
		if (x1 > s->xmax[y])
			s->xmax[y] = x1;
	}
}

/**
 * @brief Draw the 8 symmetric arcs of a circle outline for one stretch of the
 * midpoint algorithm where y stayed constant while x went from xs to xe.
 * Those pixels are straight runs, so each arc is one fill.
 */
static void ST7789_CircleArcs(int32_t x0, int32_t y0, int32_t xs, int32_t xe,
		int32_t y, uint16_t color) {
	ST7789_FillArea(x0 + xs, y0 + y, x0 + xe, y0 + y, color);
	ST7789_FillArea(x0 - xe, y0 + y, x0 - xs, y0 + y, color);
	ST7789_FillArea(x0 + xs, y0 - y, x0 + xe, y0 - y, color);
	ST7789_FillArea(x0 - xe, y0 - y, x0 - xs, y0 - y, color);

	ST7789_FillArea(x0 + y, y0 + xs, x0 + y, y0 + xe, color);
	ST7789_FillArea(x0 - y, y0 + xs, x0 - y, y0 + xe, color);
	ST7789_FillArea(x0 + y, y0 - xe, x0 + y, y0 - xs, color);
	ST7789_FillArea(x0 - y, y0 - xe, x0 - y, y0 - xs, color);
}

/**
 * @brief Fill the two rows y0 +- dy of a filled circle with half-width hw
 */
static void ST7789_CircleRows(int32_t x0, int32_t y0, int32_t dy, int32_t hw,
		uint16_t color) {
	ST7789_FillArea(x0 - hw, y0 + dy, x0 + hw, y0 + dy, color);
	if (dy != 0)
		ST7789_FillArea(x0 - hw, y0 - dy, x0 + hw, y0 - dy, color);
}

/**
 * @brief Corners of the w x h box centered on (cx, cy): x0 = cx - w/2,
 * x1 = x0 + w - 1 (same for y), so odd sizes are symmetric
 * @return 0 if the box is empty
 */
static int ST7789_CenteredBox(int32_t cx, int32_t cy, uint32_t w, uint32_t h,
		int32_t *x0, int32_t *y0, int32_t *x1, int32_t *y1) {
	if (w == 0 || h == 0)
		return 0;
	*x0 = cx - (int32_t) w / 2;
	*y0 = cy - (int32_t) h / 2;
	*x1 = *x0 + (int32_t) w - 1;
	*y1 = *y0 + (int32_t) h - 1;
	return 1;
}

/**
 * @brief Row (from the top of the character cell) in the middle of the
 * capital letters, taken from the glyph 'H'. The cell has room for
 * descenders below, so centering on capitals looks centered to the eye.
 */
static int32_t ST7789_FontCapMiddle(FontDef font) {
	const uint16_t *glyph = &font.data[('H' - 32) * font.height];
	int32_t top = 0, bottom = font.height - 1;

	while (top < bottom && glyph[top] == 0)
		top++;
	while (bottom > top && glyph[bottom] == 0)
		bottom--;
	return (top + bottom) / 2;
}

/* ========================================================================== */
/*  Control functions                                                         */
/* ========================================================================== */

/**
 * @brief Initialize ST7789 controller
 * @param none
 * @return none
 */
void ST7789_Init(void) {
	ST7789_WaitIdle();
	HAL_Delay(10);
	ST7789_RST_Clr();
	HAL_Delay(10);
	ST7789_RST_Set();
	HAL_Delay(20);

	/* Short transfers write SPI registers directly, so the peripheral
	 * must be enabled up front (HAL only enables it on its first transfer) */
	__HAL_SPI_ENABLE(&ST7789_SPI_PORT);
	ST7789_Begin();

	ST7789_WriteCommand(ST7789_COLMOD);		//	Set color mode
	ST7789_WriteSmallData(ST7789_COLOR_MODE_16bit);
	ST7789_WriteCommand(0xB2);				//	Porch control
	{
		uint8_t data[] = { 0x0C, 0x0C, 0x00, 0x33, 0x33 };
		ST7789_WriteData(data, sizeof(data));
	}
	ST7789_SendRotation(ST7789_ROTATION);	//	MADCTL (Display Rotation)

	/* Internal LCD Voltage generator settings */
	ST7789_WriteCommand(0XB7);				//	Gate Control
	ST7789_WriteSmallData(0x35);			//	Default value
	ST7789_WriteCommand(0xBB);				//	VCOM setting
	ST7789_WriteSmallData(0x19);			//	0.725v (default 0.75v for 0x20)
	ST7789_WriteCommand(0xC0);				//	LCMCTRL
	ST7789_WriteSmallData(0x2C);			//	Default value
	ST7789_WriteCommand(0xC2);				//	VDV and VRH command Enable
	ST7789_WriteSmallData(0x01);			//	Default value
	ST7789_WriteCommand(0xC3);				//	VRH set
	ST7789_WriteSmallData(0x12);		//	+-4.45v (defalut +-4.1v for 0x0B)
	ST7789_WriteCommand(0xC4);				//	VDV set
	ST7789_WriteSmallData(0x20);			//	Default value
	ST7789_WriteCommand(0xC6);			//	Frame rate control in normal mode
	ST7789_WriteSmallData(0x0F);			//	Default value (60HZ)
	ST7789_WriteCommand(0xD0);				//	Power control
	ST7789_WriteSmallData(0xA4);			//	Default value
	ST7789_WriteSmallData(0xA1);			//	Default value
	/**************** Division line ****************/

	ST7789_WriteCommand(0xE0);
	{
		uint8_t data[] = { 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C,
				0x18, 0x0D, 0x0B, 0x1F, 0x23 };
		ST7789_WriteData(data, sizeof(data));
	}

	ST7789_WriteCommand(0xE1);
	{
		uint8_t data[] = { 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51,
				0x2F, 0x1F, 0x1F, 0x20, 0x23 };
		ST7789_WriteData(data, sizeof(data));
	}
	ST7789_WriteCommand(ST7789_INVON);		//	Inversion ON
	ST7789_WriteCommand(ST7789_SLPOUT);	//	Out of sleep mode
	HAL_Delay(5);				//	Datasheet: wait 5 ms before the next command
	ST7789_WriteCommand(ST7789_NORON);		//	Normal Display on
	ST7789_WriteCommand(ST7789_DISPON);	//	Main screen turned on

	ST7789_End();
	HAL_Delay(50);
	ST7789_Fill_Color(BLACK);				//	Fill with Black.
}

/**
 * @brief Set the rotation direction of the display
 * @param m -> rotation parameter(please refer it in st7789.h)
 * @return none
 */
void ST7789_SetRotation(uint8_t m) {
	ST7789_Begin();
	ST7789_SendRotation(m);
	ST7789_End();
}

/**
 * @brief Invert Fullscreen color
 * @param invert -> Whether to invert
 * @return none
 */
void ST7789_InvertColors(uint8_t invert) {
	ST7789_Begin();
	ST7789_WriteCommand(invert ? ST7789_INVON : ST7789_INVOFF);
	ST7789_End();
}

/**
 * @brief Open/Close tearing effect line
 * @param tear -> Whether to tear
 * @return none
 */
void ST7789_TearEffect(uint8_t tear) {
	ST7789_Begin();
	if (tear) {
		ST7789_WriteCommand(ST7789_TEON);
		ST7789_WriteSmallData(0x00);	// TEM = 0: signal on V-blank only
	} else
		ST7789_WriteCommand(ST7789_TEOFF);
	ST7789_End();
}

/* ========================================================================== */
/*  Drawing functions: points and corners                                     */
/* ========================================================================== */

/**
 * @brief Fill the whole screen with a single color
 * @param color -> color to Fill with
 * @return none
 */
void ST7789_Fill_Color(uint16_t color) {
	ST7789_DrawFilledRect(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, color);
}

/**
 * @brief Draw a Pixel
 * @param x&y -> coordinate to Draw
 * @param color -> color of the Pixel
 * @return none
 */
void ST7789_DrawPixel(int16_t x, int16_t y, uint16_t color) {
	ST7789_FillArea(x, y, x, y, color);
}

/**
 * @brief Draw a line with single color
 * Consecutive pixels along the major axis are sent as one run.
 * @param x0&y0 -> coordinate of the start point
 * @param x1&y1 -> coordinate of the end point
 * @param color -> color of the line to Draw
 * @return none
 */
void ST7789_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		uint16_t color) {
	ST7789_LineRuns(x0, y0, x1, y1, ST7789_LineRunFill, &color);
}

/**
 * @brief Draw a rectangle outline (1 px) by two opposite corners
 * @param x0&y0 -> one corner (inclusive)
 * @param x1&y1 -> opposite corner (inclusive)
 * @param color -> color of the outline
 * @return none
 */
void ST7789_DrawRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		uint16_t color) {
	ST7789_FillArea(x0, y0, x1, y0, color);
	ST7789_FillArea(x0, y1, x1, y1, color);
	ST7789_FillArea(x0, y0, x0, y1, color);
	ST7789_FillArea(x1, y0, x1, y1, color);
}

/**
 * @brief Draw a filled rectangle by two opposite corners
 * @param x0&y0 -> one corner (inclusive)
 * @param x1&y1 -> opposite corner (inclusive)
 * @param color -> color of the rectangle
 * @return none
 */
void ST7789_DrawFilledRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		uint16_t color) {
	ST7789_FillArea(x0, y0, x1, y1, color);
}

/**
 * @brief Draw a Triangle with single color
 * @param xi&yi -> the 3 corners
 * @param color ->color of the lines
 * @return  none
 */
void ST7789_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		int16_t x2, int16_t y2, uint16_t color) {
	ST7789_DrawLine(x0, y0, x1, y1, color);
	ST7789_DrawLine(x1, y1, x2, y2, color);
	ST7789_DrawLine(x2, y2, x0, y0, color);
}

/**
 * @brief Draw a filled Triangle with single color
 * Each row is one span between the triangle's outline pixels, so the fill
 * covers exactly what ST7789_DrawTriangle draws for the same points.
 * @param xi&yi -> the 3 corners
 * @param color ->color of the triangle
 * @return  none
 */
void ST7789_DrawFilledTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		int16_t x2, int16_t y2, uint16_t color) {
	ST7789_Spans spans;
	int32_t y;
	int32_t ytop = y0, ybot = y0;

	if (y1 < ytop)
		ytop = y1;
	if (y2 < ytop)
		ytop = y2;
	if (y1 > ybot)
		ybot = y1;
	if (y2 > ybot)
		ybot = y2;
	if (ytop >= ST7789_HEIGHT || ybot < 0)
		return;
	if (ytop < 0)
		ytop = 0;
	if (ybot >= ST7789_HEIGHT)
		ybot = ST7789_HEIGHT - 1;

	for (y = ytop; y <= ybot; y++) {
		spans.xmin[y] = INT16_MAX;
		spans.xmax[y] = INT16_MIN;
	}

	ST7789_LineRuns(x0, y0, x1, y1, ST7789_LineRunSpan, &spans);
	ST7789_LineRuns(x1, y1, x2, y2, ST7789_LineRunSpan, &spans);
	ST7789_LineRuns(x2, y2, x0, y0, ST7789_LineRunSpan, &spans);

	for (y = ytop; y <= ybot; y++)
		if (spans.xmin[y] <= spans.xmax[y])
			ST7789_FillArea(spans.xmin[y], y, spans.xmax[y], y, color);
}

/* ========================================================================== */
/*  Drawing functions: center + size                                          */
/*  A size of w pixels spans cx - w/2 .. cx - w/2 + w - 1: odd sizes are     */
/*  symmetric, even sizes have the extra pixel on the left/top.               */
/*  A circle of radius r spans 2r+1 pixels, the same as a rectangle of        */
/*  w = h = 2r+1 with the same center.                                        */
/* ========================================================================== */

/**
 * @brief Draw a line by center and size: the diagonal of the |w| x |h| box
 * centered on (cx, cy). Same signs of w and h: top-left to bottom-right,
 * opposite signs: bottom-left to top-right. |w| = 1 or |h| = 1 gives a
 * vertical or horizontal line.
 * @param cx&cy -> center pixel
 * @param w&h -> signed width & height of the box in pixels
 * @param color -> color of the line
 * @return none
 */
void ST7789_DrawLineCentered(int16_t cx, int16_t cy, int16_t w, int16_t h,
		uint16_t color) {
	int32_t x0, y0, x1, y1;
	if (!ST7789_CenteredBox(cx, cy, ABS(w), ABS(h), &x0, &y0, &x1, &y1))
		return;
	if ((w < 0) != (h < 0))
		ST7789_LineRuns(x0, y1, x1, y0, ST7789_LineRunFill, &color);
	else
		ST7789_LineRuns(x0, y0, x1, y1, ST7789_LineRunFill, &color);
}

/**
 * @brief Draw a rectangle outline (1 px) by center and size
 * @param cx&cy -> center pixel
 * @param w&h -> outer width & height in pixels
 * @param color -> color of the outline
 * @return none
 */
void ST7789_DrawRectCentered(int16_t cx, int16_t cy, uint16_t w, uint16_t h,
		uint16_t color) {
	int32_t x0, y0, x1, y1;
	if (ST7789_CenteredBox(cx, cy, w, h, &x0, &y0, &x1, &y1))
		ST7789_DrawRect(x0, y0, x1, y1, color);
}

/**
 * @brief Draw a filled rectangle by center and size
 * @param cx&cy -> center pixel
 * @param w&h -> width & height in pixels
 * @param color -> color of the rectangle
 * @return none
 */
void ST7789_DrawFilledRectCentered(int16_t cx, int16_t cy, uint16_t w,
		uint16_t h, uint16_t color) {
	int32_t x0, y0, x1, y1;
	if (ST7789_CenteredBox(cx, cy, w, h, &x0, &y0, &x1, &y1))
		ST7789_FillArea(x0, y0, x1, y1, color);
}

/**
 * @brief Draw a circle outline by center and radius
 * @param cx&cy -> center pixel
 * @param r -> radius (the circle spans 2r+1 pixels)
 * @param color -> color of the outline
 * @return none
 */
void ST7789_DrawCircle(int16_t cx, int16_t cy, uint16_t r, uint16_t color) {
	int32_t f = 1 - (int32_t) r;
	int32_t ddF_x = 1;
	int32_t ddF_y = -2 * (int32_t) r;
	int32_t x = 0;
	int32_t y = r;
	int32_t run_start = 0;

	while (x < y) {
		if (f >= 0) {
			ST7789_CircleArcs(cx, cy, run_start, x, y, color);
			run_start = x + 1;
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
	}
	ST7789_CircleArcs(cx, cy, run_start, x, y, color);
}

/**
 * @brief Draw a filled circle by center and radius
 * Every row is sent once as a single span, at its widest extent.
 * @param cx&cy -> center pixel
 * @param r -> radius (the circle spans 2r+1 pixels)
 * @param color -> color of the circle
 * @return none
 */
void ST7789_DrawFilledCircle(int16_t cx, int16_t cy, uint16_t r, uint16_t color) {
	int32_t f = 1 - (int32_t) r;
	int32_t ddF_x = 1;
	int32_t ddF_y = -2 * (int32_t) r;
	int32_t x = 0;
	int32_t y = r;

	ST7789_FillArea(cx - y, cy, cx + y, cy, color);

	while (x < y) {
		if (f >= 0) {
			/* y is about to change, so rows cy +- y are complete:
			 * their widest span came from the current x */
			ST7789_CircleRows(cx, cy, y, x, color);
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		/* Rows cy +- x are visited only once */
		ST7789_CircleRows(cx, cy, x, y, color);
	}
	ST7789_CircleRows(cx, cy, y, x, color);
}

/**
 * @brief Draw an Image by center and size, clipped to the screen
 * @param cx&cy -> center pixel
 * @param w&h -> width & height of the Image
 * @param data -> w*h pixels, row by row, RGB565 already byte-swapped (big-endian)
 * @return none
 */
void ST7789_DrawImage(int16_t cx, int16_t cy, uint16_t w, uint16_t h,
		const uint16_t *data) {
	int32_t x0, y0, x1, y1;
	if (!ST7789_CenteredBox(cx, cy, w, h, &x0, &y0, &x1, &y1))
		return;
	int32_t vx0 = x0, vy0 = y0, vx1 = x1, vy1 = y1;
	if (!ST7789_Clip(&vx0, &vy0, &vx1, &vy1))
		return;

	uint32_t vw = vx1 - vx0 + 1;
	const uint16_t *row = data + (vy0 - y0) * w + (vx0 - x0);

	ST7789_Begin();
	ST7789_SetAddressWindow(vx0, vy0, vx1, vy1);
	if (vw == w) {
		/* Whole rows visible: they are contiguous in data */
		ST7789_WriteData((const uint8_t*) row,
				vw * (vy1 - vy0 + 1) * sizeof(uint16_t));
	} else {
		for (int32_t y = vy0; y <= vy1; y++, row += w)
			ST7789_WriteData((const uint8_t*) row, vw * sizeof(uint16_t));
	}
	ST7789_End();
}

/**
 * @brief Write a single line of text centered on a pixel
 * Horizontally the whole string is centered; vertically the capital letters
 * are. Character cells are filled with bgcolor. No wrapping: whatever falls
 * off-screen is clipped. The visible part is rendered into the pixel buffer
 * and sent as one window. Characters outside the font (printable ASCII) are
 * drawn as '?'.
 * @param cx&cy -> center pixel
 * @param str -> string to write
 * @param font -> fontstyle of the string
 * @param color -> color of the text
 * @param bgcolor -> background color of the text
 * @return none
 */
void ST7789_DrawText(int16_t cx, int16_t cy, const char *str, FontDef font,
		uint16_t color, uint16_t bgcolor) {
	int32_t len = 0;
	while (str[len])
		len++;
	if (len == 0)
		return;

	/* Top-left corner of the first character cell */
	int32_t x0 = cx - (len * font.width) / 2;
	int32_t y0 = cy - ST7789_FontCapMiddle(font);

	/* Visible part */
	int32_t vx0 = x0, vy0 = y0;
	int32_t vx1 = x0 + len * font.width - 1, vy1 = y0 + font.height - 1;
	if (!ST7789_Clip(&vx0, &vy0, &vx1, &vy1))
		return;

	uint16_t fg = ST7789_Swap16(color), bg = ST7789_Swap16(bgcolor);
	uint32_t vw = vx1 - vx0 + 1, n = 0;
	int32_t y, i, j;

	ST7789_Begin();
	ST7789_SetAddressWindow(vx0, vy0, vx1, vy1);
	ST7789_DC_Set();

	for (y = vy0; y <= vy1; y++) {
		if (n + vw > DISP_BUF_LEN) {
			ST7789_SPI_Write((uint8_t*) disp_buf, n * sizeof(uint16_t));
			n = 0;
		}
		for (i = 0; i < len; i++) {
			char ch = str[i];
			if (ch < 32 || ch > 126)
				ch = '?';
			uint32_t b = font.data[(ch - 32) * font.height + (y - y0)];
			int32_t x = x0 + i * font.width;
			for (j = 0; j < font.width; j++, x++) {
				if (x < vx0 || x > vx1)
					continue;
				disp_buf[n++] = ((b << j) & 0x8000) ? fg : bg;
			}
		}
	}
	ST7789_SPI_Write((uint8_t*) disp_buf, n * sizeof(uint16_t));

	ST7789_End();
}

/* ========================================================================== */
/*  Test                                                                      */
/* ========================================================================== */

/**
 * @brief A Simple test function for ST7789
 * @param  none
 * @return  none
 */
void ST7789_Test(void) {
	static const uint16_t colors[] = { CYAN, RED, BLUE, GREEN, YELLOW, BROWN,
	DARKBLUE, MAGENTA, LIGHTGREEN, LGRAY, LBBLUE, WHITE };
	const int16_t xc = ST7789_WIDTH / 2;
	uint32_t i;

	ST7789_Fill_Color(WHITE);
	HAL_Delay(1000);
	ST7789_DrawText(xc, 30, "Speed Test", Font_11x18, RED, WHITE);
	HAL_Delay(1000);
	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		ST7789_Fill_Color(colors[i]);
		HAL_Delay(500);
	}

	ST7789_DrawText(xc, 25, "Font test.", Font_16x26, GBLUE, WHITE);
	ST7789_DrawText(xc, 55, "Hello Steve!", Font_7x10, RED, WHITE);
	ST7789_DrawText(xc, 85, "Hello Steve!", Font_11x18, YELLOW, WHITE);
	ST7789_DrawText(xc, 115, "Hello Steve!", Font_16x26, MAGENTA, WHITE);
	HAL_Delay(1000);

	ST7789_Fill_Color(RED);
	ST7789_DrawText(xc, 15, "Rect./Line.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawRect(30, 30, 100, 100, WHITE);
	ST7789_DrawLine(30, 30, 100, 100, WHITE);
	ST7789_DrawRectCentered(165, 65, 71, 71, WHITE);
	ST7789_DrawLineCentered(165, 65, 71, -71, WHITE);
	HAL_Delay(1000);

	ST7789_Fill_Color(RED);
	ST7789_DrawText(xc, 15, "Filled Rect.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawFilledRect(30, 30, 80, 80, WHITE);
	ST7789_DrawFilledRectCentered(165, 55, 51, 51, WHITE);
	HAL_Delay(1000);

	ST7789_Fill_Color(RED);
	ST7789_DrawText(xc, 15, "Circle.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawCircle(60, 60, 25, WHITE);
	HAL_Delay(1000);

	ST7789_Fill_Color(RED);
	ST7789_DrawText(xc, 15, "Filled Cir.", Font_11x18, YELLOW, BLACK);
	ST7789_DrawFilledCircle(60, 60, 25, WHITE);
	HAL_Delay(1000);

	ST7789_Fill_Color(RED);
	ST7789_DrawText(xc, 15, "Triangle", Font_11x18, YELLOW, BLACK);
	ST7789_DrawTriangle(30, 30, 30, 70, 60, 40, WHITE);
	HAL_Delay(1000);

	ST7789_Fill_Color(RED);
	ST7789_DrawText(xc, 15, "Filled Tri", Font_11x18, YELLOW, BLACK);
	ST7789_DrawFilledTriangle(30, 30, 30, 70, 60, 40, WHITE);
	HAL_Delay(1000);

	//	If FLASH cannot storage anymore datas, please delete codes below.
	ST7789_Fill_Color(WHITE);
	ST7789_DrawImage(64, 64, 128, 128, (const uint16_t*) saber);
	HAL_Delay(3000);
}
