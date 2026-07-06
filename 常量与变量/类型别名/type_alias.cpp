#include <iostream>
#include <vector>

typedef std::vector<std::pair<std::string, int>> pairlist_t;

using string_t = std::string;

int main() {
    string_t sentence = "Hello World";

    std::cout << sentence << std::endl;

    return 0;
}