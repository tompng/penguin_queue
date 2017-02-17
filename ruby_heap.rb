class RHeap
  class Node
    attr_accessor :heap, :index, :value, :priority, :serial

    def initialize h, p, v, s
      @heap, @priority, @value, @serial = h, p, v, s
    end

    def to_s
      "#{self.class.name}{index: #{index}, priority: #{priority}, value: #{value}}"
    end

    def priority= priority
      raise unless @heap.include? self
      return if @priority == priority
      priority_was = @priority
      @priority = priority
      if priority_was > priority
        @heap.up self
      else
        @heap.down self
      end
    end

    alias inspect to_s
  end

  def include? node
    @heap[node.index].object_id == node.object_id
  end

  def initialize &compare_by
    @heap = []
    @serial = 0
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
    node = Node.new self, priority, value, (@serial+=1)
    node.index = @heap.size
    @heap.push node
    up node
    node
  end

  def first_node
    @heap[1]
  end

  def first
    first_node&.value
  end

  def first_with_priority
    node = first_node
    [node.value, node.priority] if node
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

  def deq n=nil
    if n
      [n, size].min.times.map{deq_with_priority&.first}
    else
      deq_with_priority&.first
    end
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

  def up node
    index = node.index
    while index > 1
      pindex = index/2
      pnode = @heap[pindex]
      break if pnode.priority < node.priority || (pnode.priority == node.priority && pnode.serial < node.serial)
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
        unless lnode.priority < rnode.priority || (lnode.priority == rnode.priority && lnode.serial < rnode.serial)
          lindex += 1
          lnode = rnode
        end
      end
      break unless node.priority > lnode.priority || (node.priority == lnode.priority && node.serial > lnode.serial)
      lnode.index = index
      @heap[index] = lnode
      index = lindex
    end
    node.index = index
    @heap[index] = node
  end
end
