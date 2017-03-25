class PenguinQueue::MinMax
  attr_reader :priority_proc
  def initialize &block
    @priority_proc = block
    @min_queue = PenguinQueue.new :min
    @max_queue = PenguinQueue.new :max
  end

  class Node
    attr_reader :value
    attr_reader :priority
    attr_reader :parent
    def initialize queue, value, priority
      @queue, @value, @priority = queue, value, priority
      @min_node, @max_node = yield self
    end

    def value= value
      @value = value
      if @queue.priority_proc
        @priority = @min_node.priority = @max_node.priority = @queue.priority_proc.call value
      end
    end

    def priority= priority
      raise 'priority update not supported on queue initialized with block' if @queue.priority_proc
      @priority = @min_node.priority = @max_node.priority = priority
    end

    def remove
      @min_node.remove
      @max_node.remove
    end
    alias delete remove

    def inspect
      "#{self.class.name}{priority: #{@priority}, value: #{@value}}"
    end
    alias to_s inspect
  end

  def enq value, priority: value
    priority = priority_proc.call value if priority_proc
    Node.new self, value, priority do |node|
      [@min_queue.enq(node, priority: priority), @max_queue.enq(node, priority: priority)]
    end
  end
  alias push enq
  alias unshift enq

  def << e
    enq e
  end

  def min
    node = @min_queue.first
    node.value if node
  end

  def max
    node = @max_queue.first
    node.value if node
  end

  def min_with_priority
    node = @min_queue.first
    [node.value, node.priority] if node
  end

  def max_with_priority
    node = @max_queue.first
    [node.value, node.priority] if node
  end

  def deq_min_with_priority
    node = @min_queue.deq
    return unless node
    node.remove
    [node.value, node.priority]
  end

  def deq_max_with_priority
    node = @max_queue.deq
    return unless node
    node.remove
    [node.value, node.priority]
  end

  def min_node
    @min_queue.first
  end

  def max_node
    @max_queue.first
  end

  def deq_min n=nil
    return [size, n].min.times.map { deq_min } if n
    node = @min_queue.deq
    if node
      node.remove
      node.value
    end
  end

  def deq_max n=nil
    return [size, n].min.times.map { deq_max } if n
    node = @max_queue.deq
    if node
      node.remove
      node.value
    end
  end

  def remove node
    return if node.parent != queue
    node.remove
  end
  alias delete remove

  def clear
    @min_queue.clear
    @max_queue.clear
  end

  def empty?
    @min_queue.empty?
  end

  def size
    @min_queue.size
  end

  def inspect
    "#{self.class.name}{size: #{size}}"
  end
  alias to_s inspect
end
