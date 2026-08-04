#include <iostream>

void print_int(int *val, int index)
{
    std::cout << "index: " << index << ", value: " << *val << "\n";
}

int main() {
    int a[5] = {1, 2, 3, 4, 5};
    int *p = a;
    std::cout << "p: " << p << ", next: " << p + 1 << "\n";

    for (int i = 0; i < 5; i++)
    {
        print_int(p, i);
        p++;
    }

    std::cout << "\n";

    p = a + 4;
    for (int i = 0; i < 5; i++)
    {
        print_int(p, i);
        p--; // 指针移动到上一个元素
    }
    std::cout << "\n";

    int *start = a;
    int *end = a + 4;
    int i = 0;
    while ((start + i) <= end)
    {
        print_int(start + i, i);
        i++;
    }

    return 0;
}
