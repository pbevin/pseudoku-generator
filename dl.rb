class ExactCover
  def initialize
    @root = Column.new
    @root.prev = @root
    @root.next = @root

    @cols = {}

    @solution = []
  end

  def find_col(name)
    @cols[name]
  end

  def columns(names)
    names.each do |name|
      col = Column.new
      col.name = name
      col.len = 0
      col.next = @root
      col.prev = @root.prev
      col.u = col.d = col
      @root.prev.next = col
      @root.prev = col

      @cols[name] = col
    end
  end

  def row(rowname, cols)
    prevnode = nil

    cols.each do |name|
      col = @cols[name]
      fail "No such column #{name.inspect}" if !col

      node = Node.new
      if prevnode
        node.l = prevnode
        node.r = prevnode.r
        prevnode.r.l = node
        prevnode.r = node
      else
        prevnode = node
        node.r = node.l = node
      end
      node.c = col
      node.rowname = rowname

      node.u = col.u
      node.d = col
      col.u.d = node
      col.u = node

      col.len += 1
    end
  end

  def cover(c)
    c.next.prev = c.prev
    c.prev.next = c.next
    i = c.d
    while i != c
      j = i.r
      while j != i
        j.d.u = j.u
        j.u.d = j.d
        j.c.len -= 1
        j = j.r
      end
      i = i.d
    end
  end

  def uncover(c)
    i = c.u
    while i != c
      j = i.l
      while j != i
        j.c.len += 1
        j.d.u = j
        j.u.d = j

        j = j.l
      end

      i = i.u
    end

    c.prev.next = c
    c.next.prev = c
  end

  def print_solution(k)
    p @solution.map { |r| r.rowname }
  end

  def search(k: 0)
    if @root.next == @root
      print_solution(k)
      return
    end

    c = @root.next
    cover(c)

    r = c.d
    while r != c
      @solution[k] = r

      j = r.r
      while j != r
        cover(j.c)
        j = j.r
      end
      search(k: k+1)
      r = @solution[k]
      c = r.c
      j = r.l
      while j != r
        uncover(j.c)
        j = j.l
      end
      r = r.d
    end
    uncover(c)
  end

  def dump
    col = @root.next
    while col != @root
      print col.name
      node = col.d
      while node != col
        print "->#{node.rowname}"
        node = node.d
      end
      puts
      col = col.next
    end
  end

  def build1
    columns %w[ A B C D E F G ]

    row 1, %w[ C E F ]
    row 2, %w[ A D G ]
    row 3, %w[ B C F ]
    row 4, %w[ A D ]
    row 5, %w[ B G ]
    row 6, %w[ D E G ]
  end

  def sudoku(clues)
    columns all("P") + all("C") + all("R") + all("B")
    (1..9).each do |r|
      (1..9).each do |c|
        (1..9).each do |n|
          row "#{n} at (#{r},#{c})", sdk_row(r, c, n)
        end
      end
    end

    clues.split("").each.with_index do |n, i|
      next if n == "."
      r = 1 + (i/9).to_i
      c = 1 + (i%9)
      sdk_row(r, c, n).each do |col_name|
        cover(find_col(col_name))
      end
    end
  end

  def sdk_row(r, c, n)
    b = 1 + 3 * ((r-1) / 3).to_i + ((c-1) / 3).to_i

    [ "P#{r}#{c}", "R#{r}#{n}", "C#{c}#{n}", "B#{b}#{n}" ]
  end

  def all(name)
    (1..9).map do |i|
      (1..9).map do |j|
        "#{name}#{i}#{j}"
      end
    end.flatten
  end

  class Node
    attr_accessor :l, :r, :u, :d, :c
    attr_accessor :rowname
  end

  class Column
    attr_accessor :len, :name
    attr_accessor :prev, :next
    attr_accessor :u, :d
  end
end

easy = "...5...8......1..2..5.9...4.6..34..9.38...26.2..61..5.9...2.3..6..8......4...5..."
hard = "..1.....8.4...79..2...9.1....4.....9.6.874.2.5.....4....5.3...4..35...1.9.....6.."

ec = ExactCover.new
ec.sudoku(easy)
ec.search
