#define _XOPEN_SOURCE_EXTENDED

#include <curses.h>
#include <wchar.h>
#include <stdlib.h>

#include "buffer_view.h"
#include "buffer.h"

buffer_view *create_buffer_view(int inital_size, int initial_line_size, WINDOW *target) {
	int max_y, max_x;
	buffer_view *view = malloc(sizeof(buffer_view));
	view->win = target; 
	view->buf = create_buffer(inital_size, initial_line_size); 

	getmaxyx(view->win, max_y, max_x);
	view->view_line = malloc(max_y * sizeof(int));
	view->view_offset = malloc(max_y * sizeof(int));
	build_view_offsets(0, view); 
	update_win(0, max_y, view);
	return view;
}

int resize_buffer_view(buffer_view *view) {
	int y, x; 
	int *new_view_line = realloc(view->view_line, y * sizeof(int)); 
	int *new_view_offset = realloc(view->view_offset, y * sizeof(int)); 
	if (new_view_line == NULL || new_view_offset == NULL) {
		if (new_view_line != NULL) free(new_view_line); 
		if (new_view_offset != NULL) free(new_view_offset);
		return -1;
	}
	view->view_line = new_view_line; 
	view->view_offset = new_view_offset;
	build_view_offsets(view->view_line[0], view); 
	// redraw window
}

void build_view_offsets(int start, buffer_view *view) {
	int max_y, max_x; 

	getmaxyx(view->win, max_y, max_x);
	int nol = view->buf->nol;
	int line = start;
	for (int i = 0; i < max_y; line++){
		if ((line < nol) && (line >= 0)) {
			int needed_rows = 1;
			if (view->buf->line[line].cursor > 0) {  
				needed_rows = (view->buf->line[line].cursor / max_x); // needed_rows = ganze reihen benötigt zum drucken des textes
				if ((view->buf->line[line].cursor % max_x) != 0) // rest zeichen 
					needed_rows++;
			}
			for (int j = 0; j < needed_rows && i < max_y; j++) {
				view->view_line[i] = line;
				view->view_offset[i] = j * max_x;
				i++; 
			}
		} else {
			view->view_line[i] = line; 
			view->view_offset[i] = 0;
			i++;
		}
	}
}

int insert_view(wchar_t value, buffer_view *view) {
	int y, x;
	int max_y, max_x;
	getyx(view->win, y, x); 
	getmaxyx(view->win, max_y, max_x);
	if (x+1 < max_x) {
		int ret = insert(value, view->buf);
		if (ret == -1) return ret;
		cchar_t tmp; 
		tmp.attr = 0; 
		tmp.chars[0] = value; 
		tmp.chars[1] = L'\0';
		wadd_wch(view->win, &tmp);	
	} else { // next row is still this line insert the line number in next view_line usw

	}
	return 0;
}

int insert_line_view(buffer_view *view) {
	int y, x; 
	int max_y, max_x; 
	
	getyx(view->win, y, x);
	getmaxyx(view->win, max_y, max_x); 

	int ret_set = set_cursor(0, 0, NEXT_LINE, view->buf);
	int ret = insert_line(view->buf);

	if (ret == -1 || ret_set == -1) return -1;

	build_view_offsets(view->view_line[0], view); // optimize this not necessary to rebuild all offsets
	align_cursor_view(view);
	update_win(y-1, max_y, view);
	return 0;
}

int delete_view(buffer_view *view) { // remember lines that are not exaclty one line long so rebuild
	int y, x; 
	int max_y, max_x; 
	getyx(view->win, y, x); 
	getmaxyx(view->win, max_y, max_x); 
	if (x-1 >= 0) {
		int ret = delete(view->buf); 
		if (ret == -1) return ret;
		wmove(view->win, y, x-1); 
		delch();
	} else { 
		if (get_selected_line(view->buf)->cursor == 0)
			return delete_line_view(view);	
		else { // append rest of the line to the previous line
			buffer_line *source = get_selected_line(view->buf); 
			int ret_set = set_cursor(0, 0, PREVIOUS_LINE, view->buf);

			if (ret_set == -1) return ret_set;

			int ret = append_line(source, view->buf); 

			if (ret == -1) return ret;
			
			set_cursor(0, 0, NEXT_LINE, view->buf);
			return delete_line_view(view);
		}
	}
	return 0;
}

int delete_line_view(buffer_view *view) {
	int y, x; 
	int max_y, max_x; 

	getyx(view->win, y, x); 
	getmaxyx(view->win, max_y, max_x); 

	int ret = delete_line(view->buf); 
	int ret_set = set_cursor(0, 0, PREVIOUS_LINE, view->buf);

	if (ret == -1 || ret_set == -1) return -1;

	set_cursor(0, 0, LINE_END, view->buf);
	build_view_offsets(view->view_line[0], view);
	align_cursor_view(view); 
	update_win(y-1, max_y, view);
}

void align_cursor_view(buffer_view *view) {
	int cursor_y, cursor_x; 
	int max_y, max_x;
	int target_y, target_x; 
	int y, x;

	getmaxyx(view->win, max_y, max_x); 
	getyx(view->win, y, x);
	cursor_y = view->buf->cursor_y; 
	cursor_x = view->buf->cursor_x; 
	// cursor_y is in view no scrolling required
	if (view->view_line[0] <= cursor_y && view->view_line[max_y-1] >= cursor_y) {
		for (int i = 0; i < max_y; i++) {
			if (cursor_y == view->view_line[i]) {
				target_y = i; // what is with chars that are not one field long \t ?
				if (cursor_x - view->view_offset[i] < max_x) {  
					target_x = cursor_x - view->view_offset[i];
					break;
				}
			}
		}
		wmove(view->win, target_y, target_x);
	} else { // TODO optimize 
		int row = 0;
		if (cursor_y < view->view_line[0]) {
			row = view->view_line[0] - 1; 
		} else {
			row = view->view_line[0] + 1;
		}

		if (row < 0) row = 0;

		scroll_view(row, view);
		align_cursor_view(view);
	}
}

int move_cursor(int rows, int cols, buffer_view *view) {
	int ret = set_cursor(rows, cols, CUR, view->buf); 
	if (ret == -1) return -1;
	align_cursor_view(view);
	return 0;
}

void scroll_view(int start_row, buffer_view *view) {
	int max_y, max_x; 
	getmaxyx(view->win, max_y, max_x);
	build_view_offsets(start_row, view);
	update_win(0, max_y, view);
}

// update terminal content fix for longer lines cleanup
void update_win(int update_start_row, int update_end_row, buffer_view *view) {
	int old_pos_y, old_pos_x;
	int max_y, max_x; 
	int y, x;
	int i, j;
	cchar_t tmp;
	tmp.chars[1] = L'\0';
	tmp.attr = 0;
	if (update_start_row < 0) update_start_row = 0;
	
	getmaxyx(view->win, max_y, max_x);
	getyx(view->win, old_pos_y, old_pos_x);
	wmove(view->win, update_start_row, 0);

	for (i = update_start_row; i < max_y && i < update_end_row; i++) {
		if (view->view_line[i] < view->buf->nol) {
			buffer_line *selected_line = &view->buf->line[view->view_line[i]]; 
			wclrtoeol(view->win);
			for (j = 0; j < max_x && j < selected_line->cursor; j++) {
				tmp.chars[0] = selected_line->line[j + view->view_offset[i]]; 
				wadd_wch(view->win, &tmp);	
			}
			if (selected_line->cursor == 0) { 
				tmp.chars[0] = L'~'; 
				wclrtoeol(view->win); 
				wadd_wch(view->win, &tmp);
			}
		} else {
			wclrtoeol(view->win); 
			tmp.chars[0] = L'~';
			wadd_wch(view->win, &tmp);
		}
		getyx(view->win, y, x);
		wmove(view->win, y+1, 0);
	}
	wmove(view->win, old_pos_y, old_pos_x);
}

void free_buffer_view(buffer_view *view) {
	if (view != NULL) {
		if (view->buf != NULL) free_buffer(view->buf); 
		if (view->view_line != NULL) free(view->view_line); 
		if (view->view_offset != NULL) free(view->view_offset);
		free(view);
	}
}
