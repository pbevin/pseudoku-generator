#include <stdio.h>
#include "dlx.h"

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
  struct node *solnodes[81];
  char solution[82];
};

#define BOX(r, c) (1 + 3 * ((r-1) / 3) + ((c-1) / 3))
#define P(r, c) (9 * (r-1) + (c-1))
#define C(c, d) (9 * (c-1) + (d-1) + 81)
#define R(r, d) (9 * (r-1) + (d-1) + 2*81)
#define B(b, d) (9 * (b-1) + (d-1) + 3*81)

static void cover(struct dlx *solver, struct column *c) {
  /* printf("COVER %s\n", c->name); */
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

static void uncover(struct dlx *solver, struct column *c) {
  /* printf("UNCOVER %s\n", c->name); */
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

static void printsoln(struct dlx *solver, int k) {
  int i;
  for (i = 0; i < k; i++) {
    int k = (solver->solnodes[i] - solver->nodes) / 4;
    int i = k / 9;
    int d = 1 + k % 9;

    solver->solution[i] = '0' + d;
  }
}

static int search(struct dlx *solver, int k) {
  printf("k=%d\n", k);
  int solutions = 0;
  /* printf("k = %d\n", k); */
  struct column *c = solver->h.next;
  struct node *r, *j;

  if (c == &solver->h) {
    printsoln(solver, k);
    return 1;
  }

  /* printf("CHOOSE %s\n", c->name); */
  cover(solver, c);

  for (r = c->node.d; r != &c->node; r = r->d) {
    solver->solnodes[k] = r;
    for (j = r->r; j != r; j = j->r) {
      cover(solver, j->c);
    }
    int rc = search(solver, k+1);
    solutions += search(solver, k+1);
    if (k == 0) {
      printf("Solutions increased by %d to %d\n", rc, solutions);
    }

    for (j = r->l; j != r; j = j->l) {
      uncover(solver, j->c);
    }
  }
  uncover(solver, c);

  return solutions;
}

static void mkcols(struct dlx *solver) {
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
        solver->h.prev->next = col;
        solver->h.prev = col;
      }
    }
  }
  /* printf("c = %d\n", c); */
}

static void mkrow(struct dlx *solver, const int colnums[4], const int nextnode) {
  int i;
  struct node *nodes[4];

  /* printf("nn = %d\n", nextnode); */

  for (i = 0; i < 4; i++) {
    nodes[i] = &solver->nodes[nextnode + i];
  }

  for (i = 0; i < 4; i++) {
    /* printf("colnum = %d\n", colnums[i]); */
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

  /* printf("done setup\n"); */
}

void mkrows(struct dlx *solver) {
  int r, c, d;
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

static void cover_clues(struct dlx *solver, const char clues[81], void (*fn)(struct dlx *, struct column *), int dir) {
  for (int n = 0; n < 81; n++) {
    int i = (dir > 0) ? n : 80-n;

    if (clues[i] == '.') continue;

    solver->solution[i] = clues[i];

    int r = 1 + i / 9;
    int c = 1 + i % 9;
    int d = clues[i] - '0';
    int b = BOX(r, c);

    if (dir > 0) {
      fn(solver, &solver->cols[P(r, c)]);
      fn(solver, &solver->cols[C(c, d)]);
      fn(solver, &solver->cols[R(r, d)]);
      fn(solver, &solver->cols[B(b, d)]);
    } else {
      fn(solver, &solver->cols[B(b, d)]);
      fn(solver, &solver->cols[R(r, d)]);
      fn(solver, &solver->cols[C(c, d)]);
      fn(solver, &solver->cols[P(r, c)]);
    }
  }
}

int dlx_solve(struct dlx *solver, const char clues[81]) {
  printf("dlx_solve\n");
  cover_clues(solver, clues, cover, 1);
  solver->solution[81] = '\0';

  int rc = search(solver, 0);

  cover_clues(solver, clues, uncover, -1);
  return rc;
}

const char *dlx_solution(struct dlx *solver) {
  return solver->solution;
}

#ifdef MAIN
int
main() {
  const char *puz = "...5...8......1..2..5.9...4.6..34..9.38...26.2..61..5.9...2.3..6..8......4...5...";

  struct dlx solver;
  dlx_solver_init(&solver);
  dlx_solve(&solver, puz);
  dlx_solve(&solver, puz);
  return 0;
}
#endif
