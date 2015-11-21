#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>
#include <ncurses.h>

#include "buffer.h"

buffer *create_buffer(int initial_size, int initial_line_size) {
	buffer *buf = malloc(sizeof(buffer));
	buf->cursor_x = 0; 
	buf->cursor_y = 0; 
	buf->nol = initial_size;
	buf->initial_line_size = initial_line_size;
	buf->line = malloc(initial_size * sizeof(buffer_line));
	if (buf->line == NULL) {
		free_buffer(buf); 
		return NULL;
	}
	for (int i = 0; i < buf->nol; i++) {
		buf->line[i].cursor = 0; 
		buf->line[i].size = initial_line_size;
		buf->line[i].line = malloc(initial_line_size * sizeof(wchar_t));
		memset(buf->line[i].line, '\0', initial_line_size * sizeof(wchar_t));
		if (buf->line[i].line == NULL) {
			free_buffer(buf); 
			return NULL;
		} 
	}
	return buf;
}

int resize_buffer(int new_size, int initial_line_size, buffer *buf) {
	buffer_line *new_line = realloc(buf->line, new_size * sizeof(buffer_line));
	if (new_line == NULL) {
		return -1;
	}
	buf->line = new_line;
	for (int i = buf->nol; i < new_size; i++) {
		new_line[i].cursor = 0; 
		new_line[i].size = initial_line_size; 
		new_line[i].line = malloc(initial_line_size * sizeof(wchar_t));
		if (buf->line[i].line == NULL) {
			buf->nol = i;
			return -1;
		}
		memset(buf->line[i].line, '\0', initial_line_size * sizeof(wchar_t));
	} 
	buf->nol = new_size; 
	return 0;
}

int resize_buffer_line(int new_size, buffer_line *line) {
	wchar_t *new_line = realloc(line->line, new_size * sizeof(wchar_t));
	if (new_line == NULL) {
		return -1;
	}
	line->size = new_size; 
	line->line = new_line;
	return 0;

}

void free_buffer(buffer *buf) {
	if (buf != NULL) {
		if (buf->line != NULL) {
			for (int i = 0; i < buf->nol; i++) {
				if (buf->line[i].line != NULL) {	
					free(buf->line[i].line);
				}
			}
			free(buf->line);
		}
		free(buf);
	}
}

int is_in_bounds(int row, int column, buffer *buf) {
	if (row > buf->nol || row < 0) 
		return 0; 
	if (column >= buf->line[row].size || column < 0) 
		return 0;
	return 1;
}

int set_cursor(int row, int column, int origin, buffer *buf) {
	int target_x = buf->cursor_x, target_y = buf->cursor_y;
	if (origin == CUR) { // change to switch ?
		target_y += row; 
		target_x += column; 
	} else if (origin == START) {
		target_y = row; 
		target_x = column; 
	} else if (origin == END) { // this offset is not correct end of the file != end of the buffer fix it later
		target_y = (buf->nol-1) + row; 
		target_x = column;
	} else if (origin == LINE_START) {
		target_y = buf->cursor_y; 
		target_x = 0;
	} else if (origin == LINE_END) {
		target_y = buf->cursor_y; 
		target_x = buf->line[buf->cursor_y].cursor;
	} else if (origin == NEXT_LINE) {
		target_y = buf->cursor_y+1; 
		target_x = 0;
	} else if (origin == PREVIOUS_LINE) {
		target_y = buf->cursor_y-1; 
		target_x = 0;
	}
	if (is_in_bounds(target_y, target_x, buf)) {
		buf->cursor_y = target_y; 
		if (target_x > buf->line[target_x].cursor) 
			buf->cursor_x = buf->line[target_y].cursor;
		else 
			buf->cursor_x = target_x;
		return 0;
	} 
	// invalid cursor location 
	return -1;
}

int insert(wchar_t value, buffer *buf) {
	buffer_line *line = &buf->line[buf->cursor_y];
	if (line->size - line->cursor < 2) {
		int ret = resize_buffer_line(line->size * 2, line);
		if (ret == -1) return ret;
	}
	wchar_t *start = line->line + buf->cursor_x;
	if (buf->cursor_x+1 < line->cursor) // no characters behind this one
		wmemmove(start+1, start, line->cursor - (buf->cursor_x + 1)); 
	line->line[buf->cursor_x++] = value;
	line->cursor++;
	return 0;
}

int replace(wchar_t value, buffer *buf) {
	buf->line[buf->cursor_y].line[buf->cursor_x] = value;
	return 0;
}

int delete(buffer *buf) {
	buffer_line *line = &buf->line[buf->cursor_y];
	if (line->cursor == 0) 
		return -1; // nothing to remove
	line->cursor--;
	wchar_t *start = line->line + buf->cursor_x;
	if (line->cursor > buf->cursor_x)
		wmemmove(start, start+1, line->cursor - (buf->cursor_x - 1)); 
	if (buf->cursor_x > line->cursor)  
		buf->cursor_x = line->cursor;
	return 0;
}

int insert_line(buffer *buf) {
	int old_nol = buf->nol; // if resized the other lines dont have to be moved they are empty
	if (buf->line[buf->nol-1].cursor != 0) { // line is not empty resize buffer if possible
		int ret = resize_buffer(buf->nol * 2, buf->initial_line_size, buf); 
		if (ret == -1) return ret; 
	}
	buffer_line tmp = buf->line[old_nol-1];
	buffer_line *start = &buf->line[buf->cursor_y];
	memmove(start+1, start, (old_nol - (buf->cursor_y + 1)) * sizeof(buffer_line)); 
	memcpy(&buf->line[old_nol-1], &tmp, sizeof(buffer_line));
	return 0;
}

int delete_line(buffer *buf) {
	if (buf->cursor_y == 0) return -1;
	buffer_line tmp = buf->line[buf->cursor_y];
	buffer_line *start = &buf->line[buf->cursor_y];
	memmove(start, start+1, (buf->nol - (buf->cursor_y + 1)) * sizeof(buffer_line));
	memcpy(&buf->line[buf->nol-1], &tmp, sizeof(buffer_line));
	buf->line[buf->nol-1].cursor = 0;
	return 0;
}
