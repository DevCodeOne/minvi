#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>
#include <ncurses.h>

#include "buffer.h"

const wchar_t *abc = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_";
const wchar_t *special_chars = L"!\"$%&/()=?`'*+-\'";

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
	if (column >= buf->line[row].cursor || column < 0) 
		return 0;
	return 1;
}

void clip(int *row, int *column, buffer *buf) {
	if (*row < 0) *row = 0; 
	if (*row > buf->nol) *row = buf->nol-1; 
	if (*column > buf->line[*row].cursor) *column = buf->line[*row].cursor;
	if (*column < 0) *column = 0;
}

// origin indepenpent x and y so parameters for x and y
int set_cursor(int row, int column, int origin, buffer *buf) {
	int target_x = buf->cursor_x, target_y = buf->cursor_y;

	switch(origin & 0xFFFF) { // y 
		case CUR_Y : 
			target_y += row;
			break;
		case FIRST_LINE : 
			target_y = row;
			break; 
		case LAST_LINE : 
			target_y = buf->nol - 1 + row; // fix this last written line
			break; 
		case NEXT_LINE : 
			target_y++;
			break;
		case PREVIOUS_LINE :
			target_y--;
			break;
		default :
			break;
	}

	if (is_in_bounds(target_y, target_x, buf)) {
		buf->cursor_y = target_y; 
	} else { 
		// invalid cursor location -> clipped
		clip(&target_y, &target_x, buf);
		buf->cursor_y = target_y; 
	}

	switch(origin & (0xFFFF << 16)) { // x
		case CUR_X : 
			target_x += column;
			break; 
		case LINE_START : 
			target_x = column;
			break; 
		case LINE_END : 
			target_x = buf->line[buf->cursor_y].cursor + column;
			break;
		case NEXT_WORD : 
			// implement
			break;
		case PREVIOUS_WORD : 
			// implement
			break;
		default : 
			break;

	}

	if (is_in_bounds(target_y, target_x, buf)) {
		buf->cursor_y = target_y; 
		buf->cursor_x = target_x;
		return 0; 
	} else { 
		// invalid cursor location -> clipped
		clip(&target_y, &target_x, buf);
		buf->cursor_y = target_y; 
		buf->cursor_x = target_x;
		return 1;
	}
}

void next_word(int *x, int *y, buffer *buf) {
	bool in_word = 0;
	wchar_t *c = &buf->line[buf->cursor_y].line[buf->cursor_x];	
	
	if (*c == L' ' || *c == L'\t') { // not in word
		wchar_t *next_word = wcspbrk(buf->line[buf->cursor_y].line, abc); 
		wchar_t *next_special_word = wcspbrk(buf->line[buf->cursor_y].line, special_chars); 

		if (next_word < next_special_word && next_word != NULL) { // jump to next word
			
		} else if (next_special_word != NULL) {

		}
	} else { // jump to first blank char then to the next char
		wchar_t *occurence = wcschr(abc, *c); 
		if (occurence != NULL) { // in abc
					
		} else {
			
		}
	}
}

void previous_word(int *x, int *y, buffer *buf) {

}

int insert(wchar_t value, buffer *buf) {
	buffer_line *line = &buf->line[buf->cursor_y];

	if (line->size - line->cursor < 2) {
		int ret = resize_buffer_line(line->size * 2, line);
		if (ret == -1) return ret;
	}
	
	wchar_t *start = line->line + buf->cursor_x;

	if (buf->cursor_x+1 < line->cursor) // no characters behind this one
		wmemmove(start+1, start, line->cursor - (buf->cursor_x)); 
	
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


	wchar_t *start = line->line + buf->cursor_x;

	if (line->cursor > buf->cursor_x)
		wmemmove(start, start+1, line->cursor - (buf->cursor_x)); 
	
	line->cursor--;
	return 0;
}

int insert_line(buffer *buf) { // error here
	if (buf->line[buf->nol-1].cursor != 0 || buf->cursor_y >= buf->nol-2) { 
		int ret = resize_buffer(buf->nol * 2, buf->initial_line_size, buf); 
		if (ret == -1) return ret; 
	}

	buffer_line tmp = buf->line[buf->nol-1];
	buffer_line *start = &buf->line[buf->cursor_y];
	
	memmove(start + 1, start, (buf->nol - (buf->cursor_y + 1)) * sizeof(buffer_line)); 
	buf->line[buf->cursor_y] = tmp;	
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

int append_line(buffer_line *src, buffer *buf) { // reuse copy_line
	buffer_line *selected_line = get_selected_line(buf);
	return copy_line(selected_line, src, selected_line->cursor, 0, src->cursor);
}

int copy_line(buffer_line *dst, buffer_line *src, int dst_offset, int src_offset, int len) {
	if (src->size < src_offset + len) return -1; 
	if (dst->size < dst_offset + len) { 
		int ret = resize_buffer_line(dst->size + (len * 2), dst);
		if (ret == -1) return ret;
	}

	wmemcpy(&dst->line[dst_offset], &src->line[src_offset], len);
	dst->cursor += len;
	return 0;
}

int delete_range(buffer_line *line, int off, int len) {
	if (line->cursor < off + len) 
		return -1; // nothing to remove

	wchar_t *start = &line->line[off];

	wmemmove(start, start+len, len); 
	line->cursor -= len;
	return 0;

}

buffer_line *get_selected_line(buffer *buf) {
	return &buf->line[buf->cursor_y];
}
