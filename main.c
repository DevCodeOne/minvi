#define _XOPEN_SOURCE_EXTENDED

#include <curses.h>
#include <locale.h>
#include <stdbool.h>

#include "buffer_view.h"
#include "edit_view.h"
#include "buffer.h"

bool quit = 0;
int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	set_tabsize(3); // init somewhere different later
	edit_view *view = create_edit_view(16, 8);
	
	while(!quit) {
		wchar_t ch = L'\0';
		
		update_screen(view);

		wget_wch(view->buffers[0]->win, &ch); // later it has to be the focused window and use constants instead of this mess so it can be changed easier
		handle_input(ch, view);
	}
	endwin();

	return 0;
}
