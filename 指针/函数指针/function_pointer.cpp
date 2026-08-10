#include <iostream>

int c_to_f(int c)
{
    return c * 9 / 5 + 32;
}

int f_to_c(int f)
{
    return (f - 32) * 5 / 9;
}

int main() {
    int value = 0;
    char choice = '\0';

    std::cout << "Enter a temperature: ";
    std::cin >> value;

    std::cout << "Enter 'C' for Celsius or 'F' for Fahrenheit: ";
    std::cin >> choice;

    int (*converter)(int) = nullptr;

    converter = (choice == 'C') ? c_to_f : f_to_c;

    int result = converter(value);

    std::cout << "The temperature is " << result << " degrees " << (choice == 'C' ? "Fahrenheit" : "Celsius") << ".\n";

    return 0;
}