#include "math.hpp"
#include <iostream>

using namespace std;
int main(void) {
    cout << "Testing Standalone Inline Allocator\n";
    int* array = static_cast<int*>(custom_malloc(sizeof(int) * 20));
    if(array != nullptr) {
        for (int i = 0 ; i < 5 ; ++i) {
            array[i] = (i + 1) * 10;
            cout << "Element " << i << ": " << array[i] << " at " << &array[i] << "\n";
        }
        custom_free(array);
    }
}