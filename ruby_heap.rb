class RHeap
  class Node
    attr_accessor :heap, :index, :value, :priority

    def initialize h, p, v
      @heap, @priority, @value = h, p, v
    end

    def to_s
      "#{self.class.name}(index: #{index}, priority: #{priority}, value: #{value})"
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
    node = Node.new self, priority, value
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

  # def update node, priority
  #   raise unless @heap[node.index].object_id == node.object_id
  #   return if node.priority == priority
  #   priority_was = node.priority
  #   node.priority = priority
  #   if priority_was > priority
  #     up node
  #   else
  #     down node
  #   end
  # end

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
