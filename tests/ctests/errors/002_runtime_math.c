int main() {
    int a = 1;
    int b = 0;
    if (a + b) {
        /*
            TODO: if uncommented, the error below throws a non deterministic error 
            message since it includes the PID. Find a way to write a CHECK for a
            message that changes....  
        */

        // b = a / b;
    }
    return 0;
}