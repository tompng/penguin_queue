unless ARGV.index('run')
  require "mkmf"
  create_makefile("ruby_heap")
  exec 'make && ruby test.rb run'
end
require './ruby_heap.bundle'
require './ruby_heap'
require 'benchmark'
rh=RHeap.new
ch=CExtHeap.new
bench = ->klass{
  h = klass.new
  tadd = Benchmark.measure{100000.times{h<<rand}}.real
  taddremove = Benchmark.measure{100000.times{h<<rand;h.deq}}.real
  tremove = Benchmark.measure{100000.times{h.deq}}.real
  nodes = 100000.times.map{h << rand}.shuffle
  tupdate = Benchmark.measure{nodes.each{|n|n.priority = rand}}.real
  p [klass.name, tadd, taddremove, tremove, tupdate]
}
[RHeap, CExtHeap].each &bench

arr=1000.times.map{rand}
rh=RHeap.new
ch=CExtHeap.new
rout, cout = [rh, ch].map do |heap|
  arr.take(80).each{|v|heap<<v}
  out = 50.times.map{heap.deq}
  arr.drop(80).each{|v|heap<<v}
  while val = heap.deq
    out << val
  end
  out
end
ans = arr.take(80).sort.take(50)+(arr.take(80).sort.drop(50)+arr.drop(80)).sort

def assert a, b, msg: nil
  puts "assert failed: #{msg}\n #{a}\n  #{b}" unless a==b
end

assert rout, ans
assert cout, ans

[RHeap, CExtHeap].each do |klass|
  h=klass.new
  p klass.name
  10.times.map{|i|2*i}.shuffle.map{|i|h.enq i.to_s, priority: i}
  assert 5.times.map{h.deq}, %w(0 2 4 6 8)
  10.times.map{|i|2*i+1}.shuffle.map{|i|h.enq i.to_s, priority: i}
  assert 15.times.map{h.deq}, %w(1 3 5 7 9 10 11 12 13 14 15 16 17 18 19)
  nodes = 13.times.map{|i|h.enq i, priority: 13*i%13}
  assert h.first_with_priority[1], 0
  nodes.each{|n|n.priority = n.value}
  assert h.first, 0
  assert 13.times.map{h.deq}, 13.times.to_a

  h = klass.new
  10.times{|i|h.enq i, priority: 0}
  arr = 5.times.map{h.deq}
  assert arr, arr.sort
  assert h.deq(2).size, 2
  assert h.deq(3), [7,8,9]
  nodes = 30.times.to_a.shuffle.map{|i|h<<i}.shuffle.each{|n|
    n.remove if n.priority%3==1
    h.remove n if n.priority%3==2
  }
  assert h.size.times.map{h.deq}, 30.times.select{|i|i%3==0}, msg: 'node remove'

  h = klass.new{|v|-v.to_i}
  assert h.empty?, true, msg: 'empty'
  h.push(*20.times.map(&:to_s).shuffle)
  assert h.empty?, false, msg: 'not empty'
  assert h.size, 20, msg: 'size'
  assert 10.times.map{h.deq}, (10...20).map(&:to_s).reverse, msg: 'compare by'
  h.first_node.value=:hello
  assert h.first, :hello
end
require 'pry'
binding.pry
