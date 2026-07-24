# const成员函数

在C++中，const成员函数是指那些不能修改类或者结构体成员变量的成员函数

它们通常用于读取对象的状态，或者在不修改对象的情况下执行某些操作

## 语法

### 结构体

```cpp
struct 结构体名 {
    返回类型 成员函数名() const
    {
        // 成员函数体
    }
};
```

- 只有C++支持在结构体定义函数，C语言不支持 (需要使用函数指针替代)

### 类

```cpp
class 类名 {
    public:
        返回类型 成员函数名() const
        {
            // 成员函数体
        }
};
```

### 示例

现在有个类 `Base`

```cpp
class Base {
    private:
        int x;
    public:
        Base(int x)
        {
            this->x = x;
        }

        int getX()
        {
            return x;
        }

        void setX(int x)
        {
            this->x = x;
        }
};
```

由于 `getX` 函数没有修改 `Base` 对象的成员变量，那么它就可以声明为 const 成员函数

### 声明为 const 成员函数

```cpp
class Base {
    private:
        int x;
    public:
        Base(int x)
        {
            this->x = x;
        }

        int getX() const
        {
            return x;
        }

        void setX(int x)
        {
            this->x = x;
        }
};
```

### 在 const 成员函数中修改成员变量

如果想要在 const 成员函数中允许修改成员变量，需要使用 `mutable` 关键字声明成员变量

```cpp
class Base {
    private:
        int x;
        mutable bool canEdit;
    public:
        Base(int x)
        {
            this->x = x;
            this->canEdit = false;
        }

        int getX() const
        {
            return x;
        }

        void changeEdit() const
        {
            this->canEdit ^= true; // 这是被允许的
            // x = 10; // 这是不允许的，因为此时 x 是 const 成员变量
        }


        void setX(int x)
        {
            this->canEdit = true;
            this->x = x;
        }
};
```


### 对于只读对象或结构体

对于只读对象或结构体，只能访问 const 成员函数

```cpp
const Base obj(10);
int x = obj.getX();
obj.setX(20); // 报错
```

```
const_members.cpp:27:13: error: passing ‘const Base’ as ‘this’ argument discards qualifiers
```
