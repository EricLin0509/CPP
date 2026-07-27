#include <iostream>

class firstName {
    protected:
        std::string first_name;
    public:
        firstName(std::string name)
        {
            this->first_name = name;
        }

        virtual void printName() const
        {
            std::cout << "First Name: " << first_name << "\n";
        }
};

class lastName : public firstName {
    private:
        std::string last_name;
    public:
        lastName(std::string first_name, std::string last_name) : firstName(first_name)
        {
            this->last_name = last_name;
        }

        void printName() const override
        {
            std::cout << "First Name: " << this->first_name << "\n";
            std::cout << "Last Name: " << this->last_name << "\n";
        }
};

int main()
{
    lastName person("John", "Doe");
    person.printName();

    return 0;
}