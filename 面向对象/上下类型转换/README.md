# 上下类型转换

在之前的[多态](../多态/README.md)中,我们已经了解了如何通过基类指针调用派生类的方法(向上转换)

但有时我们需要反过来——将基类指针转换为派生类指针(向下转换)

这就是**Upcasting**(向上转换)和**Downcasting**(向下转换)的概念

## 语法

### Upcasting(向上转换)

向上转换是指将派生类对象或指针转换为基类类型。这是隐式且安全的操作,因为派生类包含了基类的所有成员。在多态中,我们经常使用向上转换来实现统一接口处理不同类型的对象

```cpp
Derived derivedObj(1); // 创建派生类对象
Base *basePtr = nullptr;
basePtr = &derivedObj; // 向上转换
```

### Downcasting(向下转换)

向下转换是指将基类指针或引用转换为派生类类型。这种转换需要显式进行,并且存在风险——如果基类指针实际指向的不是目标派生类对象,转换会导致未定义行为

因此,C++提供了两种主要的向下转换方式:`static_cast`用于已知安全的情况,而`dynamic_cast`则会在运行时进行类型检查,确保转换的安全性

```cpp
Base baseObj;
Derived *derivedPtr = nullptr;
derivedPtr = static_cast<Derived*>(&baseObj); // 静态向下转换
derivedPtr = dynamic_cast<Derived*>(&baseObj); // 动态向下转换
```

- 必须使用**指针**或**引用**进行向下转换

静态与动态向下转换的区别在于:

- **静态向下转换**在编译时不会进行运行时类型检查,它允许任何相关的指针/引用转换
    - 即使实际类型不匹配也不会报错，会发生未定义行为
- **动态向下转换**在运行时进行类型检查,如果转换失败,`dynamic_cast`会返回`nullptr`
    - 由于会进行类型检查，所以性能上会比 `static_cast` 慢

## 示例

现在有个基类 `Shape`

```cpp
class Shape {
    public:
        virtual void draw()
        {
            std::cout << "Drawing a shape" << "\n";
        }
};
```

利用这个基类,我们创建一个派生类 `Circle`

```cpp
class Circle : public Shape {
    private:
        int radius;
    public:
        Circle(int r)
        {
            this->radius = r;
        }

        void draw() override
        {
            std::cout << "Drawing a circle with radius " << this->radius << "\n";
        }

        void setRadius(int r)
        {
            this->radius = r;
        }
};
```

创建一个 `Circle` 对象

```cpp
Circle circle(5);
```

### Upcasting(向上转换)

我们可以使用一个基类指针并指向一个派生类对象，这就是向上转换

```cpp
Shape *shapePtr = &circle;
shapePtr->draw(); // 输出: Drawing a circle with radius 5
```

此时只能访问基类的成员和方法，不能访问派生类**独有**的成员和方法

```cpp
shapePtr->setRadius(10);
```

```
downcasting.cpp:35:15: error: ‘class Shape’ has no member named ‘setRadius’
```

### Downcasting(向下转换)

此时需要使用 `dynamic_cast` 或者 `static_cast` 进行向下转换

### dynamic_cast(动态向下转换)

```cpp
Circle *circlePtr = dynamic_cast<Circle*>(shapePtr);
circlePtr->setRadius(10);
circlePtr->draw(); // 输出: Drawing a circle with radius 10
```

此时会有个小问题，如果这里 `dynamic_cast` 的转换失败

```cpp
shapePtr = nullptr; // 将 shapePtr 设置为 nullptr
Circle *circlePtr = dynamic_cast<Circle*>(shapePtr);
circlePtr->draw();
```

这会导致空指针解引用 (NULL pointer dereference) 错误

```
[1]    11531 segmentation fault (core dumped)  ./downcasting
```

这是因为 `dynamic_cast` 在转换失败时会返回 `nullptr`，而不是抛出异常

所以我们需要在转换后检查是否为 `nullptr`

```cpp
Circle *circlePtr = dynamic_cast<Circle*>(shapePtr);
if (circlePtr == nullptr)
{
    std::cout << "Conversion failed" << "\n";
    return 1; // 提前退出程序
}

circlePtr->setRadius(10);
circlePtr->draw(); // 输出: Drawing a circle with radius 10
```

### static_cast(静态向下转换)

这通常在**性能敏感**或能**确定类型匹配**的情况下使用

例如有个 `Shape` 指针类型的数组/列表，里面全部是 `Circle` 对象的指针

```cpp
Circle circle1(1);
Circle circle2(2);
Circle circle3(3);

Shape *shapes[] = {&circle1, &circle2, &circle3};
```

那么此时就可以使用 `static_cast` 进行静态向下转换

```cpp
void drawCircles(Shape **shapes, int size)
{
    for (int i = 0; i < size; i++)
    {
        Circle *circlePtr = static_cast<Circle*>(shapes[i]);
        circlePtr->draw();
    }
}
```

```cpp
drawCircles(shapes, 3);
```
