#include <iostream>

class Stove {
    private:
        int temperature = 0;
    public:
        int getTemperature()
        {
            return this->temperature;
        }

        void setTemperature(int temp)
        {
            if (temp < 0)
                this->temperature = 0;
            else if (temp > 1000)
                this->temperature = 1000;
            else
                this->temperature = temp;
        }
};

int main() {
    Stove stove;
    stove.setTemperature(500);
    std::cout << "Current temperature: " << stove.getTemperature() << "\n";
    return 0;
}
