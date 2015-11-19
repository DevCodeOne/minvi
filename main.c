#include <stdio.h>

#include "buffer.h"

int main(int argc, char *argv[]) {
	buffer *buf = create_buffer(16, 64);
	insert(L'H', buf); 
	insert(L'E', buf); 
	insert(L'L', buf); 
	insert(L'L', buf); 
	insert(L'O', buf);
	set_cursor(1, 0, START, buf);
	insert(L'W', buf); 
	insert(L'O', buf); 
	insert(L'R', buf); 
	insert(L'L', buf); 
	insert(L'D', buf);
	set_cursor(0, 0, START, buf);
	delete_line(buf);
	printf("cursor : %d, %d \n", buf->cursor_y, buf->cursor_x);
	for (int i = 0; i < buf->nol; i++) {
		for (int j = 0; j < buf->line[i].size; j++) {
			putwchar(buf->line[i].line[j]); 
		}	
		printf("\t %p\n", buf->line[i].line);
	} 
	return 0;
}
