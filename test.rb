unless ARGV.index('run')
  require "mkmf"
  create_makefile("ruby_heap")
  exec 'make && ruby test.rb run'
end
require './ruby_heap.bundle'
require './ruby_heap'
rh=RHeap.new
ch=CExtHeap.new
t=Time.now;1000000.times{rh<<rand};p [:ruby, Time.now-t]
t=Time.now;1000000.times{ch<<rand};p [:c, Time.now-t]

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
puts :done
