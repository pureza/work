public class Counter {
    int value = 0;
    
    public Counter() { }

    public atomic int increment() {
        value++;
        return value++;
    }
}
