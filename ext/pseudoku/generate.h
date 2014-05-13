struct stats {
  int backtracks;
  int clues;
};

extern void generate(void(*callback)(const char *grid, struct stats *stats));
