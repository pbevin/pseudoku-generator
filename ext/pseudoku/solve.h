#ifndef SOLVE_H
#define SOLVE_H 1

extern void initsolve();
extern int solve(int *puzzle);
extern int solver_backtracks;
extern int solver_debug;
extern int easier(int *puzzle);
extern int solution[81];
extern void permute(int *perm, int len);

/* debug */
extern void dumpsoln();

#endif /* SOLVE_H */
