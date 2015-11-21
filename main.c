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
			case '\n' :
				insert_line_view(view);
				break;
			case 127 : 
				delete_view(view);
				break;
			default : 
				insert_view(ch, view);
		}	

	}
	refresh();
	getch();
	endwin();

	return 0;
}
