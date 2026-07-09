#include <iostream>

int main()
{
    using std::cout;
    using std::cin;

    int day = 0;
    cout << "请输入星期几(1-7): ";
    cin >> day;

    switch (day)
    {
        case 1:
            cout << "星期一" << "\n";
            break;
        case 2:
            cout << "星期二" << "\n";
            break;
        case 3:
            cout << "星期三" << "\n";
            break;
        case 4:
            cout << "星期四" << "\n";
            break;
        case 5:
            cout << "星期五" << "\n";
            break;
        case 6:
            cout << "星期六" << "\n";
            break;
        case 7:
            cout << "星期日" << "\n";
            break;
        default:
            cout << "输入错误" << "\n";
            break;
    }

    return 0;
}