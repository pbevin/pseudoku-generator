require 'pseudoku'

describe Pseudoku do
  let(:pseudoku) { subject }

  describe '#generate' do
    let(:puzzle1) { pseudoku.generate }
    let(:puzzle2) { pseudoku.generate }

    it "generates a puzzle" do
      expect(puzzle1.length).to eq(81)
    end

    it "generates different puzzles" do
      expect(puzzle1).not_to eq(puzzle2)

    end
  end
end

