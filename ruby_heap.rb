class RHeap
  class Node
    attr_accessor :index, :value, :priority

    def initialize p, v
      @priority, @value = p, v
    end

    def to_s
      "#{self.class.name}(index: #{index}, priority: #{priority}, value: #{value})"
    end

    alias inspect to_s
  end

  def initialize &compare_by
    @heap = []
    @compare_by = compare_by
    @heap.unshift nil
  end

  def push *values
    values.map { |v| enq v }
  end

  def << v
    enq v
  end

  def enq value, priority: value
    priority = @compare_by.call value if @compare_by
    node = Node.new priority, value
    node.index = @heap.size
    @heap.push node
    up node
    node
  end

  def first_node
    @heap[1]
  end

  def first
    node = first_node
    [node.priority, node.value] if node
  end

  def update node, priority
    raise unless @heap[node.index].object_id == node.object_id
    return if node.priority == priority
    priority_was = node.priority
    node.priority = priority
    if priority_was > priority
      up node
    else
      down node
    end
  end

  def deq_with_priority
    return nil if empty?
    first = @heap[1]
    node = @heap.pop
    if @heap.size > 1
      node.index = 1
      down node
    end
    [first.value, first.priority]
  end

  def deq
    deq_with_priority&.first
  end

  def to_s
    "#<#{self.class.name}[size=#{size}]>"
  end

  alias inspect to_s

  def empty?
    @heap.size == 1
  end

  def size
    @heap.size-1
  end

  private

  def up node
    index = node.index
    while index > 1
      pindex = index/2
      pnode = @heap[pindex]
      break if pnode.priority < node.priority
      pnode.index = index
      @heap[index] = pnode
      index = pindex
    end
    node.index = index
    @heap[index] = node
  end

  def down node
    index = node.index
    while 2*index < @heap.size
      lindex = 2*index
      lnode = @heap[lindex]
      if lindex+1 < @heap.size
        rnode = @heap[lindex+1]
        unless lnode.priority < rnode.priority
          lindex += 1
          lnode = rnode
        end
      end
      break unless node.priority > lnode.priority
      lnode.index = index
      @heap[index] = lnode
      index = lindex
    end
    node.index = index
    @heap[index] = node
  end
end

def assert a, b
  raise "assert failed\n  #{a}\n  #{b}" unless a==b
end
h=RHeap.new
10.times.map{|i|2*i}.shuffle.map{|i|h.enq i.to_s, priority: i}
assert 5.times.map{h.deq}, %w(0 2 4 6 8)
10.times.map{|i|2*i+1}.shuffle.map{|i|h.enq i.to_s, priority: i}
assert 15.times.map{h.deq}, %w(1 3 5 7 9 10 11 12 13 14 15 16 17 18 19)
nodes = 13.times.map{|i|h.enq i, priority: 13*i%13}
nodes.each{|n|h.update n, n.value}
assert 13.times.map{h.deq}, 13.times.to_a

h2 = RHeap.new{|v|-v.to_i}
h2.push(*20.times.map(&:to_s).shuffle)
assert 10.times.map{h2.deq}, (10...20).map(&:to_s).reverse
