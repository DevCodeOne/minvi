#include <wchar.h>

#ifndef BUFFER_HEADER
#define BUFFER_HEADER

// new ones
// x and y
#define CUR 			CUR_X | CUR_Y	
#define START			LINE_START | FIRST_LINE
#define END			LINE_END | LAST_LINE

// x only
#define CUR_X			0x0 << 16
#define LINE_START 		0x1 << 16
#define LINE_END 		0x2 << 16
#define NEXT_WORD 		0x3 << 16
#define PREVIOUS_WORD		0x4 << 16

// y only
#define CUR_Y 			0x0
#define FIRST_LINE 		0x1
#define LAST_LINE		0x2
#define NEXT_LINE		0x3
#define PREVIOUS_LINE		0x4


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
int delete_range(buffer_line *line, int off, int len);

buffer_line *get_selected_line(buffer *buf);

buffer *create_buffer(int initial_size, int initial_line_size); 
int resize_buffer(int new_size, int initial_line_size, buffer *buf); 
int resize_buffer_line(int new_size, buffer_line *line); 

void free_buffer(buffer *buf); 

#endif
