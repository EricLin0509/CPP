#include <iostream>

void print_all_data_recursive() {}

template <typename First, typename... Args>
void print_all_data_recursive(First first, Args... args)
{
    std::cout << first << "\n";
    print_all_data_recursive(args...);
}

template <typename... Args>
void print_all_data_fold(Args... args)
{
    ((std::cout << args << "\n"), ...);
}

template <typename... Args>
void print_all_data_template_for(Args... args)
{
    template for (auto a : {args...})
    {
        std::cout << a << "\n";
    }
}

int main() {
    print_all_data_recursive(1, 2, 3, 4, 5);
    print_all_data_fold(1, 2, 3, 4, 5);

    print_all_data_template_for(1, 2, 3, 4, 5);
    return 0;
}