#define _XOPEN_SOURCE_EXTENDED

#include <curses.h>
#include <wchar.h>
#include <stdlib.h>

#include "buffer_view.h"
#include "buffer.h"

buffer_view *create_buffer_view(int inital_size, int initial_line_size, WINDOW *target) {
	buffer_view *view = malloc(sizeof(buffer_view));
	view->win = target; 
	view->buf = create_buffer(inital_size, initial_line_size); 
	return view;
}

int view_insert(wchar_t value, buffer_view *view) {
	cchar_t tmp; 
	tmp.attr = 0; 
	tmp.chars[0] = value; 
	tmp.chars[1] = L'\0';
	int ret = insert(value, view->buf); 
	if (ret == -1) return ret;
	wadd_wch(view->win, &tmp); 
	return ret;
}

int view_replace(wchar_t value, buffer_view *view) {
	cchar_t tmp; 
	tmp.attr = 0; 
	tmp.chars[0] = value; 
	tmp.chars[1] = L'\0';

	wdelch(view->win); 
	wadd_wch(view->win, &tmp); 
	replace(value, view->buf);
	return 0;
}

int view_delete(buffer_view *view) {
	int y, x;
	getyx(view->win, y, x); 
	if (x != 0) {
		int ret = delete(view->buf);
		if (ret == -1) return ret;
		wmove(view->win, y, x-1);
		wdelch(view->win); 
		return ret;
	}
	if (y > 0) {
		buffer_line *line = &view->buf->line[view->buf->cursor_y-1];
		view_set_cursor(-1, line->cursor, CUR, view);
	}
	return -1;
}

int view_delete_line(buffer_view *view) {

}

int view_insert_line(buffer_view *view) {

}

int view_set_cursor(int row, int column, int origin, buffer_view *view) {
	int y, x; 
	getyx(view->win, y, x); 
	if (origin == CUR) {
		int ret = set_cursor(row, column, origin, view->buf);
		if (ret == -1) return ret; 
		wmove(view->win, y+row, x+column);
	} else { // jump line for later
	}
}

void view_print_buffer(buffer_view *view) {
	int y, x; 
	int max_y, max_x;
	getmaxyx(view->win, max_y, max_x);
	for (int i = 0; i < view->buf->nol && i < max_y; i++) {
		for (int j = 0; j < view->buf->line[i].cursor && j < max_x; j++) {
			cchar_t tmp;
			tmp.attr = 0; 
			tmp.chars[0] = view->buf->line[i].line[j];
			tmp.chars[1] = L'\0';
			wadd_wch(view->win, &tmp); 
		}
		getyx(view->win, y, x); 
		wmove(view->win, y+1, 0);
	}
}
