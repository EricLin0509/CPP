#include <iostream>
#include <exception>

class DevideByZeroException : public std::runtime_error {
    public:
        DevideByZeroException() : std::runtime_error("Can not devide by zero") {} 
};

double divide(int a, int b)
{
    if (b == 0)
    {
        throw DevideByZeroException{};
    }
    return a / b;
}

int main() {
    try
    {
        double result = divide(10, 0);
    }
    catch (DevideByZeroException& e)
    {
        std::cout << e.what() << "\n";
    }

    return 0;
}