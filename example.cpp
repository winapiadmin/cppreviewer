void badFunction() {
    int* arr = new int[100], *b=new int[1048576];
}
template <typename T>
T* allocateMemory(unsigned long size) {
    T* ptr = new T[size];
    return ptr;
}