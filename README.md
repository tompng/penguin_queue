# Priority Queue
```ruby
10000.times { array << rand; array.sort!; array.shift } #=> slow
10000.times { pq << rand; pq.deq } #=> fast
```

# Install
```ruby
# Gemfile
gem 'penguin_queue', git: 'https://github.com/tompng/penguin_queue'
```

# Usage
```ruby
require 'penguin_queue'
q = PenguinQueue.new
10.times.to_a.shuffle.each { |i| q << i }
10.times.map { q.deq }  #=> [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

# custom priority
q.enq 'hello', priority: 0
q.first #=> "hello"
q.first_with_priority #=> ["hello", 0]
q.deq_with_priority #=> ["hello", 0]

# update priority, remove node
nodes = 10.times.to_a.shuffle.map { |i| q << i }
nodes.each { |n| n.priority = -n.priority }
nodes.each_with_index{ |n| n.remove if n.value.odd? }
q.size.times.map { q.deq } #=> [8, 6, 4, 2, 0]
```

# API
```ruby
# PenguinQueue
PenguinQueue.new PenguinQueue.new(&calc_priority_from_element_proc)
# enqueue multiple
push(*e) unshift(*e)
# enqueue
<<(e) enq(e)
# enqueue with custom priority
enq(e, priority: p)
# dequeue
deq shift pop deque_with_priority
# dequeue multiple
deq(n) shift(n) pop(n)
# fetch
first first_with_priority first_node
# remove
remove(node) delete(node)
# other
to_s inspect size empty?

# PenguinQueue::Node
remove delete value value= priority priority=
```
