#include <iostream>

int main()
{
    using std::cout;
    using std::cin;

    int num = 0;

    do
    {
        cout << "请输入一个正整数：";
        cin >> num;
    } while (num < 0);
    cout << "你输入的正整数是：" << num << "\n";
}