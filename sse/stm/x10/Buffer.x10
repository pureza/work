public class Buffer {
    int[] contents;
    int start = 0;
    int end = 0;
    final int capacity;

    public Buffer(int capacity) {
        this.capacity = capacity;
	contents = new int[capacity];
    }

    public int length() {
        return end - start;
    }

    public void send(int value) {
        when(length() < this.capacity) {
	    contents[end % capacity] = value;
	    end++;
	}
    }

    public int receive() {
       when (length() > 0) {
           int value = contents[start % capacity];
	   start++;
	   return value;
       }
    }
}
