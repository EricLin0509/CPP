# 友元函数

友元函数 (Friend Function) 定义在类外部，但有权访问类的所有私有成员和保护成员

尽管友元函数的原型有在类的定义中出现过，但是友元函数并不是成员函数

## 语法

### 声明

```cpp
class 类名 {
    public:
        // 声明友元函数
        friend 返回类型 函数名(参数列表);
};
```

### 实现

```cpp
class 类名 {
    public:
        // 声明友元函数
        friend 返回类型 函数名(参数列表);
};

// 实现友元函数
返回类型 函数名(参数列表) {    }
```

## 示例

现在有个 `Rectangle` 类

```cpp
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
};
```

我们希望在类外面实现计算周长和面积的函数，就可以使用友元函数

### 声明友元函数

```cpp
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
```

### 实现友元函数

```cpp
int getPerimeter(const Rectangle& r)
{
    return 2 * (r.width + r.height);
}

int getArea(const Rectangle& r)
{
    return r.width * r.height;
}
```

### 调用友元函数

由于友元函数是类外部的，所以不需要通过 `对象名.方法名` 的方式调用，而是直接调用函数名

```cpp
Rectangle rect(5, 10);

std::cout << "Perimeter: " << getPerimeter(rect) << std::endl;
std::cout << "Area: " << getArea(rect) << std::endl;
```

