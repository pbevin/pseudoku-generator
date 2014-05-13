#ifndef GRID_H
#define GRID_H 1

#include <stdio.h>

extern void readgrid(char *s, int *grid);
extern void rotate_grid(int *grid);
extern void printgrid(int *grid, FILE *fp);

#endif /* GRID_H */
