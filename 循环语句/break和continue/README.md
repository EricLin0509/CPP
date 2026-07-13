# break和continue

break和continue是循环语句中的两个关键字

它们的作用是控制循环的执行

- break：终止循环，不再执行循环体中的代码，直接跳出循环
- continue：跳过当前循环的剩余代码，直接进入下一次循环

## break

break的作用是终止循环，不再执行循环体中的代码，直接跳出循环

```cpp
for (int i = 0; i < 10; i++)
{
    if (i == 5)
        break;
    cout << i << "\n";
}
```

此时当i == 5时，break语句被执行，循环终止，不再执行循环体中的代码，直接跳出循环

输出结果为：

```
0
1
2
3
4
```

## continue

continue的作用是跳过当前循环的剩余代码，直接进入下一次循环

```cpp
for (int i = 0; i < 10; i++)
{
    if (i == 5)
        continue;
    cout << i << "\n";
}
```

此时当i == 5时，continue语句被执行，循环终止，不再执行循环体中的代码，直接进入下一次循环

输出结果为：

```
0
1
2
3
4
6
7
8
9
```
