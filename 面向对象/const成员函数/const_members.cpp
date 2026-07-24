#include <iostream>

class Base {
    private:
        int x;
        mutable bool canEdit;
    public:
        Base(int x)
        {
            this->x = x;
            this->canEdit = false;
        }

        int getX() const
        {
            return x;
        }

        void changeEdit() const
        {
            this->canEdit ^= true; // 这是被允许的
            // x = 10; // 这是不允许的，因为此时 x 是 const 成员变量
        }


        void setX(int x)
        {
            this->canEdit = true;
            this->x = x;
        }

        void printInfo() const
        {
            std::cout << "x = " << x << std::endl;
            std::cout << "canEdit = " << (canEdit ? "true" : "false") << std::endl;
        }
};

int main()
{
    const Base obj(10);
    int x = obj.getX();
    obj.changeEdit();
    // obj.setX(20); // 这是不允许的，因为 obj 是 const 对象

    obj.printInfo();

    return 0;
}