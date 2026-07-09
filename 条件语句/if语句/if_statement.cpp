#include <iostream>

int main()
{
    /* 不推荐直接使用 using namespace std */ 
    using std::cin;
    using std::cout;

    int age = -1;
    cout << "请输入你的年龄: ";
    cin >> age;

    if (age >= 18)
    {
        cout << "你已经成年了！\n";
    }
    else if (age < 0)
    {
        cout << "你输入的年龄不合法！\n";
    }
    else
    {
        cout << "你还没有成年！\n";
    }
}