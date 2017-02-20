require 'test_helper'

class PenguinQueueTest < Minitest::Test
  def test_that_it_has_a_version_number
    refute_nil ::PenguinQueue::VERSION
  end

  def test_enq_deq
    h = PenguinQueue.new
    10.times{|i|h.enq i.to_s}
    assert h.empty? == false
    assert h.size == 10
    assert 10.times.map{h.deq} == 10.times.map(&:to_s)
    assert h.empty? == true
    assert h.size == 0
  end

  def test_priority
    h = PenguinQueue.new
    nodes = 13.times.map{|i|h.enq i, priority: 13*(i+1)%13}
    assert h.first_with_priority == [h.first_node.value, h.first_node.priority]
    assert h.first_node.priority == 0
    nodes.each{|n|n.priority = n.value}
    assert 13.times.map{h.deq} == 13.times.to_a
  end

  def test_same_priority
    h = PenguinQueue.new
    10.times{|i|h.enq i, priority: 0}
    arr1 = 5.times.map{h.deq}
    arr2 = h.deq 2
    arr3 = h.deq 3
    arr = arr1+arr2+arr3
    assert arr == arr.sort
  end

  def test_node_remove
    h = PenguinQueue.new
    nodes = 30.times.to_a.shuffle.map{|i|h<<i}.shuffle.each{|n|
      n.remove if n.priority%3==1
      h.remove n if n.priority%3==2
    }
    assert h.size.times.map{h.deq} == 30.times.select{|i|i%3==0}
  end

  def test_priority_proc
    h = PenguinQueue.new{|v|-v.to_i}
    h.push(*20.times.map(&:to_s).shuffle)
    assert 10.times.map{h.deq} == (10...20).map(&:to_s).reverse
  end

  def test_invalid_compare
    h = PenguinQueue.new
    h << 'a'
    err = :compare_error
    b = h << 'b' rescue err
    c = h << :c rescue err
    assert b != err
    assert c == err
  end

  def test_value_update
    h = PenguinQueue.new
    h.push(*10.times)
    assert h.first == 0
    h.first_node.value = 1
    assert h.first == 1
  end
end
