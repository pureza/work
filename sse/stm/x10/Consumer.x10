public class Consumer {
    private Buffer buffer;
    private int id;
    
    public Consumer(Buffer buffer, int id) {
        this.buffer = buffer;
        this.id = id; 
    }

    public void consume() {
        int value = buffer.receive();
        System.out.println("Consumer " + id + " consumed " + value);
    }
}

