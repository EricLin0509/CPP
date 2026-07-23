# 销毁函数

在C++中，销毁函数 (Destructor) 是一种特殊的方法

用于在对象生命周期结束时自动调用

它通常用于释放对象占用的资源，例如内存、文件句柄等

## 语法

销毁函数需要以 `~` 符号开头，后面跟着类名

```cpp
class 类名 {
    权限修饰符:
        ~类名()
        {
            // 销毁函数的代码
        }
};
```

- 编译器会默认为每个类提供一个默认的销毁函数
- 当这个类不在后续代码中被使用时，会自动调用这个默认的销毁函数
- 销毁函数**不能**有参数

## 示例

假设现在有个 `Person` 类

```cpp
class Person {
    private:
        int *age = new int;
    public:
        Person(int age)
        {
            *(this->age) = age;
        }

        int getAge()
        {
            return *(this->age);
        }
};
```

但这个 `age` 是动态分配的内存

如果不在 `Person` 对象生命周期结束时释放这个内存，就会导致内存泄漏

使用 `valgrind` 进行内存泄漏检测

```
==17274==   total heap usage: 3 allocs, 2 frees, 74,756 bytes allocated
==17274== 
==17274== LEAK SUMMARY:
==17274==    definitely lost: 4 bytes in 1 blocks
==17274==    indirectly lost: 0 bytes in 0 blocks
==17274==      possibly lost: 0 bytes in 0 blocks
==17274==    still reachable: 0 bytes in 0 blocks
==17274==         suppressed: 0 bytes in 0 blocks
==17274== Rerun with --leak-check=full to see details of leaked memory
==17274== 
==17274== For lists of detected and suppressed errors, rerun with: -s
==17274== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

- 这里有一个 `4 bytes in 1 blocks` 的泄漏，对应 `age` 的内存泄漏

那么需要手动声明销毁函数并在其中释放内存

### 手动声明销毁函数

```cpp
class Person {
    private:
        int *age = new int;
    public:
        Person(int age)
        {
            *(this->age) = age;
        }

        ~Person()
        {
            std::cout << "Destroying this object" << "\n";
            delete age;
        }

        int getAge()
        {
            return *(this->age);
        }
};
```

那么当 `Person` 对象生命周期结束时，会看到以下输出

```
Destroying this object
```

说明销毁函数已经正确调用并释放了内存

使用 `valgrind` 进行内存泄漏检测

```
==17863== 
==17863== HEAP SUMMARY:
==17863==     in use at exit: 0 bytes in 0 blocks
==17863==   total heap usage: 3 allocs, 3 frees, 74,756 bytes allocated
==17863== 
==17863== All heap blocks were freed -- no leaks are possible
==17863== 
==17863== For lists of detected and suppressed errors, rerun with: -s
==17863== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

这里没有内存泄漏，说明销毁函数已经正确释放了内存