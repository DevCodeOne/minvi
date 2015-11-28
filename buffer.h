#include <wchar.h>

#ifndef BUFFER_HEADER
#define BUFFER_HEADER

#define CUR 		0x0
#define START 		0x1
#define END 		0x2
#define LINE_START 	0x3
#define LINE_END 	0x4
#define NEXT_LINE 	0x5
#define PREVIOUS_LINE 	0x6 

typedef struct {
	int cursor; 
	int size;
	wchar_t *line;
} buffer_line;

typedef struct {
	int cursor_x, cursor_y; 
	int nol; // number of lines
	int initial_line_size;
	buffer_line *line;	
} buffer;

int insert(wchar_t value, buffer *buf); 
int append_line(buffer_line *line, buffer *buf);
int replace(wchar_t value, buffer *buf);
int delete(buffer *buf); 
int delete_line(buffer *buf); 
int insert_line(buffer *buf); 
int set_cursor(int row, int column, int origin, buffer *buf); 
int is_in_bounds(int row, int column, buffer *buf); 

int copy_line(buffer_line *dst, buffer_line *src, int dst_offset, int src_offset, int len); 
int delete_from_to(buffer_line *line, int off, int len);

buffer_line *get_selected_line(buffer *buf);

buffer *create_buffer(int initial_size, int initial_line_size); 
int resize_buffer(int new_size, int initial_line_size, buffer *buf); 
int resize_buffer_line(int new_size, buffer_line *line); 

void free_buffer(buffer *buf); 

#endif
