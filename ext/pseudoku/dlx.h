struct dlx;

extern void dlx_solver_init(struct dlx *solver);
extern int dlx_solve(struct dlx *solver, const char clues[81]);
