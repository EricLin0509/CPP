#include <iostream>

int main()
{
    using std::string;
    using std::cout;
    using std::cin;
    using std::getline;

    string username = "";

    while (username.empty()) // 判断用户名是否为空
    {
        cout << "请输入用户名: ";
        getline(cin, username); // 读取用户名
    }

    cout << "欢迎 " << username << " 登录!" << "\n";

    return 0;
}