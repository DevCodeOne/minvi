#ifndef BUFFER_VIEW_HEADER
#define BUFFER_VIEW_HEADER 

#include <curses.h>
#include <wchar.h>

#include "buffer.h"

#define TAB_WIDTH 3

typedef struct { // standard functions for events like resized or something like that
	WINDOW *win; 
	buffer *buf;
	int *view_line;
	int *view_offset;
} buffer_view;

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
int calculate_needed_rows(buffer_line *line, buffer_view * view);
int calculate_needed_rows_from_columns(int columns, buffer_view *view);
int calculate_needed_columns(buffer_line *line);
int get_charwidth(wchar_t ch);

// navigation functions
int move_cursor(int rows, int cols, buffer_view *view);
void align_cursor_view(buffer_view *view);

buffer_view *create_buffer_view(int inital_size, int initial_line_size, WINDOW *target); 
int resize_buffer_view(buffer_view *view);

void free_buffer_view(buffer_view *view);

#endif
