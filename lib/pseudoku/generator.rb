module Pseudoku

  class Generator
    DIGITS = %w[ 1 2 3 4 5 6 7 8 9 ]

    def initialize(solver=Solver.new)
      @solver = solver
    end

    def generate
      pare_down(random_puzzle)
    end

    def random_puzzle
      grid = "." * 81

      loop do
        begin
          pos = rand(81)
          reflect = 80 - pos
        end while grid[pos] != '.'

        grid[pos] = DIGITS.sample
        grid[reflect] = DIGITS.sample

        case @solver.solve(grid)
        when nil
          # No solution, back out
          grid[pos] = '.'
          grid[reflect] = '.'
        when false
          # multiple solutions, keep going
        else
          break
        end
      end

      grid
    end

    def pare_down(grid)
      # Try removing cells in random order as long as
      # the solution is still unique.

      (0..40).to_a.shuffle.each do |pos|
        reflect = 80 - pos
        if grid[pos]
          a, b = grid[pos], grid[reflect]
          grid[pos] = grid[reflect] = "."

          if !@solver.solve(grid)
            grid[pos], grid[reflect] = a, b
          end
        end
      end

      grid
    end
  end
end
