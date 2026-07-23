#include <iostream>

class Engine {
    private:
        std::string model;
        std::string type;

    public:
        Engine(const std::string& model, const std::string& type) // 使用引用传递避免不必要的内存拷贝
        {
            this->model = model;
            this->type = type;
        }

        void displayEngineInfo()
        {
            std::cout << "Engine Model: " << model << "\n";
            std::cout << "Engine Type: " << type << "\n";
        }
};

class Car {
    private:
        std::string make;
        std::string model;
        Engine engine;

    public:
        Car(const std::string& make, const std::string& model, const std::string& engineModel, const std::string& engineType) : engine(engineModel, engineType)
        {
            this->make = make;
            this->model = model;

        }

        void displayCarInfo()
        {
            std::cout << "Car Manufacturer: " << make << "\n";
            std::cout << "Car Model: " << model << "\n";
            engine.displayEngineInfo();
        }
};

int main()
{
    Car myCar("Subaru", "WRX STI S209", "EJ257", "F4");
    myCar.displayCarInfo();

    return 0;
}