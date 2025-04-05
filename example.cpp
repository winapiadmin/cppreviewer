void badFunction() {
    int* arr = new int[100], *b=new int[1048576];
    delete[] arr;
    delete[] b;
}
