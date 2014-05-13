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
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <memory.h>
#include <stdarg.h>
#include "solve.h"

#define NROWS 729
#define NCOLS 500
#define COLS_SET 4
#define ROWS_SET 18

#define NEXTCOL(r) col[r][ncols[r]++]

static short row[NCOLS][9];
static short col[NROWS][7];
static short ncols[NROWS];
static short nrows[NCOLS];
static int colcount;
static int colperm[NCOLS];
int solver_debug = 0;

struct puztype puztype = { 0, 1, 0 };

int solver_backtracks;
int first_branch;


/* Algorithm P, Knuth section 3.4.2 */
static void permute(int *P, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    int j = random() % (i + 1);
    P[i] = P[j];
    P[j] = i;
  }
}

static int dbprintf(char *fmt, ...)
{
  va_list ap;
  int n = 0;

  va_start(ap, fmt);
  if (solver_debug) {
    n = vprintf(fmt, ap);
  }
  va_end(ap);
  return n;
}

static void explainrow(int r)
{
  dbprintf("row %d (%d,%d)=%d\n", r, r/81+1, (r/9)%9+1, r%9+1);
}

static void explaincol(int c)
{
  dbprintf("==> ");
  if (c < 81) {
    dbprintf("row %d col %d\n", c/9+1, c%9+1);
  }
  else if (c < 162) {
    c -= 81;
    dbprintf("row %d val %d\n", c/9+1, c%9+1);
  }
  else if (c < 243) {
    c -= 162;
    dbprintf("col %d val %d\n", c/9+1, c%9+1);
  }
  else if (c < 324) {
    c -= 243;
    dbprintf("box %d val %d\n", c/9+1, c%9+1);
  }
  else if (c < 405) {
    c -= 324;
    dbprintf("box2 %d val %d\n", c/9+1, c%9+1);
  }
  else if (c < 414) {
    c -= 405;
    dbprintf("diag \\ val %d\n", c+1);
  }
  else {
    c -= 414;
    dbprintf("diag / val %d\n", c+1);
  }
}


/* Call before first call to solve()
*/
void initsolve()
{
  int x, y, s;
  int r = 0, c;
  int base;

  memset(nrows, 0, sizeof(nrows));
  memset(ncols, 0, sizeof(ncols));

  for (x = 0; x < 9; x++) {
    for (y = 0; y < 9; y++) {
      int box = 3*(x/3) + y/3;
      for (s = 0; s < 9; s++) {
        base = 0;
        ncols[r] = 0;

        NEXTCOL(r) = base + x * 9 + y;
        base += 81;
        NEXTCOL(r) = base + x * 9 + s;
        base += 81;
        NEXTCOL(r) = base + y * 9 + s;
        base += 81;
        NEXTCOL(r) = base + box * 9 + s;
        base += 81;

        if (puztype.usesecondary) {
          int box2 = puztype.grid2[x*9+y] - 1;
          NEXTCOL(r) = base + box2 * 9 + s;
          base += 81;
        }

        if (puztype.diagonals) {
          if (x == y) {  /* diagonal A (\) */
            NEXTCOL(r) = base + s;
          }
          if (x == 8-y) { /* diagonal B (/) */
            NEXTCOL(r) = base + 9 + s;
          }
          base += 18;
        }

        r++;
      }
    }
  }

  colcount = base;

  permute(colperm, colcount);

  for (r = 0; r < NROWS; r++) {
    int i;
    for (i = 0; i < ncols[r]; i++) {
      c = col[r][i];
      row[c][nrows[c]] = r;
      nrows[c]++;
#if 0
      printf("%d - %d rows (cur row %d)\n", c, nrows[c], r);
      explaincol(c);
#endif
      assert(nrows[c] <= ROWS_SET);
    }
  }
}

static int rowused[NROWS], colused[NCOLS], count[NCOLS];
static int solrow[81];
int solution[81];
int solution_row[81];
static int nchoices;


/* Best column to proceed with for quick fail */
static int mincol()
{
  int min = 999;
  int i;
  int best = 0;

  for (i = 0; i < colcount; i++) {
    int c = colperm[i];
    int n = count[c];
    if (!colused[c] && n < min) {
      min = n;
      best = c;
      if (min < 2)
        break;
    }
  }
  dbprintf("mincol(): choosing col %d (count=%d)\n", best, min);
  nchoices = min;
  return best;
}

  void
dumpsoln()
{
  int i;

  for (i = 0; i < 81; i++) {
    dbprintf("%d", solution[i]);
    if (i % 9 == 8)
      dbprintf("\n");
  }
}

/* Recursive routine to find and count solutions. */
static int search(int level, int solutions, int forcingcol, int backtracks)
{
  int c, i, j;
  int impossible = 0, reason = 0;

  if (solutions >= 2) {
    return solutions;
  }
  if (level == 81) {
    if (solutions == 0) { /* first solution */
      solver_backtracks = backtracks;
      for (i = 0; i < 81; i++) {
        solution_row[i] = solrow[i];
      }

      memset(solution, 0, sizeof(solution));
      for (i = 0; i < 81; i++) {
        int r = solrow[i];
        solution[r/9] = r%9+1;
      }

      dumpsoln();
    }
    return solutions + 1;
  }

  c = forcingcol ? forcingcol : mincol();
  dbprintf("Choosing col %d%s\n", c, forcingcol ? " (forced)" : "");
  explaincol(c);

  if (nchoices >= 2) {
    backtracks++;
    if (first_branch == -1) {
      first_branch = level;
    }
  }

  for (j = 0; j < nrows[c]; j++) {
    int r = row[c][j];

    if (rowused[r]) {
      continue;
    }

    solrow[level] = r;

    dbprintf("row %d (%d,%d)=%d\n", r, r/81+1, (r/9)%9+1, r%9+1);

    impossible = 0;
    forcingcol = 0;
    for (i = 0; i < ncols[r]; i++) {
      int c1 = col[r][i];
      colused[c1]++;
    }
    for (i = 0; i < ncols[r]; i++) {
      int c1 = col[r][i];
      int j;
      for (j = 0; j < nrows[c1]; j++) {
        int r1 = row[c1][j];
        rowused[r1]++;
#if 0
        dbprintf("using ");
        explainrow(r1);
#endif
        if (rowused[r1] == 1) {
          int k;

          for (k = 0; k < ncols[r1]; k++) {
            int c2;
            c2 = col[r1][k];
            count[c2]--;
            if (!colused[c2]) {
              int v = count[c2];

#if 0
              dbprintf("reduced %d to %d ", c2, v);
              explaincol(c2);
#endif

              if (v == 0) {
                impossible = 1;
                reason = c2;
              }
              else if (v  == 1) {
                forcingcol = c2;
              }
            }
          }
        }
      }
    }

    if (!impossible) {
      solutions = search(level + 1, solutions, forcingcol, backtracks);
      if (solutions >= 2)
        break;
    }
    else {
      dbprintf("impossible: %d", reason);
      explaincol(reason);
    }

    for (i = 0; i < ncols[r]; i++) {
      int j;
      int c1 = col[r][i];
      colused[c1]--;
      for (j = 0; j < nrows[c1]; j++) {
        int r1 = row[c1][j];
        rowused[r1]--;
        if (rowused[r1] == 0) {
          int k;
          for (k = 0; k < ncols[r1]; k++) {
            int c2 = col[r1][k];
            count[c2]++;
          }
        }
      }
    }
  }
  return solutions;
}

static void initcounts()
{
  int r, i;

  memset(count, 0, sizeof(count));
  for (r = 0; r < NROWS; r++) {
    if (rowused[r] == 0) {
      for (i = 0; i < ncols[r]; i++) {
        count[col[r][i]]++;
      }
    }
  }
}

static int applyclues(int *puzzle)
{
  int i;
  int clues = 0;

  memset(rowused, 0, sizeof(rowused));
  memset(colused, 0, sizeof(colused));
  for (i = 0; i < 81; i++) {
    if (puzzle[i]) {
      int r, j;

      r = i * 9 + (puzzle[i] - 1);
      solrow[clues] = r;

      clues++;
      dbprintf("clue ");
      explainrow(r);
      for (j = 0; j < ncols[r]; j++) {
        int d = col[r][j];
        int k;
        if (colused[d])
          return -1;
        colused[d] = 1;
        for (k = 0; k < nrows[d]; k++) {
          rowused[row[d][k]]++;
        }
      }
    }
  }

  initcounts();

  return clues;
}

  static int
search2(int clues)
{
  solver_backtracks = 0;
  first_branch = -1;
  return search(clues, 0, 0, 0);
}


int solve(int *puzzle) {
  int nsol;
  int clues = applyclues(puzzle);
  /* printgrid(puzzle, stdout); */
  if (clues == -1)
    return 0;
  nsol = search2(clues);

  return nsol;
}

  int
easier(int *puzzle)
{
  int n;
  int clues = applyclues(puzzle);
  solver_backtracks = 0;
  first_branch = -1;
  n = search2(clues);
  assert(n == 1);
  if (first_branch != -1) {
    int r = solution_row[first_branch];
    int cell = r/9;
    int newval = r%9+1;

    dbprintf("first_branch = %d; r = %d\n", first_branch, r);

    dbprintf("changing puzzle[%d] from %d to %d\n", cell, puzzle[cell], newval);
    puzzle[cell] = newval;
    return 1;
  }
  return 0;
}
