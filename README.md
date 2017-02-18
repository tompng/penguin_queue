`ruby test.rb`

```ruby
# priority queue
10000.times { array<<rand; array.sort!; array.shift } #=> slow
10000.times { heap<<rand; heap.deq } #=> fast

h = CExtHeap.new
10.times.to_a.shuffle.each { |i| h << i }
10.times.map { h.deq }  #=> [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

# update priority
nodes = 10.times.to_a.shuffle.map { |i| h << i }
nodes.each { |n| n.priority = -n.priority }
10.times.map { |h| h.deq } #=> [9, 8, 7, 6, 5, 4, 3, 2, 1, 0]

# priority & value
h.enq 'hello', priority: 0.5
value, priority = h.deq_with_priority

h = CExtHeap.new { |v| v.score }
score_objects.each { |s| h << s }

# Heap methods
# class methods
new new(&calc_priority_from_element_block)
# enqueue multiple
push(*e) unshift(*e)
# enqueue
<<(e) enq(e)
# enqueue with custom priority
enq(e, priority: p)
# dequeue
deq shift pop
# dequeue multiple
deq(n) shift(n) pop(n)
# fetch
first first_with_priority first_node
# remove
remove(node)
# other
to_s inspect size empty?

# Node methods
remove value value= priority priority=
```
