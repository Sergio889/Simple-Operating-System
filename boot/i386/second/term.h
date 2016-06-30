/*
 * Simple terminal-handling capabilities.
 */
#ifndef TERM
#define TERM
#include "../../../libcommon/com.h"
#include "../../../libcommon/elf32.h"
#define TRUE 1
#define NIL 0
#define ESC 1
#define ENTER 2
#define TAB 3
#define NL 4
#define CTRL 5
#define LSHIFT 6
#define RSHIFT 6
#define ALT 7
#define CAPS 9
#define F1 10
#define F2 11
#define F3 12
#define F4 13
#define F5 14
#define F6 15
#define F7 16
#define F8 17
#define F9 18
#define F10 19
#define NUMLOCK 20
#define SCRLOCK 21
#define HOME 22
#define CURUP 23
#define PGUP 24
#define PGDOWN 25
#define LEFT 26
#define MID 26
#define RIGHT 27
#define END 28
#define DOWN 29
#define DELETE 31
#define INSERT -1
#define F11 -34
#define F12 -35
#define EXT_KEY -36
#define CTRL_SCAN 0x1d
#define LSHIFT_SCAN 42
#define RSHIFT_SCAN 54
#define QUEUE_LEN 512
#define COLOR 0x3d4
#define COLOR_VRAM 0xb8000
#define MONO 0x3b4
#define MONO_VRAM 0xc8000
#define SHIFT_HOLD 0x2
#define ANY_SPEC_MASK 0xfb
#define CTRL_MASK 0x01
#define SHIFT_MASK 0x02
#define UPDOWN 0x04
#define ALT_MASK 0x08
#define FALSE 0
#define ASCII 0
/* vector for pages' start address */
static uint_t lin_cur_pos[] = { 0, 80, 160, 240, 320, 400, 480, 560, 640, 720, 800, 880, 960, 1040, 1120, 1200, 1280, 1360, 1440, 1520, 1600, 1680, 1760, 1840, 1920, 2000, 2080, 2160, 2240, 2320, 2400, 2480, 2560, 2640, 2720, 2800, 2880, 2960, 3040, 3120, 3200, 3280, 3360, 3440, 3520, 3600, 3680, 3760, 3840, 3920, 4000, 4080, 4160, 4240, 4320, 4400, 4480, 4560, 4640, 4720, 4800, 4880, 4960, 5040, 5120, 5200, 5280, 5360, 5440, 5520, 5600, 5680, 5760, 5840, 5920, 6000, 6080, 6160, 6240, 6320, 6400, 6480, 6560, 6640, 6720, 6800, 6880, 6960, 7040, 7120, 7200, 7280, 7360, 7440, 7520, 7600, 7680, 7760, 7840, 7920, 8000, 8080, 8160, 8240, 8320, 8400, 8480, 8560, 8640, 8720, 8800, 8880, 8960, 9040, 9120, 9200, 9280, 9360, 9440, 9520, 9600, 9680, 9760, 9840, 9920, 10000, 10080, 10160, 10240, 10320, 10400, 10480, 10560, 10640, 10720, 10800, 10880, 10960, 11040, 11120, 11200, 11280, 11360, 11440, 11520, 11600, 11680, 11760, 11840, 11920, 12000, 12080, 12160, 12240, 12320, 12400, 12480, 12560, 12640, 12720, 12800, 12880, 12960, 13040, 13120, 13200, 13280, 13360, 13440, 13520, 13600, 13680, 13760, 13840, 13920, 14000, 14080, 14160, 14240, 14320, 14400, 14480, 14560, 14640, 14720, 14800, 14880, 14960, 15040, 15120, 15200, 15280, 15360, 15440, 15520, 15600, 15680, 15760, 15840, 15920, 16000 };
/* map vectors */
static unsigned char map_vector[] = { NIL, ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x9, TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', ENTER, CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ' ', '\\', '`', LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', RSHIFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLOCK, SCRLOCK, HOME, CURUP, PGUP, '-', LEFT, MID, RIGHT, '+', END, DOWN, PGDOWN, INSERT, DELETE, ENTER, NIL, NIL, F11, F12, NIL, NIL, NIL, NIL, NIL, NIL, NIL, EXT_KEY };
static unsigned char shift_map_vector[] = { NIL, ESC, '!', '"', '#', '$', '%', '&', '/', '(', ')', '=', '_', '=', 0x9, TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', ENTER, CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ' ', '\\', '`', LSHIFT, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '?', RSHIFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLOCK, SCRLOCK, HOME, CURUP, PGUP, '?', LEFT, MID, RIGHT, '+', END, DOWN, PGDOWN, INSERT, DELETE, ENTER, NIL, NIL, F11, F12, NIL, NIL, NIL, NIL, NIL, NIL, NIL, EXT_KEY };

/*
 * Objects created from this structures provides a reasonable power for
 * terminal-handling.
 * Needless to say, the structure allows us to create objects with enough
 * power for our needs. Who says it's not possible to do object-orientation
 * with any language doesn't really understand the paradigm.
 */
struct tty_SOS {
	/*
	 * Data
	 */
	byte tty_key_holds; // bit map for keys being hold
	byte tty_com_buf[512]; // buffer to store logical line characters
	byte *tty_que_buf; // buffer to store logical line characters
	ubyte_t *tty_ptr_tail_que;
	uint_t tty_num_chars; // number of characters
	uint_t tty_type; // mono, color...
	uint_t tty_cur_line; // current line
	uint_t tty_cur_line_scr; // current line screen
	byte *tty_cur_addr;
	uint_t tty_base_vram;
	uint_t tty_cur_pos; // position in line relative to tty_base_vram
	uint_t tty_cursor; // cursor's position

	/*
	 * Functions
	 */
	void (*tty_init)(); // initialize VRAM space
	void (*tty_updown)(char); // scroll screen
	void (*tty_write)(unsigned char); // write characters to terminal
	void (*tty_unwrite)(); // 'delete' one character from terminal
	void (*tty_inter_spec)(char);
	void (*tty_clear_screen)();
	void (*tty_clear_line)();
	void (*tty_que_char)(char);
} simple_tty;
#endif
