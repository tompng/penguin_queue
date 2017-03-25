require 'test_helper'

class MinMaxQueueTest < Minitest::Test
  def test_minmax
    q = PenguinQueue::MinMax.new
    10.times.to_a.shuffle.each { |i| q.enq i.to_s, priority: i}
    assert q.size == 10
    assert !q.empty?
    assert q.min == '0'
    assert q.max == '9'
    node = q.min_node
    node.priority=100
    node.value = '10'
    min = []
    max = []
    min << q.deq_min
    max += q.deq_max 4
    min += q.deq_min 4
    max << q.deq_max
    assert q.empty?
    assert min+max.reverse == (1..10).map(&:to_s)
  end
end
