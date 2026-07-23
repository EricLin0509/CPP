#include <iostream>

class Person {
    private:
        int *age = new int;
    public:
        Person(int age)
        {
            *(this->age) = age;
        }

        ~Person()
        {
            std::cout << "Destroying this object" << "\n";
            delete age;
        }

        int getAge()
        {
            return *(this->age);
        }
};

int main()
{
    Person p(20);
    std::cout << "Person's age is: " << p.getAge() << std::endl;

    return 0;
}