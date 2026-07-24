#include <iostream>

class Rectangle {
    private:
        int width;
        int height;

    public:
        Rectangle(int w, int h)
        {
            this->width = w;
            this->height = h;
        }

        // 声明友元函数
        friend int getPerimeter(const Rectangle& r);
        friend int getArea(const Rectangle& r);
};

int getPerimeter(const Rectangle& r)
{
    return 2 * (r.width + r.height);
}

int getArea(const Rectangle& r)
{
    return r.width * r.height;
}

int main() {
    Rectangle rect(5, 10);

    std::cout << "Perimeter: " << getPerimeter(rect) << std::endl;
    std::cout << "Area: " << getArea(rect) << std::endl;

    return 0;
}