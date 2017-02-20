# Priority Queue
```ruby
10000.times { array << rand; array.delete(array.min) } #=> slow when array.size is large
10000.times { pq << rand; pq.deq } #=> fast even if pq.size is large
```

# Install
```
gem install penguin_queue
```

```ruby
# Gemfile
gem 'penguin_queue'
```

# Usage
```ruby
require 'penguin_queue'
pq = PenguinQueue.new
10.times.to_a.shuffle.each { |i| pq << i }
10.times.map { pq.deq }  #=> [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

# custom priority
pq.enq 'hello', priority: 0
pq.first #=> "hello"
pq.first_with_priority #=> ["hello", 0]
pq.deq_with_priority #=> ["hello", 0]

# update priority, remove node
nodes = 10.times.to_a.shuffle.map { |i| pq << i }
nodes.each { |n| n.priority = -n.priority }
nodes.each_with_index{ |n| n.remove if n.value.odd? }
pq.size.times.map { pq.deq } #=> [8, 6, 4, 2, 0]
```

# Methods
```ruby
# initialize
PenguinQueue.new
PenguinQueue.new(order: :max) # min(default) or max
PenguinQueue.new(&calc_priority_from_element_proc)
# enqueue
<<(e) enq(e) push(e) unshift(e)
# enqueue with custom priority
enq(e, priority: p) push(e, priority: p) unshift(e, priority: e)
# dequeue
deq shift pop poll deq_with_priority
# dequeue multiple
deq(n) shift(n) pop(n) poll(n)
# fetch
first peek top first_with_priority first_node
# remove
remove(node) delete(node)
# other
to_s inspect clear size empty? min? max?

# PenguinQueue::Node
remove delete value value= priority priority= to_s inspect
```
