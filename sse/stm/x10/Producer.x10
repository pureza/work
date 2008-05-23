public class Producer {
    private Buffer buffer;
    private Counter counter;
    private int id;

    public Producer(Buffer buffer, Counter counter, int id) {
        this.buffer = buffer;
        this.counter = counter;
	this.id = id;
    }

    public void produce() {
        int value = counter.increment();
        buffer.send(value);
	System.out.println("Producer " + id + " produced " + value);
    }

    public static void main(String[] args) {
	final Buffer buffer = new Buffer(10);
	final Counter counter = new Counter();

	for (int i = 0; i < 10; i++) {
	    final int id = i;
            async (here) {
	        Producer producer = new Producer(buffer, counter, id);
	        while (true) {
	            producer.produce();
 	        }
	    }
	}

	for (int i = 0; i < 10; i++) {
	    final int id = i;
            async (here) {
	        Consumer consumer = new Consumer(buffer, id);
	        while (true) {
	            consumer.consume();
 	        }
	    }
	}

    }
}
