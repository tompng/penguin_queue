require 'test_helper'

class PenguinQueueTest < Minitest::Test
  def test_that_it_has_a_version_number
    refute_nil ::PenguinQueue::VERSION
  end

  def test_enq_deq
    q = PenguinQueue.new
    10.times{|i|q.enq i.to_s}
    assert q.empty? == false
    assert q.size == 10
    assert 10.times.map{q.deq} == 10.times.map(&:to_s)
    assert q.empty? == true
    assert q.size == 0
  end

  def test_priority
    q = PenguinQueue.new
    nodes = 13.times.map{|i|q.enq i, priority: 13*(i+1)%13}
    assert q.first_with_priority == [q.first_node.value, q.first_node.priority]
    assert q.first_node.priority == 0
    nodes.each{|n|n.priority = n.value}
    assert 13.times.map{q.deq} == 13.times.to_a
  end

  def test_same_priority
    q = PenguinQueue.new
    10.times{|i|q.enq i, priority: 0}
    arr1 = 5.times.map{q.deq}
    arr2 = q.deq 2
    arr3 = q.deq 3
    arr = arr1+arr2+arr3
    assert arr == arr.sort
  end

  def test_node_remove
    q = PenguinQueue.new
    nodes = 30.times.to_a.shuffle.map{|i|q<<i}.shuffle.each{|n|
      n.remove if n.priority%3==1
      q.remove n if n.priority%3==2
    }
    assert q.size.times.map{q.deq} == 30.times.select{|i|i%3==0}
  end

  def test_priority_proc
    q = PenguinQueue.new{|v|-v.to_i}
    q.push(*20.times.map(&:to_s).shuffle)
    assert 10.times.map{q.deq} == (10...20).map(&:to_s).reverse
  end

  def test_invalid_compare
    q = PenguinQueue.new
    q << 'a'
    err = :compare_error
    b = q << 'b' rescue err
    c = q << :c rescue err
    assert b != err
    assert c == err
  end

  def test_value_update
    q = PenguinQueue.new
    q.push(*10.times)
    assert q.first == 0
    q.first_node.value = 1
    assert q.first == 1
  end
end
