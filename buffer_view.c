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
	view->mode = 0;
	view->cursor_x = 0; 
	view->cursor_y = 0;
	build_view_offsets(0, view); 
	update_win(0, max_y, view);
	return view;
}

int resize_buffer_view(buffer_view *view) {
	int max_y, max_x;

	getmaxyx(view->win, max_y, max_x);

	int *new_view_line = realloc(view->view_line, max_y * sizeof(int)); 
	int *new_view_offset = realloc(view->view_offset, max_y * sizeof(int)); 

	if (new_view_line == NULL || new_view_offset == NULL) {
		if (new_view_line != NULL) free(new_view_line); 
		if (new_view_offset != NULL) free(new_view_offset);
		return -1;
	}

	view->view_line = new_view_line; 
	view->view_offset = new_view_offset;
	build_view_offsets(view->view_line[0], view); 
	update_win(0, max_y, view); 
	return 0;
}

void build_view_offsets(int start, buffer_view *view) {
	int max_y, max_x; 

	getmaxyx(view->win, max_y, max_x);
	int nol = view->buf->nol;
	int line = start;

	max_x--;
	for (int i = 0; i < max_y; line++){
		if ((line < nol) && (line >= 0)) {
			buffer_line *selected_line = &view->buf->line[line];
			int columns = 0;
			view->view_offset[i] = 0; 
			view->view_line[i] = line;
			i++;

			for (int j = 0; j < selected_line->cursor; j++) {
				int char_width = get_charwidth(selected_line->line[j]);

				columns += char_width;
				if (columns > max_x) {
					columns = 0; 
					view->view_offset[i] = j--; // one char too much jump back
					view->view_line[i] = line;
					i++;
				}
			}
		} else {
			view->view_line[i] = line; 
			view->view_offset[i] = 0;
			i++;
		}
	}
}

int insert_view(wchar_t value, buffer_view *view) { // bug with tabs not jumping to next line when max_x is reached
	int y, x;
	int max_y, max_x;
	char update_complete_screen = 0;

	getyx(view->win, y, x); 
	getmaxyx(view->win, max_y, max_x);

	max_x--;
	int ret = insert(value, view->buf);
	if (ret == -1) return ret;

	if (x+1 >= max_x) { // next row is stil this line 
		update_complete_screen = 1;  	
		build_view_offsets(view->view_offset[0], view);
	}
		
	align_cursor_view(view);
	update_win(update_complete_screen ? 0 : y, max_y, view); // efficient updating pls
	return 0;
}

int insert_line_view(buffer_view *view) {
	int y, x; 
	int max_y, max_x; 
	
	getyx(view->win, y, x);
	getmaxyx(view->win, max_y, max_x); 

	int old_cursor_x = view->buf->cursor_x;
	buffer_line *previous_line = get_selected_line(view->buf);	

	// insert rest of the previous line in the next line
	set_cursor(0, 0, NEXT_LINE, view->buf);
	
	int ret = insert_line(view->buf); 
	
	if (ret == -1) return -1;
	
	buffer_line *sel_line = get_selected_line(view->buf);
	copy_line(sel_line, previous_line, 0, old_cursor_x, previous_line->cursor - old_cursor_x); 
	delete_range(previous_line, old_cursor_x, previous_line->cursor - old_cursor_x); 
	
	build_view_offsets(view->view_line[0], view); // optimize this not necessary to rebuild all offsets
	align_cursor_view(view);
	update_win(y-1, max_y, view);
	return 0;
}

int delete_view(buffer_view *view) { // fix just because x is at 0 doesnt mean start of the line
	int y, x; 
	int max_y, max_x; 
	
	getyx(view->win, y, x); 
	getmaxyx(view->win, max_y, max_x); 
	if (x-1 >= 0) {
		set_cursor(0, -1, CUR, view->buf);

		int ret = delete(view->buf); 
		if (ret == -1) return -1;

		build_view_offsets(view->view_line[0], view); 
		align_cursor_view(view);
		update_win(y-1, max_y, view);
	} else { 
		if (get_selected_line(view->buf)->cursor == 0)
			return delete_line_view(view);	
		else { // append rest of the line to the previous line
			buffer_line *source = get_selected_line(view->buf); 
			set_cursor(0, 0, PREVIOUS_LINE, view->buf);

			if (view->buf->cursor_y > 0) {
				int ret = append_line(source, view->buf); 

				if (ret == -1) return ret;
				set_cursor(0, 0, NEXT_LINE, view->buf);
				return delete_line_view(view);
			}
			return 0;
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
	set_cursor(0, 0, PREVIOUS_LINE, view->buf);

	if (ret == -1) return -1;
	
	set_cursor(0, 0, LINE_END, view->buf);
	build_view_offsets(view->view_line[0], view);
	align_cursor_view(view); 
	update_win(y-1, max_y, view);
}

void align_cursor_view(buffer_view *view) { // bug relating tabs is here
	int cursor_y, cursor_x; 
	int max_y, max_x;
	int target_y, target_x; 
	int y, x;

	getmaxyx(view->win, max_y, max_x); 
	getyx(view->win, y, x);
	cursor_y = view->buf->cursor_y; 
	cursor_x = view->buf->cursor_x; 

	max_x--;

	if (view->view_line[0] <= cursor_y && view->view_line[max_y-1] >= cursor_y) {
		for (int i = 0; i < max_y; i++) {
			if (cursor_y == view->view_line[i]) {
				target_y = i; 
				buffer_line *line = &view->buf->line[view->view_line[i]];

				int pos = view->view_offset[i];

				for (target_x = 0; pos < cursor_x; ){
					int char_width = 0;
					switch(line->line[pos]) {
						case L'\t' :
							char_width = TAB_WIDTH; 
							break;
						default : 
						 	char_width = 1;	
					}
					if (target_x + char_width >= max_x) {
						break;
					} else {
						target_x += char_width; 
						pos++;
					}
				}
				if (pos == cursor_x) break;
			}
		}
		wmove(view->win, target_y, target_x);
		view->cursor_x = target_x;
		view->cursor_y = target_y;

	} else { // TODO optimize scroll more than one line 
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

	max_x--; // otherwise the placement of the cursor and some other things is fucked up
	for (i = update_start_row; i < max_y && i < update_end_row; i++) {
		if (view->view_line[i] < view->buf->nol) {
			buffer_line *selected_line = &view->buf->line[view->view_line[i]]; 
			int off = view->view_offset[i];
			wclrtoeol(view->win);
			for (j = 0; j < max_x && off < selected_line->cursor; ){
				wchar_t c = selected_line->line[off++];
				switch(c) {
					case L'\t' : 
						tmp.chars[0] = L' ';
						for (int k = 0; k < TAB_WIDTH; k++) 
							wadd_wch(view->win, &tmp);
						j += TAB_WIDTH;
						break; 
					default : 
						tmp.chars[0] = c; 
						wadd_wch(view->win, &tmp);	
						j++;
				}
			}

			if (selected_line->cursor == 0) { 
				tmp.chars[0] = L'~'; 
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

int calculate_needed_rows(buffer_line *line, buffer_view *view) { // maybe return struct with offsets
	int columns = calculate_needed_columns(line); 
	
	return calculate_needed_rows_from_columns(columns, view);
}

int calculate_needed_rows_from_columns(int columns, buffer_view *view) {
	int max_y, max_x;	

	getmaxyx(view->win, max_y, max_x); 
	max_x--;

	if (columns < max_x) 
		return 1;
	return (columns / max_x) + ((!(columns % max_x == 0)) ? 1 : 0);
}

int calculate_needed_columns(buffer_line *line) {
	int columns = 0;
	for (int i = 0; i < line->cursor; i++) {
		switch(line->line[i]) {
			case L'\t' : 
				columns += TAB_WIDTH;
				break;
			default : 
				columns++;
		}

	}
	return columns;
}

int get_charwidth(wchar_t value) {
	switch(value) {
		case L'\t' : 
			return TAB_WIDTH; 
			break; // unnecessary
		default : 
			return 1;
	}
}

int handle_input_view(wchar_t value, buffer_view *view) {
	if (view->mode == NORMAL_MODE) {
		switch(value) {
			case L'i' : 
				view->mode = INPUT_MODE;	
				break;
			case L'a' : 
				view->mode = INPUT_MODE; // move cursor one to the right
				break;
			case L'v' : 
				view->mode = VISUAL_MODE; // not yet implemented
				break;
			case L'r' : 
				view->mode = REPLACE_MODE;
				break;
			case L'h' : // move left
				move_cursor(0, -1, view);
				break;
			case L'l' : // move right 
				move_cursor(0, 1, view);
				break;
			case L'j' : // move down 
				move_cursor(1, 0, view);
				break;
			case L'k' : // move up 
				move_cursor(-1, 0, view);
				break;
			case L'x' : // delete one char
				move_cursor(0, 1, view); 
				delete_view(view); 
			break;
		}
	} else if (view->mode == INPUT_MODE) {
		switch(value) {
			case 27 : // escape
				view->mode = NORMAL_MODE;
				break;
			case 10 : // enter (new line)
				insert_line_view(view);
				break; 
			case 127 : // delete
				delete_view(view); 
				break;
			default : 
				insert_view(value, view);

		}
	} else if (view->mode == REPLACE_MODE) {
		view->mode = NORMAL_MODE;
	} else if (view->mode == VISUAL_MODE) {
		view->mode = NORMAL_MODE;
	}
}
