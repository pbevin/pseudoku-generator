#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "solve.h"

#include "generate.h"

extern int solve();
extern void initsolve();

/* Forward declarations */
static void parserange(char *s, int *min, int *max);
static int inrange(int val, int min, int max);
static void printgrid(char *gridchars, const int *grid);

static int setseed = 0;

static int minbacktracks = -1;
static int maxbacktracks = -1;

static int minclues = -1;
static int maxclues = -1;

static char ratingbuf[100];

static char gridchar[11] = ".123456789";

void generate(char *output) {
  int grid[81];
  int perm[81];
  int nsol;
  int i;
  int clues;
  int backtracks;
  extern int solver_backtracks;
  int symmetry = 1;

  struct solver solver;

  if (!setseed) {
    srandom(time(NULL));
    setseed = 1;
  }

  permute(perm, 81);

  initsolve(&solver);

  /* Build a unique puzzle by adding symmetrical pairs of clues
   * randomly to the grid.  After adding each pair, try to solve:
   * solve() returns one of 0 (overconstrained: backtrack),
   * 1 (unique solution: finished), or 2 (underconstrained: add
   * more clues).
   */
  do {
    memset(grid, 0, sizeof(grid));
    clues = 0;
    do {
      int pos, reflect;
      do {
        pos = random() % 81;
        reflect = 80 - pos;
      } while (grid[pos]);
      grid[pos] = 1 + random() % 9;
      if (symmetry)
        grid[reflect] = 1 + random() % 9;

      nsol = solve(&solver, grid);
      if (nsol == 0) {
        grid[pos] = 0;
        if (symmetry)
          grid[reflect] = 0;
      }
      else {
        if (symmetry)
          clues += (pos == reflect) ? 1 : 2;
        else
          clues++;
        backtracks = solver_backtracks;
      }
    }
    while (nsol != 1);

    /* Add cells to reduce backtracks if over limit */
    if (!symmetry && maxbacktracks != -1) {
      while (backtracks > maxbacktracks) {
        if (easier(&solver, grid)) {
          //printf("easier (bt=%d)\n", solver_backtracks);
          clues++;
        }
        backtracks = solver_backtracks;
        /*
           print_grid(grid, format, puzzle);
           solver_debug = 1;
           dumpsoln();
           solver_debug = 0;
           */
      }
      //printf("easiest (bt=%d)\n", backtracks);
    }

    /* Try removing cells in random order as long as
     * the solution is still unique.
     */
    for (i = 0; i < (symmetry ? 41 : 81); i++) {
      int pos = symmetry ? i : perm[i];
      int reflect = symmetry ? 80 - pos : -1;
      if (grid[pos]) {
        int removed = 1;
        int save1 = grid[pos];
        int save2 = 0;

        grid[pos] = 0;
        if (symmetry) {
          save2 = grid[reflect];
          grid[reflect] = 0;
        }
        if (solve(&solver, grid) != 1 || !inrange(solver.solver_backtracks, minbacktracks, maxbacktracks)) {
          grid[pos] = save1;
          if (symmetry && save2) {
            grid[reflect] = save2;
          }
          removed = 0;
        }

        if (removed) {
          clues -= symmetry ? 1 : 2;
          backtracks = solver_backtracks;
        }
      }
    }
  }
  while (!(inrange(backtracks, minbacktracks, maxbacktracks)
        && inrange(clues, minclues, maxclues)));

  printgrid(output, grid);
}

static void printgrid(char *p, const int *q) {
  int i;
  for (i = 0; i < 81; i++) {
    p[i] = gridchar[q[i]];
  }
  p[81] = 0;
}

/* Parse a ranged argument as a min and max. */
static void parserange(char *s, int *min, int *max) {
  char *p = strchr(s, '-');

  if (p == NULL)
    *min = *max = atoi(s);
  else {
    *min = atoi(s);
    *max = atoi(p+1);
  }
}

static int inrange(int v, int min, int max) {
  if (min != -1 && v < min)
    return 0;
  if (max != -1 && v > max)
    return 0;
  return 1;
}
