#include <iostream>

class Shape {
    public:
        virtual void draw()
        {
            std::cout << "Drawing a shape" << "\n";
        }
};

class Circle : public Shape {
    private:
        int radius;
    public:
        Circle(int r)
        {
            this->radius = r;
        }

        void draw() override
        {
            std::cout << "Drawing a circle with radius " << this->radius << "\n";
        }

        void setRadius(int r)
        {
            this->radius = r;
        }
};

void drawCircles(Shape **shapes, int size)
{
    for (int i = 0; i < size; i++)
    {
        Circle *circlePtr = static_cast<Circle*>(shapes[i]);
        circlePtr->draw();
    }
}

int main() {
    Circle circle(5);

    Shape *shapePtr = &circle;
    shapePtr->draw();

    Circle *circlePtr = dynamic_cast<Circle*>(shapePtr);
    if (circlePtr == nullptr)
    {
        std::cout << "Conversion failed" << "\n";
        return 1; // 提前退出程序
    }

    circlePtr->setRadius(10);
    circlePtr->draw();

    Circle circle1(1);
    Circle circle2(2);
    Circle circle3(3);

    Shape *shapes[] = {&circle1, &circle2, &circle3};
    drawCircles(shapes, 3);

    return 0;
}