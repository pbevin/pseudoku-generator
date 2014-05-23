struct dlx;

extern void dlx_solver_init(struct dlx *solver);
extern void dlx_add_column(struct dlx *solver, const char *name);
extern void dlx_cover_column(struct dlx *solver, const char *name);
extern void dlx_uncover_column(struct dlx *solver, const char *name);
extern int dlx_search(struct dlx *solver);
