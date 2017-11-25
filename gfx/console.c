#include "console.h"

static framebuffer *_framebuffer;

#define FONT_PPM_WIDTH 240
#define FONT_PPM_HEIGHT 240
#define FONT_CHAR_WIDTH 15
#define FONT_CHAR_HEIGHT 15
#define FONT_PPM_HEADER_SIZE 15
#define FONT_MATRIX_WIDTH 16
#define FONT_MATRIX_HEIGHT 16

extern char _binary_gfx_font_ppm_start;

typedef struct {
	uint8_t *buffer;
	int width;
	int height;
} bitmap;
bitmap font_bitmap;

static int console_width;
static int console_height;

static int console_x = 0;
static int console_y = 0;

void console_init(framebuffer *framebuffer) {
	_framebuffer = framebuffer;
        
        font_bitmap.buffer = (uint8_t *)(&_binary_gfx_font_ppm_start + FONT_PPM_HEADER_SIZE);
	font_bitmap.width = FONT_PPM_WIDTH;
	font_bitmap.height = FONT_PPM_HEIGHT;

	console_width = framebuffer->width / FONT_CHAR_WIDTH;
	console_height = framebuffer->height / FONT_CHAR_HEIGHT;
}

static uint32_t encode_pixel(int x, int y) {
	union {
		uint8_t bytes[4];
		uint32_t int32_value;
	} encoder;
        
	uint8_t *px = font_bitmap.buffer + (y * font_bitmap.width + x) * 3;
        
	encoder.bytes[0] = px[0];
	encoder.bytes[1] = px[1];
	encoder.bytes[2] = px[2];
	encoder.bytes[3] = 0xFF;
        
	return encoder.int32_value;
}

void put_pixel(unsigned x, unsigned y, uint32_t px)
{
	int32_t *fb_base = (int32_t *) _framebuffer->base;
	int32_t *pixel_address = fb_base + y * _framebuffer->pixelsPerScanline + x;
	*pixel_address = px;
}

static void safe_put_pixel(unsigned x, unsigned y, uint32_t px) {
	if (x < _framebuffer->width && x < _framebuffer->height) {
            put_pixel(x, y, px);
	}
}

static void put_bits(unsigned x, int y, unsigned map_x, unsigned map_y, 
        unsigned char_width, unsigned char_height)
{
    for (unsigned i = 0; i < char_height; ++i) {
        for (unsigned j = 0; j < char_width; ++j) {
            uint32_t px = encode_pixel(map_x + j, map_y + i);
            safe_put_pixel(x + j, y + i, px);
        }
    }
}    

static void put_char(int x, int y, uint8_t character) {
    int map_x = character / FONT_MATRIX_WIDTH;
    int map_y = character / FONT_MATRIX_HEIGHT;
    
    int pixel_x = map_x * FONT_CHAR_WIDTH;
    int pixel_y = map_y * FONT_CHAR_HEIGHT;
    
    put_bits(x * FONT_CHAR_WIDTH, y * FONT_CHAR_HEIGHT, 
        pixel_x, pixel_y,
        FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT);
}

static void clean_line(int y) {
	for(int x = 0; x < console_width; ++x) {
		put_char(x, y, ' ');
	}
}

static void print_char(uint8_t character) {
	if (character == '\n') {
		console_y = (console_y + 1) % console_height;
		clean_line(console_y);
		console_x = 0;
	} else {
		put_char(console_x, console_y, character);
		++console_x;
	}
}

void console_print(const char *message) {
	const uint8_t *character = (const uint8_t *) message;
        
	while(*character) {
		print_char(*character);
		++character;
	}
}
