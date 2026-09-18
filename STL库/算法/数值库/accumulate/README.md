# accumulate

`std::accumulate` 是 `<numeric>` 头文件中最基础的数值算法，用于对范围内的元素进行累积运算

简单来说，它从初始值出发，依次将每个元素"累加"到结果中——默认是求和，但通过自定义二元操作可以实现求积、拼接、取最大值等任意累积逻辑

## 函数签名

### C++20 前

```cpp
template< class InputIt, class T >
T accumulate( InputIt first, InputIt last, T init );

template< class InputIt, class T, class BinaryOperation >
T accumulate( InputIt first, InputIt last, T init, BinaryOperation op );
```

### C++20 起（constexpr）

```cpp
template< class InputIt, class T >
constexpr T accumulate( InputIt first, InputIt last, T init );

template< class InputIt, class T, class BinaryOperation >
constexpr T accumulate( InputIt first, InputIt last, T init, BinaryOperation op );
```

- `first`/`last`：要累积的范围的起始和结束迭代器
- `init`：累积的初始值，**同时决定返回类型**（这是最关键的参数）
- `op`：自定义二元操作，签名等价于 `Ret op(const Type1& a, const Type2& b);`

## 计算逻辑

等价于：

```cpp
T result = init;
for (auto it = first; it != last; ++it)
    result = op(result, *it);  // 默认 op 为 std::plus<>()
return result;
```

**关键细节**：`op` 的第一个参数是当前的累积值，第二个参数是当前元素

## 示例

### 默认求和

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
int sum = std::accumulate(v.begin(), v.end(), 0);
// sum = 0 + 1 + 2 + 3 + 4 + 5 = 15
```

- 初始值 `0` 决定了返回类型为 `int`

### 求积

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>());
// product = 1 × 1 × 2 × 3 × 4 × 5 = 120
```

- 初始值 `1`（乘法单位元），操作改为 `std::multiplies`

### 拼接字符串

```cpp
std::vector<std::string> words = {"Hello", " ", "World", "!"};
std::string result = std::accumulate(words.begin(), words.end(), std::string(""));
// result = "Hello World!"
```

- 初始值 `std::string("")` 决定了返回类型为 `std::string`，元素间的 `+` 即字符串拼接

### 自定义操作：求阶乘

```cpp
int factorial(int n) {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 1);  // v = {1, 2, ..., n}
    return std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>());
}
// factorial(5) = 120
```

### 自定义操作：求最大值

```cpp
std::vector<int> v = {3, 7, 2, 9, 5};
int max_val = std::accumulate(v.begin(), v.end(), v[0],
    [](int a, int b) { return std::max(a, b); });
// max_val = 9
```

### 自定义操作：计算加权平均

```cpp
std::vector<double> values = {80.0, 90.0, 70.0};
std::vector<double> weights = {0.3, 0.5, 0.2};

double weighted_sum = std::accumulate(values.begin(), values.end(), 0.0,
    [i = 0, &weights](double acc, double val) mutable {
        return acc + val * weights[i++];
    });
// weighted_sum = 80×0.3 + 90×0.5 + 70×0.2 = 83.0
```

## ⚠️ 常见陷阱

### 1. 初始值决定返回类型

这是 `std::accumulate` 最容易踩的坑：

```cpp
std::vector<int> v = {100, 200, 300};

// ❌ 错误：初始值 0 是 int，累积过程在 int 范围内运算，可能溢出
int sum = std::accumulate(v.begin(), v.end(), 0);

// ✅ 正确：初始值 0L 是 long，累积过程在 long 范围内运算
long sum = std::accumulate(v.begin(), v.end(), 0L);

// ✅ 正确：初始值 0.0 是 double，累积过程在 double 范围内运算
double sum = std::accumulate(v.begin(), v.end(), 0.0);
```

**原理**：`init` 的类型 `T` 就是累积变量的类型，每个元素会被隐式转换为 `T` 再参与运算

### 2. 空范围的行为

```cpp
std::vector<int> v;  // 空
int result = std::accumulate(v.begin(), v.end(), 42);
// result = 42（直接返回初始值，不执行任何操作）
```

- 对空范围调用 `accumulate` 是合法的，结果就是初始值本身

### 3. 字符串拼接的性能问题

```cpp
// ❌ 低效：每次拼接都产生临时 string，O(n²) 复杂度
std::string result = std::accumulate(words.begin(), words.end(), std::string(""));

// ✅ 高效：预分配空间后手动拼接，O(n) 复杂度
std::string result;
result.reserve(total_length);
for (const auto& w : words) result += w;
```

- `accumulate` 对字符串拼接的语义正确，但性能不如手动循环（无法预分配）

### 4. 浮点数累积的精度损失

```cpp
// 大量浮点数求和时，accumulate 从左到右顺序累积，可能产生显著精度损失
// 若需高精度求和，考虑 Kahan summation 或 std::reduce（C++17 并行，顺序不确定）
```

## 常见用途

| 用途 | 初始值 | 操作 |
| :-- | :--: | :--: |
| 求和 | `0` | 默认（`+`） |
| 求积 | `1` | `std::multiplies` |
| 字符串拼接 | `""` | 默认（`+`） |
| 求最大值 | 首元素 | `std::max` |
| 求最小值 | 首元素 | `std::min` |
| 计数满足条件的元素 | `0` | lambda 判断后 `+1` |
| 计算向量点积 | `0.0` | 逐元素相乘后累加 |

### 计数满足条件的元素

```cpp
std::vector<int> v = {1, -2, 3, -4, 5};
int neg_count = std::accumulate(v.begin(), v.end(), 0,
    [](int count, int x) { return count + (x < 0 ? 1 : 0); });
// neg_count = 2
```

### 计算向量点积

```cpp
std::vector<double> a = {1.0, 2.0, 3.0};
std::vector<double> b = {4.0, 5.0, 6.0};

double dot = std::accumulate(a.begin(), a.end(), 0.0,
    [it = b.begin()](double acc, double val) mutable {
        return acc + val * (*it++);
    });
// dot = 1×4 + 2×5 + 3×6 = 32.0
```

## 与相关算法的区别

| 算法 | 行为 | 头文件 | 并行支持 |
| :--: | :--: | :--: | :--: |
| `accumulate` | 从左到右顺序累积，结果确定 | `<numeric>` | ❌ |
| `reduce` (C++17) | 可并行累积，结果顺序不确定 | `<numeric>` | ✅ |
| `inner_product` | 两范围对应元素相乘后累加（点积/内积） | `<numeric>` | ❌ |
| `partial_sum` | 逐前缀累积，输出每个前缀的结果 | `<numeric>` | ❌ |

- `std::reduce`（C++17）是 `accumulate` 的并行版本，但不保证从左到右的顺序，因此浮点数结果可能不同，且自定义操作必须满足交换律和结合律
- `std::inner_product` 可以实现点积，但 `accumulate` 配合 lambda 更灵活

## 头文件

```cpp
#include <numeric>  // std::accumulate
```