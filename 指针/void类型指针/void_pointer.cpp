#include <iostream>

int memcmp(void *ptr1, void *ptr2, size_t size)
{
    unsigned char *data1 = (unsigned char *)ptr1;
    unsigned char *data2 = (unsigned char *)ptr2;
    
    for (size_t i = 0; i < size; i++)
    {
        if (data1[i] != data2[i])
            return data1[i] - data2[i];
    }
    
    return 0;
}

int main() {
    int arr1[] = {1, 2, 3};
    int arr2[] = {4, 5, 6};
    int result_int = memcmp(arr1, arr2, sizeof(arr1));

    std::cout << "Result for int array: " << result_int << "\n";

    double arr3[] = {1.1, 2.2, 3.3};
    double arr4[] = {4.4, 5.5, 6.6};
    int result_double = memcmp(arr3, arr4, sizeof(arr3));

    std::cout << "Result for double array: " << result_double << "\n";

    return 0;
}