#include <iostream>
#include <meta>

enum Color {
    Red,
    Green,
    Blue,
    Yellow,
};

int main() {
    Color c = Color::Red;
    switch (c)
    {
        template for (const std::meta::info e : std::meta::enumerators_of(^^Color))
        {
            case [:e:]:
                std::cout << std::meta::name_of(e) << "\n";
                break;
        }
        default:
            break;
    }

    return 0;
}