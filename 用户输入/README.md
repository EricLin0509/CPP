# 用户输入

有两种方式可以获取用户输入

- `cin`：标准输入流
- `getline`：获取一行字符串

## cin

语法如下

```cpp
std::cin >> <变量名>;
```

- `>>`：输入运算符
- 当 `cin` 遇到空格、换行符等空白字符时，会自动停止输入

例如现在我们要获取用户年龄

```cpp
int age = 0;
std::cin >> age;

std::cout << "Your age is " << age << std::endl;
```

现在我们用 `cin` 获取用户姓名

```cpp
std::string name = "";
std::cin >> name;

std::cout << "Your name is " << name << std::endl;
```

但如果用户输入的是全名，会导致以下情况

```
John Smith
Your name is John
```

这是因为 `cin` 只会读取第一个单词，所以我们可以使用 `std::getline` 来获取整行字符串

## getline

语法如下

```cpp
std::getline(std::cin, <变量名>);
```

- `std::getline`：获取一行字符串
- `std::cin`：标准输入流
- `<变量名>`：要存储输入的变量

例如现在我们要获取用户姓名

```cpp
std::string name = "";
std::getline(std::cin, name);

std::cout << "Your name is " << name << std::endl;
```

这样就可以获取用户输入的完整姓名了

```
John Smith
Your name is John Smith
```

## 清理缓冲区

假设我们使用 `cin` 获取用户输入，然后立即使用 `std::getline` 获取下一行输入，可能会导致以下情况

```
20

Your age is 20
Your name is
```

这是因为 `cin` 会将输入的空白字符（如换行符）留在缓冲区中，所以我们可以在 `getline` 中的 `cin` 添加 `std:ws` (whitespace) 来清除缓冲区

```cpp
std::string name = "";
std::getline(std::cin >> std::ws, name);

std::cout << "Your name is " << name << std::endl;
```

这样就可以避免这种情况了

```
20

Your age is 20
Your name is John Smith
```
