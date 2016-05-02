class RHeap
  def initialize data=[], &compare_by
    @size = data.size
    if compare_by
      @table = {}
      @compare_by = compare_by
      @heap = data.map(&compare_by).uniq.sort
      data.each do |value|
        key = compare_by[value]
        (@table[key] ||= []) << value
      end
    else
      @heap = data.sort
    end
  end
  def enq data
    @size += 1
    if @compare_by
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
    while index > 0
      pindex = (index-1)/2
      pvalue = @heap[pindex]
      break if pvalue < value
      @heap[index] = pvalue
      index = pindex
    end
    @heap[index] = value
    self
  end
  def first
    value = @heap.first
    return value unless @compare_by
    @table[value].first if value
  end
  def deq
    return nil if @heap.empty?
    @size -= 1
    value = @heap.first
    if @compare_by
      list = @table[value]
      data = list.shift
      return data unless list.empty?
      @table.delete value
    else
      data = value
    end
    value = @heap.pop
    lastindex = @heap.size-1
    return data if @heap.empty?
    index = 0
    heap = @heap
    while true
      lindex = 2*index+1
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
    data
  end
  def to_s;"#<#{self.class.name}:0x#{(object_id*2).to_s(16)}[#{@size}]>";end
  def empty?;@heap.empty?;end
  def size;@size;end
  alias pop deq
  alias shift deq
  alias push enq
  alias unshift enq
  alias << enq
  alias inspect to_s
end
