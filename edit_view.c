#include <stdlib.h>
#include <curses.h>
#include <wchar.h>

#include "edit_view.h"
#include "buffer_view.h"

edit_view *create_edit_view(int inital_size, int initial_line_size) {	
	int max_y, max_x; 
	getmaxyx(stdscr, max_y, max_x);

	WINDOW *buffer_window = newwin(max_y - 2, max_x, 0, 0);
	edit_view *view = malloc(sizeof(edit_view));
	view->buffers = malloc(MAX_SPLITS * sizeof(buffer_view *));
	view->buffers[0] = create_buffer_view(inital_size, initial_line_size, buffer_window);
	view->status_line = newwin(1, max_x, max_y - 1, 0); 
	view->focus = 0; 
	return view;
}

int handle_input(wchar_t input, edit_view *view) {
	buffer_view *focused_view = view->buffers[view->focus];
	int ret = 0;

	if (focused_view->mode != NORMAL_MODE) { // keys handled by buffer_view in focus
		 ret = handle_input_view(input, focused_view);	
	} else {
		if (input == L':') { // command

		} else {
			ret = handle_input_view(input, focused_view);	
		}
	}

	wclear(view->status_line);

	char *mode = "NORMAL";
	switch(focused_view->mode) {
		case NORMAL_MODE : 
			mode = "NORMAL";
			break; 
		case INPUT_MODE : 
			mode = "INPUT"; 
			break;
		case VISUAL_MODE : 
			mode = "VISUAL";
			break;
		case REPLACE_MODE : 
			mode = "REPLACE";
			break;
	}
	wprintw(view->status_line, "%s ", mode);
	wmove(focused_view->win, focused_view->cursor_y, focused_view->cursor_x); 
	return ret;
}

void update_screen(edit_view *view) {
	buffer_view *focused_view = view->buffers[view->focus];
	for (int i = 0; i < MAX_SPLITS; i++) {
		if (view->buffers[i] != NULL) {
			wrefresh(view->buffers[i]->win);
		}
	}
	wrefresh(view->status_line);
	wmove(focused_view->win, focused_view->cursor_y, focused_view->cursor_x); 
}
