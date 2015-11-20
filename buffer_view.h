#include <curses.h>
#include <wchar.h>

#ifndef BUFFER_VIEW_HEADER
#define BUFFER_VIEW_HEADER 

#include "buffer.h"

typedef struct { // standard functions for events like resized or something like that
	WINDOW *win; 
	buffer *buf;
} buffer_view;

void view_print_buffer(buffer_view *view);
int view_insert(wchar_t value, buffer_view *view); 
int view_replace(wchar_t value, buffer_view *view); 
int view_delete(buffer_view *view);
int view_delete_line(buffer_view *view); 
int view_insert_line(buffer_view *view);
int view_set_cursor(int row, int column, int origin, buffer_view *buf); 

buffer_view *create_buffer_view(int inital_size, int initial_line_size, WINDOW *target); 

void free_buffer_view(buffer_view *view);

#endif
