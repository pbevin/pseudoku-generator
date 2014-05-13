#ifndef SOLVE_H
#define SOLVE_H 1

struct puztype {
    int diagonals;  /* Constrain diagonals to have numbers 1-9 */
    int useprimary;
    int usesecondary;
    int grid2[81];  /* Secondary grid for irregular sudoku */
};

extern struct puztype puztype;

extern void initsolve();
extern int solve(int *puzzle);
extern int solver_backtracks;
extern int solver_debug;
extern int easier(int *puzzle);

/* debug */
extern void dumpsoln();

#endif /* SOLVE_H */
