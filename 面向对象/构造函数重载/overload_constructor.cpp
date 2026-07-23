#include <iostream>

class Pizza {
    public:
        std::string topping1;
        std::string topping2;

        Pizza()
        {
            this->topping1 = "Pepperoni";
            this->topping2 = "";
        }

        Pizza(std::string topping1)
        {
            this->topping1 = topping1;
            this->topping2 = "";
        }

        Pizza(std::string topping1, std::string topping2)
        {
            this->topping1 = topping1;
            this->topping2 = topping2;
        }

        void print_toppings()
        {
            std::cout << "Pizza with topping(s) " << topping1;

            if (topping2 != "")
                std::cout << " and " << topping2;

            std::cout << "\n";
        }
};

int main() {
    Pizza pizza1("Pepperoni");
    pizza1.print_toppings();

    Pizza pizza2("Pepperoni", "Mushrooms");
    pizza2.print_toppings();

    Pizza pizza3;
    pizza3.print_toppings();

    return 0;
}