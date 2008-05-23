require 'thread'

class Barrier
  def initialize(count)
    @count = count
    @mutex = Mutex.new
    @cond = ConditionVariable.new
    @waiting = 0
  end

  def wait
    @mutex.synchronize {
      @waiting += 1
      if (@waiting >= @count)
	@waiting = 0
        @cond.broadcast
      else
        @cond.wait(@mutex)
      end
    }
  end
end
