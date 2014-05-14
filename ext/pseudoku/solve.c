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

#define COLS_SET 4
#define ROWS_SET 18

#define NEXTCOL(r) self->col[r][self->ncols[r]++]

int solver_debug = 0;


/* Algorithm P, Knuth section 3.4.2 */
void permute(int *P, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    P[i] = i;
  }

  /* int i; */
  /* for (i = 0; i < len; i++) { */
  /*   int j = random() % (i + 1); */
  /*   P[i] = P[j]; */
  /*   P[j] = i; */
  /* } */
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
}


/* Call before first call to solve()
*/
void initsolve(struct solver *self)
{
  int x, y, s;
  int r = 0, c;
  int base;

  memset(self->nrows, 0, sizeof(self->nrows));
  memset(self->ncols, 0, sizeof(self->ncols));

  for (x = 0; x < 9; x++) {
    for (y = 0; y < 9; y++) {
      int box = 3*(x/3) + y/3;
      for (s = 0; s < 9; s++) {
        base = 0;
        self->ncols[r] = 0;

        NEXTCOL(r) = base + x * 9 + y;
        base += 81;
        NEXTCOL(r) = base + x * 9 + s;
        base += 81;
        NEXTCOL(r) = base + y * 9 + s;
        base += 81;
        NEXTCOL(r) = base + box * 9 + s;

        r++;
      }
    }
  }

  self->colcount = base;

  for (r = 0; r < NROWS; r++) {
    int i;
    for (i = 0; i < self->ncols[r]; i++) {
      c = self->col[r][i];
      self->row[c][self->nrows[c]] = r;
      self->nrows[c]++;
#if 0
      printf("%d - %d rows (cur row %d)\n", c, nrows[c], r);
      explaincol(c);
#endif
      assert(self->nrows[c] <= ROWS_SET);
    }
  }
}



/* Best column to proceed with for quick fail */
static int mincol(struct solver *self)
{
  int min = 999;
  int i;
  int best = 0;

  for (i = 0; i < self->colcount; i++) {
    int c = i;
    int n = self->count[c];
    if (!self->colused[c] && n < min) {
      min = n;
      best = c;
      if (min < 2)
        break;
    }
  }
  dbprintf("mincol(): choosing col %d (count=%d)\n", best, min);
  self->nchoices = min;
  return best;
}

/* Recursive routine to find and count solutions. */
static int search(struct solver *self, int level, int solutions, int forcingcol, int backtracks)
{
  int c, i, j;
  int impossible = 0, reason = 0;

  if (solutions >= 2) {
    return solutions;
  }
  if (level == 81) {
    if (solutions == 0) { /* first solution */
      self->solver_backtracks = backtracks;
      for (i = 0; i < 81; i++) {
        self->solution_row[i] = self->solrow[i];
      }

      memset(self->solution, 0, sizeof(self->solution));
      for (i = 0; i < 81; i++) {
        int r = self->solrow[i];
        self->solution[r/9] = r%9+1;
      }

      /* dumpsoln(); */
    }
    return solutions + 1;
  }

  c = forcingcol ? forcingcol : mincol(self);
  dbprintf("Choosing col %d%s\n", c, forcingcol ? " (forced)" : "");
  explaincol(c);

  if (self->nchoices >= 2) {
    backtracks++;
    if (self->first_branch == -1) {
      self->first_branch = level;
    }
  }

  for (j = 0; j < self->nrows[c]; j++) {
    int r = self->row[c][j];

    if (self->rowused[r]) {
      continue;
    }

    self->solrow[level] = r;

    dbprintf("row %d (%d,%d)=%d\n", r, r/81+1, (r/9)%9+1, r%9+1);

    impossible = 0;
    forcingcol = 0;
    for (i = 0; i < self->ncols[r]; i++) {
      int c1 = self->col[r][i];
      self->colused[c1]++;
    }
    for (i = 0; i < self->ncols[r]; i++) {
      int c1 = self->col[r][i];
      int j;
      for (j = 0; j < self->nrows[c1]; j++) {
        int r1 = self->row[c1][j];
        self->rowused[r1]++;
#if 0
        dbprintf("using ");
        explainrow(r1);
#endif
        if (self->rowused[r1] == 1) {
          int k;

          for (k = 0; k < self->ncols[r1]; k++) {
            int c2;
            c2 = self->col[r1][k];
            self->count[c2]--;
            if (!self->colused[c2]) {
              int v = self->count[c2];

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
      solutions = search(self, level + 1, solutions, forcingcol, backtracks);
      if (solutions >= 2)
        break;
    }
    else {
      dbprintf("impossible: %d", reason);
      explaincol(reason);
    }

    for (i = 0; i < self->ncols[r]; i++) {
      int j;
      int c1 = self->col[r][i];
      self->colused[c1]--;
      for (j = 0; j < self->nrows[c1]; j++) {
        int r1 = self->row[c1][j];
        self->rowused[r1]--;
        if (self->rowused[r1] == 0) {
          int k;
          for (k = 0; k < self->ncols[r1]; k++) {
            int c2 = self->col[r1][k];
            self->count[c2]++;
          }
        }
      }
    }
  }
  return solutions;
}

static void initcounts(struct solver *self)
{
  int r, i;

  memset(self->count, 0, sizeof(self->count));
  for (r = 0; r < NROWS; r++) {
    if (self->rowused[r] == 0) {
      for (i = 0; i < self->ncols[r]; i++) {
        self->count[self->col[r][i]]++;
      }
    }
  }
}

static int applyclues(struct solver *self, int *puzzle)
{
  int i;
  int clues = 0;

  memset(self->rowused, 0, sizeof(self->rowused));
  memset(self->colused, 0, sizeof(self->colused));
  for (i = 0; i < 81; i++) {
    if (puzzle[i]) {
      int r, j;

      r = i * 9 + (puzzle[i] - 1);
      self->solrow[clues] = r;

      clues++;
      dbprintf("clue ");
      explainrow(r);
      for (j = 0; j < self->ncols[r]; j++) {
        int d = self->col[r][j];
        int k;
        if (self->colused[d])
          return -1;
        self->colused[d] = 1;
        for (k = 0; k < self->nrows[d]; k++) {
          self->rowused[self->row[d][k]]++;
        }
      }
    }
  }

  initcounts(self);

  return clues;
}

static int
search2(struct solver *self, int clues)
{
  self->solver_backtracks = 0;
  self->first_branch = -1;
  return search(self, clues, 0, 0, 0);
}


int
solve(struct solver *self, int *puzzle) {
  int nsol;
  int clues = applyclues(self, puzzle);
  /* printgrid(puzzle, stdout); */
  if (clues == -1)
    return 0;
  nsol = search2(self, clues);

  return nsol;
}

int
easier(struct solver *self, int *puzzle)
{
  int n;
  int clues = applyclues(self, puzzle);
  self->solver_backtracks = 0;
  self->first_branch = -1;
  n = search2(self, clues);
  assert(n == 1);
  if (self->first_branch != -1) {
    int r = self->solution_row[self->first_branch];
    int cell = r/9;
    int newval = r%9+1;

    dbprintf("first_branch = %d; r = %d\n", self->first_branch, r);

    dbprintf("changing puzzle[%d] from %d to %d\n", cell, puzzle[cell], newval);
    puzzle[cell] = newval;
    return 1;
  }
  return 0;
}
