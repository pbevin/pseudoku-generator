#ifndef SOLVE_H
#define SOLVE_H 1

#define NROWS 729
#define NCOLS 500

extern void permute(int *perm, int len);

struct solver {
  short row[NCOLS][9];
  short col[NROWS][7];
  short ncols[NROWS];
  short nrows[NCOLS];
  int colcount;

  int solver_backtracks;
  int first_branch;

  int rowused[NROWS], colused[NCOLS], count[NCOLS];
  int solrow[81];
  int solution[81];
  int solution_row[81];
  int nchoices;
};

extern void initsolve(struct solver *self);
extern int solve(struct solver *self, int *puzzle);
extern int easier(struct solver *self, int *puzzle);


#endif /* SOLVE_H */
