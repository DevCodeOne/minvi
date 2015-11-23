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
		wget_wch(stdscr, &ch); // later it has to be the focused window and use constants instead of this mess so it can be changed easier
		switch(ch) {
			case '\n' :
				insert_line_view(view);
				break;
			case 127 : 
				delete_view(view);
				break;
			case KEY_UP : 
				move_cursor(-1, 0, view); 
				break; 
			case KEY_DOWN : 
				move_cursor(1, 0, view); 
			case KEY_RIGHT : 
				move_cursor(0, 1, view); 
				break;
			case KEY_LEFT : 
				move_cursor(0, -1, view);
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
