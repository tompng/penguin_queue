class RHeap
  def initialize data=[], &compare_by
    if compare_by
      @table = {}
      @compare_by = compare_by
      @heap = data.map(&compare_by).uniq.sort
      data.each do |value|
        key = compare_by[value]
        (@table[key] ||= []) << value
      end
      @size = data.size
    else
      @heap = data.sort
    end
    @heap.unshift nil
  end
  def enq data
    if @compare_by
      @size += 1
      key = @compare_by[data]
      if @table[key]
        @table[key] << data
        return self
      end
      @table[key] = [data]
      value = key
    else
      value = data
    end
    index = @heap.size
    while index > 1
      pindex = index/2
      pvalue = @heap[pindex]
      break if pvalue < value
      @heap[index] = pvalue
      index = pindex
    end
    @heap[index] = value
    self
  end
  def first
    value = @heap[1]
    return value unless @compare_by
    @table[value].first if value
  end
  def deq
    return nil if @heap.size == 1
    retval = @heap[1]
    if @compare_by
      @size -= 1
      key = retval
      list = @table[key]
      retval = list.shift
      return retval unless list.empty?
      @table.delete key
    end
    value = @heap.pop
    return retval if @heap.size == 1
    lastindex = @heap.size - 1
    index = 1
    heap = @heap
    while true
      lindex = 2*index
      break if lindex > lastindex
      lvalue = heap[lindex]
      if lindex < lastindex
        rvalue = heap[lindex+1]
        unless lvalue < rvalue
          lindex += 1
          lvalue = rvalue
        end
      end
      break unless value > lvalue
      heap[index] = lvalue
      index = lindex
    end
    heap[index] = value
    retval
  end
  def to_s;"#<#{self.class.name}:0x#{(object_id*2).to_s(16)}[#{@size}]>";end
  def empty?;@heap.empty?;end
  def size;@size||@heap.size-1;end
  alias pop deq
  alias shift deq
  alias push enq
  alias unshift enq
  alias << enq
  alias inspect to_s
end
