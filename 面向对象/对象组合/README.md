# 对象组合

对象组合 (Composition) 是一种关系，其中一个类包含另一个类的实例

这种关系类似于“包含”或“嵌套”关系，但不是**继承**关系

## 语法

### 组合关系

```cpp
class 内部类 {
    // 内部类的成员和方法
};

class 外部类 {
    内部类 内部对象; // 外部类包含内部类的实例
    // 外部类的成员和方法
};
```

- 如果外部类被销毁，内部类也会同时被销毁

### 在外部类调用内部类构造函数

```cpp
外部类 (参数列表) : 内部类(参数列表)
{
    // 外部类的构造函数代码
}
```

## 示例

现在有个 `Engine` 类

```cpp
class Engine {
    private:
        std::string model;
        std::string type;

    public:
        Engine(const std::string& model, const std::string& type) // 使用引用传递避免不必要的内存拷贝
        {
            this->model = model;
            this->type = type;
        }
};
```

创建一个 `Car` 类，它包含一个 `Engine` 对象

```cpp
class Car {
    private:
        std::string make;
        std::string model;
        Engine engine;
};
```

### 设计构造函数

那怎么实现在 `Car` 类中初始化 `Engine` 对象呢

我们需要在 `Car` 类的构造函数中调用 `Engine` 类的构造函数

大致如下

```cpp
Car(const std::string& make, const std::string& model, const std::string& engineModel, const std::string& engineType) : engine(engineModel, engineType)
```

完整代码如下

```cpp
class Car {
    private:
        std::string make;
        std::string model;
        Engine engine;

    public:
        Car(const std::string& make, const std::string& model, const std::string& engineModel, const std::string& engineType) : engine(engineModel, engineType)
        {
            this->make = make;
            this->model = model;

        }
};
```
