#include <ctype.h>
#include <stdio.h>
#include "grid.h"

void
readgrid(char *s, int *grid)
{
    int i;
    for (i = 0; i < 81 && *s; i++) {
        char ch = *s++;
        if (isdigit(ch))
            grid[i] = ch - '0';
        else
            grid[i] = 0;
    }
}

void
rotate_grid(int *grid)
{
    int temp[81];
    int sr, sc;
    int i;

    for (sr = 0; sr < 9; sr++) {
        for (sc = 0; sc < 9; sc++) {
            int dr = sc;
            int dc = 8 - sr;
            temp[dr*9+dc] = grid[sr*9+sc];
        }
    }

    for (i = 0; i < 81; i++) {
        grid[i] = temp[i];
    }
}

void
printgrid(int *grid, FILE *fp)
{
    int i;

    for (i = 0; i < 81; i++) {
        char ch;
        if (grid[i] == 0)
            ch = '.';
        else
            ch = grid[i] + '0';
        fputc(ch, fp);
    }
    fputc('\n', fp);
}
