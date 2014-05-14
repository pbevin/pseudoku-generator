require 'pseudoku'

require 'rspec/expectations'

RSpec::Matchers.define :be_a_solution_of do |puzzle|
  match do |solution|
    solution && solution.length == puzzle.length && (0..solution.length).all? do |i|
      puzzle[i] == '.' || puzzle[i] == solution[i]
    end
  end
end


describe Pseudoku do
  let(:pseudoku) { subject }

  let(:easy_puzzle)   { "...5...8......1..2..5.9...4.6..34..9.38...26.2..61..5.9...2.3..6..8......4...5..." }
  let(:easy_solution) { "124563987796481532385792614561234879438957261279618453957126348613849725842375196" }
  let(:hard_puzzle)   { "..1.....8.4...79..2...9.1....4.....9.6.874.2.5.....4....5.3...4..35...1.9.....6.." }
  let(:hard_solution) { "791652348346187952258493176824315769169874523537926481675231894483569217912748635" }
  let(:unsolvable)    { "5..5...8......1..2..5.9...4.6..34..9.38...26.2..61..5.9...2.3..6..8......4...5..." }
  let(:no_clues) { "." * 81 }

  describe "an easy puzzle" do

    it "has a solution" do
      expect(pseudoku.solve easy_puzzle).to eq(easy_solution)
    end

    it "has 0 backtracks" do
      pseudoku.solve easy_puzzle
      expect(pseudoku.backtracks).to eq(0)
    end

  end

  describe "a hard puzzle" do

    it "has a solution" do
      expect(pseudoku.solve hard_puzzle).to eq(hard_solution)
    end

    it "has 1 backtrack" do
      expect(pseudoku.backtracks).to eq(1)
    end

  end

  describe "an unsolvable puzzle" do

    it "has no solution" do
      expect(pseudoku.solve unsolvable).to be_nil
    end

  end

  describe "a puzzle with no clues" do

    it "has no solution" do
      expect(pseudoku.solve no_clues).to eq(false)
    end

  end

  describe '#generate' do
    let(:puzzle1) { pseudoku.generate2 }
    let(:puzzle2) { pseudoku.generate2 }

    it "generates a puzzle" do
      expect(puzzle1.length).to eq(81)
    end

    it "generates different puzzles" do
      expect(puzzle1).not_to eq(puzzle2)
    end

    it "can solve a puzzle it generated" do
      10.times do
        puzzle = pseudoku.generate2
        solution = pseudoku.solve(puzzle)
        expect(solution).to be_a_solution_of(puzzle)
      end
    end
  end
end

