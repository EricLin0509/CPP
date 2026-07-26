# 模板类

在C++中，模板类 (template class) 是一种允许在编译时确定类型参数的类

模板类可以接受任意类型的参数，并在编译时根据这些参数生成相应的代码

## 语法

```cpp
template <typename T1, typename T2>
class 类名 {
    private:
        T1 变量1;
        T2 变量2;
        ...
    public:
        类名(T1 t1, T2 t2) : 变量1(t1), 变量2(t2) {} // 构造函数
        // 其他成员方法
};
```

- `template`: 模板关键字
- `typename`: 指定类型参数
- `T1, T2`: 类型参数，可以有多个
- 模板类通常需要构造函数，以便在实例化时初始化变量

## 示例

现在创建一个模板类 `Stack`，用于存储任意类型的元素，并提供基本的栈操作

```cpp
using std::vector;

template <typename Type>

class Stack {
    private:
        vector<Type> stack;
};
```

### 使用模板类创建对象

由于编译器需要知道模板类的具体类型参数，因此在实例化模板类时需要指定类型参数

```cpp
Stack<int> intStack;
Stack<double> doubleStack;
```

### 设计成员方法

#### 栈是否为空

```cpp
bool isEmpty() const
{
    return this->stack.empty();
}
```

#### 栈大小

```cpp
int size() const
{
    return this->stack.size();
}
```

#### 入栈

```cpp
void push(Type item)
{
    this->stack.push_back(item);
}
```

#### 出栈

```cpp
Type pop()
{
    if (this->isEmpty())
    {
        std::cerr << "Stack is empty!" << "\n";
        return Type();
    }

    Type poppedItem = this->stack.back();
    this->stack.erase(this->stack.end());
    return poppedItem;
}
```

### 调用成员方法

```cpp
Stack<int> intStack;

for (int i = 1; i <= 5; i++)
{
    intStack.push(i);
}

intStack.pop();
```
