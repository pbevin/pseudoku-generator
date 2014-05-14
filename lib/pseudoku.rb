require "pseudoku/version"
require "pseudoku/pseudoku"

class Pseudoku
  DIGITS = %w[ 1 2 3 4 5 6 7 8 9 ]

  def initialize
    nodebug
  end

  def generate2
    pare_down(random_puzzle)
  end

  def random_puzzle
    symmetric = true
    grid = "." * 81

    loop do
      begin
        pos = rand(81)
        reflect = 80 - pos
      end while grid[pos] != '.'

      grid[pos] = DIGITS.sample
      grid[reflect] = DIGITS.sample if symmetric

      case solve(grid)
      when nil
        # No solution, back out
        grid[pos] = '.'
        grid[reflect] = '.' if symmetric
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
        new_grid = grid.dup
        new_grid[pos] = '.'
        new_grid[reflect] = '.'
        
        if solve(new_grid)
          grid = new_grid
        end
      end
    end

    grid
  end
end
