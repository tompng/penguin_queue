require 'test_helper'

class PenguinQueueTest < Minitest::Test
  def test_that_it_has_a_version_number
    refute_nil ::PenguinQueue::VERSION
  end

  def test_it_does_something_useful
    assert (PenguinQueue.new<<3).value==3
  end
end
