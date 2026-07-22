# getter & setter

getter & setter 是面向对象编程中常用的两个概念，它们分别用于获取和设置对象的属性，通常用于private属性的访问和修改

- `getter`: 用于返回对象的属性值
- `setter`: 用于设置对象的属性值

## 语法

```cpp
class 类名 {
    private:
        类型 属性名;
    public:
        类型 getter方法名() {    }
        void setter方法名(参数列表) {    }
}；
```

## 示例

现在有个 `Stove` 类，它有一个 `temperature` 属性

```cpp
class Stove {
    public:
        int temperature = 0;
};
```

但是这样是不安全的，因为 `temperature` 属性是公有的，任何人都可以随意修改它

```cpp
Stove stove;
stove.temperature = 100; // 这样就修改了温度
```

为了安全起见，需要把 `temperature` 属性设置为私有

```cpp
class Stove {
    private:
        int temperature = 0;
};
```

但是这样就不能直接访问 `temperature` 属性了

```
getter_and_setter.cpp:10:11: error: ‘int Stove::temperature’ is private within this context
```

这时就需要使用 `getter` 和 `setter` 方法了

### getter

```cpp
class Stove {
    private:
        int temperature = 0;
    public:
        int getTemperature()
        {
            return this->temperature;
        }
};
```

### setter

```cpp
class Stove {
    private:
        int temperature = 0;
    public:
        void setTemperature(int temp)
        {
            if (temp < 0)
                this->temperature = 0;
            else if (temp > 1000)
                this->temperature = 1000;
            else
                this->temperature = temp;
        }
};
```

### 使用

```cpp
Stove stove;
stove.setTemperature(500); // 设置温度
std::cout << "Current temperature: " << stove.getTemperature() << "\n"; // 获取温度
```
