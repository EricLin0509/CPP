#include <iostream>

int main()
{
    std::string students[] = {"Alice", "Bob", "Charlie", "David"};

    for (int i = 0; i < sizeof(students) / sizeof(students[0]); i++)
    {
        std::cout << students[i] << std::endl;
    }

    return 0;
}