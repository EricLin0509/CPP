#include <iostream>

class Shape {
    virtual double area() = 0;
    virtual double perimeter() = 0;
    protected:
        virtual void print() = 0;
};

void Shape::print()
{
    std::cout << "[Warning] Shape::print() is not implemented!\n";
}

class Circle : public Shape {
    private:
        double radius;
    public:
        Circle(double r)
        {
            this->radius = r;
        }
        double area() override
        {
            return 3.14 * this->radius * this->radius;
        }
        double perimeter() override
        {
            return 2 * 3.14 * this->radius;
        }
        void print() override
        {
            std::cout << "Circle radius: " << this->radius << "\n";
            std::cout << "Circle area: " << this->area() << "\n";
            std::cout << "Circle perimeter: " << this->perimeter() << "\n";
        }
};

int main() {
    Circle circle(5);
    circle.print();
    
    return 0;
}