/* Copyright (C) 2006 Pete Bevin <pete@petebevin.com>
 *
 * This file is part of Pseudoku.
 *
 * Pseudoku is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pseudoku is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pseudoku; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "solve.h"

static int ox = 70;
static int oy = 200;
static int sz = 450;

char seedbuf[100];
char *rating_str;

static void commify(char *s);
static void stripnl(char *s);

void pshead(char *seed, char *rating) {
    strcpy(seedbuf, seed);

    stripnl(seedbuf);
    commify(seedbuf);

    rating_str = rating;

    printf("%%!PS-Adobe-2.1\n");
    printf("%%%%Title: Sudoku\n");
    printf("%%%%BoundingBox: %d %d %d %d\n", ox, oy, ox+sz, 700);
    printf("%%%%DocumentFonts: Helvetica Helvetica-Bold Helvetica-Oblique\n");
    printf("%%%%EndComments\n");
}

void psgrid(int *grid, int n) {
    int i, j;
    int *p;

    printf("%%%%Page: %d %d\n", n, n);

    printf("/Helvetica-Bold findfont 48 scalefont setfont\n");
    printf("%d %d moveto\n", ox, 700);
    printf("(Puzzle %d) show\n", n);

    printf("/Helvetica-Oblique findfont 12 scalefont setfont\n");
    printf("%d %d moveto\n", ox, 50);
    printf("(http://www.pseudoku.ca/ - book #%s (%s)) show\n", seedbuf, rating_str);

    printf("3 setlinewidth\n");

    if (puztype.usesecondary) {
        for (i = 0; i < 9; i++) {
            for (j = 0; j < 9; j++) {
                int here = puztype.grid2[i*9+j];
                int west = ox + j * sz/9;
                int east = ox + (j+1) * sz/9;
                int north = oy + sz - i * sz/9;
                int south = oy + sz - (i+1) * sz/9;

                if (j > 0) {
                    int left = puztype.grid2[i*9 + (j-1)];
                    if (left != here) {
                        printf("%d %d moveto\n", west, north);
                        printf("%d %d lineto stroke\n", west, south);
                    }
                }
                if (i > 0) {
                    int up   = puztype.grid2[(i-1)*9 + j];
                    if (up != here) {
                        printf("%d %d moveto\n", west, north);
                        printf("%d %d lineto stroke\n", east, north);
                    }
                }
            }
        }
    }
    else {
        for (i = 0; i < 4; i++) {
            printf("%d %d moveto\n", ox + i * sz / 3, oy);
            printf("%d %d lineto stroke\n", ox + i * sz / 3, oy + sz);
            printf("%d %d moveto\n", ox, oy + i * sz / 3);
            printf("%d %d lineto stroke\n", ox + sz, oy + i * sz / 3);
        }
    }

    printf("1 setlinewidth\n");
    for (i = 0; i <= 9; i++) {
        printf("%d %d moveto\n", ox + i * sz / 9, oy);
        printf("%d %d lineto stroke\n", ox + i * sz / 9, oy + sz);
        printf("%d %d moveto\n", ox, oy + i * sz / 9);
        printf("%d %d lineto stroke\n", ox + sz, oy + i * sz / 9);
    }

    printf("/Helvetica findfont 36 scalefont setfont\n");
    
    p = grid;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            int val = *p++;
            int x = ox + j * sz / 9 + 0.3 * sz/9;
            int y = oy + sz - (i * sz / 9) - 0.73 * sz/9;

            if (val > 0) {
              printf("%d %d moveto\n", x, y);
              printf("(%d) show\n", val);
            }
        }
    }
    printf("showpage\n");
}

void psfoot() {
    printf("%%%%Trailer\n");
}

static void stripnl(char *s) {
    char *last = s + strlen(s) - 1;
    while (isspace(*last))
        *last-- = '\0';
}


static void commify(char *s) {
    int len = strlen(s);
    int commas = (len-1)/3;
    char *src, *dst;
    int i;

    src = s + len;
    dst = s + len + commas;

    for (i = 0; i < len; i++) {
        *dst-- = *src--;
        if (i > 1 && i % 3 == 0)
            *dst-- = ',';
    }
}
