#define _XOPEN_SOURCE_EXTENDED

#include <curses.h>
#include <locale.h>
#include <stdbool.h>

#include "buffer_view.h"
#include "buffer.h"

bool quit = 0;
int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	buffer_view *view = create_buffer_view(16, 64, stdscr);
	
	while(!quit) {
		wchar_t ch = L'\0';
		wget_wch(stdscr, &ch); // later it has to be the focused window
		switch(ch) {
			case 127 : // delete
				view_delete(view);
				break;
			case '\n' : // enter ? 
				view_set_cursor(1, -, CUR, view);
				printf("shiiiet!");
				break;
			default: 
				view_insert(ch, view);
		}	

	}
	view_print_buffer(view);
	refresh();
	getch();
	endwin();

	return 0;
}
