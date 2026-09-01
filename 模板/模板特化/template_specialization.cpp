#include <iostream>
#include <unordered_set>

class Student
{
    private:
        std::string name;
        int age;

    public:
        Student(const char *name, int age)
        {
            this->name = name;
            this->age = age;
        }

        bool operator==(const Student &other) const
        {
            return this->name == other.name && this->age == other.age;
        }

        friend struct std::hash<Student>; // 使用友元函数使其能访问私有成员
};

/* 全特化版本 */
template <>
struct std::hash<Student> {
    size_t operator()(const Student &student) const
    {
        return std::hash<std::string>{}(student.name) ^ std::hash<int>{}(student.age);
    }
};

/* 主模板 */
template <typename T>
class MyClass {
    private:
        T data;
};

/* 全特化版本 */
template <>
class MyClass<int[10]> {
    private:
        int data[10];
};


/* 偏特化版本 */
template <typename T>
class MyClass<T*> {
    private:
        T *data;
};

int main() {
    std::unordered_set<Student> students;

    MyClass<int[10]> obj1; // 实例化全特化版本
    MyClass<int*> obj2; // 实例化偏特化版本
    MyClass<int> obj3; // 实例化主模板

    std::cout << sizeof(obj1) << " bytes\n"; // 全特化版本的大小
    std::cout << sizeof(obj2) << " bytes\n"; // 偏特化版本的大小
    std::cout << sizeof(obj3) << " bytes\n"; // 主模板的大小

    return 0;
}