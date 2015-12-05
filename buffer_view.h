#ifndef BUFFER_VIEW_HEADER
#define BUFFER_VIEW_HEADER 

#include <curses.h>
#include <wchar.h>

#include "buffer.h"

#define TAB_WIDTH 3

#define NORMAL_MODE			0x0
#define INPUT_MODE			0x1
#define VISUAL_MODE			0x2
#define REPLACE_MODE 			0x3

typedef struct { // standard functions for events like resized or something like that
	WINDOW *win; 
	buffer *buf;
	int *view_line;
	int *view_offset;
	int mode;
	int cursor_x, cursor_y;
} buffer_view;

int handle_input_view(wchar_t value, buffer_view *view);

// editing functions 
int insert_view(wchar_t value, buffer_view *view); 
int insert_line_view(buffer_view *view);
int delete_view(buffer_view *view);
int delete_line_view(buffer_view *view); 
int replace_view(wchar_t value, buffer_view *view); 

// screen updating functions
void update_win(int update_start_row, int update_end_row, buffer_view *view);
void build_view_offsets(int start, buffer_view *view);
void scroll_view(int start_row, buffer_view *view);
int get_charwidth(wchar_t ch);

// navigation functions
int move_cursor(int rows, int column, int origin, buffer_view *view);
void align_cursor_view(buffer_view *view);

buffer_view *create_buffer_view(int inital_size, int initial_line_size, WINDOW *target); 
int resize_buffer_view(buffer_view *view);

void free_buffer_view(buffer_view *view);

#endif
