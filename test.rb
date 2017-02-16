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
  nodes = 100000.times.map{h << rand}
  tupdate = Benchmark.measure{nodes.shuffle.each{|n|n.priority = rand}}.real
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
require 'pry'
binding.pry unless rout == ans
binding.pry unless cout == ans
r=RHeap.new{|a|a/10}
100.times.reverse_each{|i|r<<i}
puts :done




def assert a, b
  raise "assert failed\n #{a}\n  #{b}" unless a==b
end
[RHeap, CExtHeap].each do |klass|
  h=klass.new
  p klass.name
  10.times.map{|i|2*i}.shuffle.map{|i|h.enq i.to_s, priority: i}
  assert 5.times.map{h.deq}, %w(0 2 4 6 8)
  10.times.map{|i|2*i+1}.shuffle.map{|i|h.enq i.to_s, priority: i}
  assert 15.times.map{h.deq}, %w(1 3 5 7 9 10 11 12 13 14 15 16 17 18 19)
  nodes = 13.times.map{|i|h.enq i, priority: 13*i%13}
  nodes.each{|n|n.priority = n.value}
  assert 13.times.map{h.deq}, 13.times.to_a

  h2 = klass.new{|v|-v.to_i}
  h2.push(*20.times.map(&:to_s).shuffle)
  assert 10.times.map{h2.deq}, (10...20).map(&:to_s).reverse
end

binding.pry
