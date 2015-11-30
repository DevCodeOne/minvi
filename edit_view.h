#ifndef EDIT_VIEW_HEADER
#define EDIT_VIEW_HEADER

#include <curses.h>
#include <wchar.h>

#include "buffer_view.h"

#define VERTICAL 	0x0
#define HORIZONTAL 	0x1

#define MAX_SPLITS 	16

typedef struct {
	buffer_view **buffers;
	WINDOW *status_line;
	int focus;
} edit_view;

edit_view *create_edit_view(int inital_size, int initial_line_size); // create with file later
int handle_input(wchar_t input, edit_view *view);
void update_screen(edit_view *view);

#endif
