# while语句

while语句用于在条件为真时重复执行一段代码

## 语法

```cpp
while (条件表达式)
{
    // 代码块
}
```

- 条件表达式：一个布尔表达式，如果为真，则执行代码块，否则跳过代码块
- **注意**：如果条件表达式永远为真，那么while语句会**无限循环**，直到程序被 `SIGINT`（Ctrl+C）中断
    - 所以在使用while语句时，**一定要确保条件表达式会最终变为假**

## 示例

假设我们设计了一个获取用户名的程序

```cpp

using std::string;
using std::cout;
using std::cin;

string username = "";

if (username.empty()) // 判断用户名是否为空
{
    cout << "请输入用户名: ";
    getline(cin, username); // 读取用户名
}

cout << "欢迎 " << username << " 登录!" << "\n";

```

但这样会有一个问题，如果用户输入的用户名为空，程序会直接输出欢迎语句，而不是提示用户重新输入用户名

我们可以使用while语句来解决这个问题

### 使用 while 语句

```cpp
using std::string;
using std::cout;
using std::cin;

string username = "";

while (username.empty()) // 判断用户名是否为空
{
    cout << "请输入用户名: ";
    getline(cin, username); // 读取用户名
}

cout << "欢迎 " << username << " 登录!" << "\n";
```

这样，程序会一直提示用户输入用户名，直到用户输入的用户名不为空为止

```
请输入用户名: 
请输入用户名: 
请输入用户名: Enzo Ferrari
欢迎 Enzo Ferrari 登录!
```
