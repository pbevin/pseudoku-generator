#include <stdio.h>
#include "dlx.h"

struct column;
struct node {
  struct node *l, *r, *u, *d;
  struct column *c;
  char rowname[24];
};
struct column {
  struct node node;
  int len;
  char name[4];
  struct column *prev, *next;
};
struct dlx {
  struct column h;
  struct column cols[324];
  struct node nodes[729 * 4];
};

void dlx_solver_init(struct dlx *solver) {
  int i, j, n;
  int c = 0, r = 0;

  solver->h.node.u = (struct node *)&solver->h;
  solver->h.node.d = (struct node *)&solver->h;
  solver->h.next = &solver->h;
  solver->h.prev = &solver->h;

  for (n = 0; n < 4; n++) {
    char letter = "PCRB"[n];
    for (i = 1; i <= 9; i++) {
      for (j = 1; j <= 9; j++) {
        struct column *col = &solver->cols[c++];
        sprintf(col->name, "%c%d%d", letter, i, j);
        col->len = 0;
        col->next = &solver->h;
        col->prev = solver->h.prev;
        col->node.u = &col->node;
        col->node.d = &col->node;
      }
    }
  }
}

void dlx_cover_column(struct dlx *solver, const char *name) {
}
void dlx_uncover_column(struct dlx *solver, const char *name) {
}
int dlx_search(struct dlx *solver) {
  return 0;
}
