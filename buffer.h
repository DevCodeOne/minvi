#include <wchar.h>

#ifndef BUFFER_HEADER
#define BUFFER_HEADER

#define CUR 0x0
#define START 0x1
#define END 0x2

typedef struct {
	int cursor; 
	int size;
	wchar_t *line;
} buffer_line;

typedef struct {
	int cursor_x, cursor_y; 
	int nol; // number of lines
	buffer_line *line;	
} buffer;

int insert(wchar_t value, buffer *buf); 
int replace(wchar_t value, buffer *buf);
int delete(buffer *buf); 
int delete_line(buffer *buf); 
int insert_line(buffer *buf); 
int set_cursor(int row, int column, int origin, buffer *buf); 
int is_in_bounds(int row, int column, buffer *buf); 

buffer *create_buffer(int initial_size, int initial_line_size); 
int resize_buffer(int new_size, int initial_line_size, buffer *buf); 
int resize_buffer_line(int new_size, buffer_line *line); 

void free_buffer(buffer *buf); 

#endif
