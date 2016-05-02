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
    index = 0
    value = @heap[index]
    if @compare_by
      list = @table[value]
      data = list.shift
      return data unless list.empty?
      @table.delete value
    else
      data = value
    end
    value = @heap.pop
    return data if @heap.empty?
    while true
      lindex, rindex = 2*index+1, 2*index+2
      left, right = @heap[lindex], @heap[rindex]
      break unless left
      cindex, cvalue = !right || left < right ? [lindex, left] : [rindex, right]
      break unless value > cvalue
      @heap[index] = cvalue
      index = cindex
    end
    @heap[index] = value
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
