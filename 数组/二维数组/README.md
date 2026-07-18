# 二维数组

二维数组是数组的数组，可以理解为一个二维的表格

通常用于矩阵运算，如矩阵乘法、矩阵转置等

## 语法

```cpp
int arr[行数][列数] = {
    {第1行},
    {第2行},
    ...
};
```
- 列数**必须显式指定**，行数可以省略

## 示例

创建一个3x3的二维数组

```cpp
std::string arr[3][3] = {
    {"E46 M3 CSL", "E60 M5", "E63 M6"}, // BMW
    {"W204 C63 AMG", "W212 E63 AMG", "C218 CLS63 AMG"}, // Mercedes-Benz
    {"B7 RS4", "B7 RS5", "C6 RS6"} // Audi
};
```

### 遍历二维数组

#### 计算行数和列数

```cpp
int rows = sizeof(arr) / sizeof(arr[0]); // 行数
int cols = sizeof(arr[0]) / sizeof(arr[0][0]); // 列数
```

#### 遍历行

```cpp
for (int i = 0; i < rows; i++) // 遍历行
{

}
```

#### 遍历列

```cpp
for (int i = 0; i < rows; i++) // 遍历行
{
    for (int j = 0; j < cols; j++) // 遍历列
    {
        std::cout << arr[i][j] << "    ";
    }
    std::cout << "\n";
}
```