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

#define BOX(r, c) (3 * (r / 3) + (c / 3))
#define P(r, c) (9 * (r-1) + (c-1))
#define R(r, d) (9 * (r-1) + (d-1) + 81)
#define C(c, d) (9 * (c-1) + (d-1) + 2*81)
#define B(b, d) (9 * (b-1) + (d-1) + 3*81)

void cover(struct dlx *solver, struct column *c) {
  struct node *i, *j;

  c->next->prev = c->prev;
  c->prev->next = c->next;

  for (i = c->node.d; i != &c->node; i = i->d) {
    for (j = i->r; j != i; j = j->r) {
      j->d->u = j->u;
      j->u->d = j->d;
      j->c->len--;
    }
  }
}

void uncover(struct dlx *solver, struct column *c) {
  struct node *i, *j;

  for (i = c->node.u; i != &c->node; i = i->u) {
    for (j = i->l; j != i; j = j->l) {
      j->c->len++;
      j->d->u = j;
      j->u->d = j;
    }
  }

  c->prev->next = c;
  c->next->prev = c;
}

int search(struct dlx *solver) {
  return 0;
}

void mkcols(struct dlx *solver) {
  int n, i, j;
  int c = 0;

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

void mkrow(struct dlx *solver, const int colnums[4], const int nextnode) {
  int i;
  struct node *nodes[4];

  for (i = 0; i < 4; i++) {
    nodes[i] = &solver->nodes[nextnode + i];
  }

  for (i = 0; i < 4; i++) {
    struct node *node = nodes[i];
    struct column *col = &solver->cols[colnums[i]];
    node->c = col;
    node->u = col->node.u;
    node->d = &col->node;
    col->node.u->d = node;
    col->node.u = node;
    col->len++;

    node->l = nodes[(i+3)%4];
    node->r = nodes[(i+1)%4];
  }
}

void mkrows(struct dlx *solver) {
  int r, c, d;
  int n = 0;
  int nextnode = 0;
  int colnums[4];
  for (r = 1; r <= 9; r++) {
    for (c = 1; c <= 9; c++) {
      for (d = 1; d <= 9; d++) {
        colnums[0] = P(r, c);
        colnums[1] = R(r, d);
        colnums[2] = C(c, d);
        colnums[3] = B(BOX(r, c), d);
        mkrow(solver, colnums, nextnode);
        nextnode += 4;
      }
    }
  }
}

void dlx_solver_init(struct dlx *solver) {
  solver->h.node.u = &solver->h.node;
  solver->h.node.d = &solver->h.node;
  solver->h.next = &solver->h;
  solver->h.prev = &solver->h;

  mkcols(solver);
  mkrows(solver);

}

int dlx_solve(struct dlx *solver, const char clues[81]) {
  for (int i = 0; i < 81; i++) {
    int r = i / 9;
    int c = i % 9;
    int b = BOX(r, c);
    int d = clues[i] - '0';
    if (clues[i] == '.') continue;

    cover(solver, &solver->cols[P(r, c)]);
    cover(solver, &solver->cols[R(r, d)]);
    cover(solver, &solver->cols[C(c, d)]);
    cover(solver, &solver->cols[B(b, d)]);
  }

  return 1;
}

