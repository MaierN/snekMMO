#include "display.h"

#include <stdio.h>

void display_clear() {
    printf("\033[2J");
}

void display_write(int x, int y, char* text) {
    x += 40;
    printf("\033[%d;%dH%s", y+1, x+1, text);
}

void display_show(int x, int y) {
    x += 40;
    printf("\033[%d;%dH", y+1, x+1);
    fflush(stdout);
}
