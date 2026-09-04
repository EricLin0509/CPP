# `std::hive` 技术拆解

> [!WARNING]
> 本文代码经过了大量精简，不等同于 `std::hive` 容器的实现，请勿**直接用于生产环境!**

我们以实现字符串为例，来详细讲解 `std::hive` 容器的底层设计

## 链表的痛点

通常我们希望字符串可以扩展大小

这有两种方式实现

- 动态数组: `std::vector`
- 双向链表: `std::list`

但使用动态数组，扩容需要先分配新内存，再将旧内存的数据复制到新内存中，效率较低

而使用链表，扩容只需要在链表尾部插入新节点，效率较高

```c
typedef struct Node {
    char data;
    struct Node* next;
} Node;

typedef struct String {
    Node *head;
    Node *tail;
} String;
```

但是如果我们尝试插入或删除数量很大的字符串，链表的性能会急剧下降

例如说要插入长度为 1000 的字符串，上述实现会需要插入 1000 个节点，这会导致严重的碎片化

## 解决方案——块分配

现在的链表节点只能保存一个字符，那可不可以一个节点保存多个字符呢

当然可以，这就是块分配技术

```c
typedef struct Block {
    char data[8];
    struct Block* next;
    struct Block* prev;
} Block;
```

- 这里为了方便，我们假设每个块可以保存 8 个字符
    - `std::hive` 通常为 `64KB` 的块

现在我们插入长度为 1000 的字符串，只需要分配 125 个块 (1000 / 8 = 125)，这大大减少了碎片化

但现在有个问题，怎么表示删除字符串呢

## 标记位

我们可以再保存一个位数组来表示当前块中的字符是否被删除

```c
typedef struct Block {
    char data[8];
    bool occupied[8];
    struct Block* next;
    struct Block* prev;
} Block;
```

- `occupied`: 表示当前位是否被占用

这样，如果要删除块中的字符，只需要将对应的位设置为 `false`，这样就不会影响到其他块中的字符

但是这样有个问题，如果这个块的大小为 1024 个字符，那么个块分配需要 1024 * 2 = 2048 个字节

这样有一半的内存是浪费的

同时如果需要读取或写入块标记位，需要遍历标记位数组

```c
for (int i = 0; i < 1024; i++)
{
    /* 如果当前位没被占用，就跳出循环 */
    if (!occupied[i])
        break;
}
```

这样的时间复杂度是 `O(n)`

那有什么方案可以解决这个问题呢

## 位掩码

我们可以使用位掩码 (bitmask) 技术来解决这个问题

### 什么是位掩码

位掩码是一种将多个位组合在一起的技巧

这通常用于把多个布尔值放在一个无符号整数中，一个位 (bit) 表示一个布尔值

例如说，我们可以把 8 个布尔值放在一个无符号整数中

```c
typedef struct Block {
    char data[8];
    uint8_t occupied; // 8位
    struct Block* next;
    struct Block* prev;
} Block;
```

这里我们只需要额外1个字节就能表示所有位是否被占用，同时把移动标记位数组的时间复杂度降低到 `O(1)`

### 移动到目标位

通过左移 `<<` 操作，我们可以 `1` 这个数移动到目标位

这个 1 在二进制上是 `0b00001`，表示第1位

```c
uint8_t mask = 1 << 3; // 到达第4位
```

通过左移 3 位，此时 `1` 在二进制上是 `0b01000`，表示第4位

### 检查位是否被占用

使用位与 `&` 操作，我们可以检查目标位是否被占用

位与操作会比较两个数的每一位，如果两个数的每一位都为 `1`，那么结果为 `1`，否则结果为 `0`

```
0100
0110
---- &
0100
```

在 `if` 逻辑表达式中，非零表示真，零表示假

```c
if (occupied & mask)
    printf("第4位被占用\n");
```

### 修改目标位

#### 置位

使用位或 `|` 操作，我们可以将目标位设置为 1

```c
occupied |= mask; // 置位
```

#### 清位

使用位与 `&` 操作和对操作位进行 `~` 取反操作，我们可以将目标位清为 0

```c
occupied &= ~mask; // 清位
```

## 实现

那么数据结构如下

```c
#define BLOCK_SIZE 8
#define RECYCLE_THRESHOLD 10 // 最多只保存10个回收块

typedef struct Block {
    char data[BLOCK_SIZE];
    uint8_t occupied; // 8位
    struct Block* next;
    struct Block* prev;
} Block;

typedef struct Hive {
    Block *head;
    Block *tail;
    Block *recycled_list; // 用单链表存储回收块
    size_t block_count;
    size_t recycled_count;
} Hive;
```

### 重置块

```c
void block_reset(Block *block)
{
    if (!block) return;

    block->occupied = 0;
    block->next = NULL;
    block->prev = NULL;
}
```

### 创建块

```c
Block *create_block(void)
{
    Block *block = malloc(sizeof(Block));
    if (block == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    block_reset(block); // 重置块数据
    return block;
}
```

### 插入新的块

```c
void insert_block(Hive *hive)
{
    if (!hive) return;

    Block *new_block = create_block();
    
    if (hive->head == NULL)
    {
        assert(hive->tail == NULL); // 需要保证整个Hive为空
        hive->head = hive->tail = new_block;
    }
    else
    {
        hive->tail->next = new_block;
        new_block->prev = hive->tail;
        hive->tail = new_block;
    }

    hive->block_count++;
}
```

#### 使用回收块

我们可以判断回收块链表 `recycled_list` 是否不为空

- 如果不为空，就从回收块链表中取出一个块
- 如果为空，就创建新的块

```c
void insert_block(Hive *hive)
{
    if (!hive) return;

    Block *new_block = NULL;
    if (hive->recycled_count > 0)
    {
        new_block = hive->recycled_list;

        /* 从回收列表中移除 */
        hive->recycled_list = hive->recycled_list->next;
        hive->recycled_count--;

        /* 重置回收块 */
        block_reset(new_block); // 重置回收块数据
        printf("Use recycled block %p.\n", new_block); // 打印回收块信息
    }
    else
    {
        new_block = create_block(); // 如果没有回收块，就创建一个新的块
    }
    
    if (hive->head == NULL)
    {
        assert(hive->tail == NULL); // 需要保证整个Hive为空
        hive->head = hive->tail = new_block;
    }
    else
    {
        hive->tail->next = new_block;
        new_block->prev = hive->tail;
        hive->tail = new_block;
    }

    hive->block_count++;
}
```

### 块回收

如果 `occupied` 为 0 时，说明这个块可以回收

```c
void recycle_block(Hive *hive, Block *block)
{
    if (!hive || !block) return;

    if (block->prev) block->prev->next = block->next;
    else hive->head = block->next; // 头节点边界处理

    if (block->next) block->next->prev = block->prev;
    else hive->tail = block->prev; // 尾节点边界处理

    hive->block_count--;

    free(block);
}
```

#### 判断是否需要回收

通过比较 `recycled_count` 和 `RECYCLE_THRESHOLD` 的大小

- 如果 `recycled_count` 小于 `RECYCLE_THRESHOLD`，就回收块
- 如果 `recycled_count` 大于等于 `RECYCLE_THRESHOLD`，就销毁块
    - 在 `std::hive` 中，它会检测系统内存使用情况，如果内存使用率过高，就销毁块
    - 这里为了方便演示，这里我们假设 `RECYCLE_THRESHOLD` 为 10
        - 实际可以通过一些表达式改变这个 `RECYCLE_THRESHOLD` 的值

```c
void recycle_block(Hive *hive, Block *block)
{
    if (!hive || !block) return;

    if (block->prev) block->prev->next = block->next;
    else hive->head = block->next; // 头节点边界处理

    if (block->next) block->next->prev = block->prev;
    else hive->tail = block->prev; // 尾节点边界处理

    hive->block_count--;

    if (hive->recycled_count < RECYCLE_THRESHOLD) // 如果回收列表未满，就将块加入回收列表
    {
        block_reset(block); // 重置回收块数据
        block->next = hive->recycled_list;
        hive->recycled_list = block; // 将块加入回收列表的头部
        hive->recycled_count++; // 更新回收块计数
        printf("Block %p added to recycled list.\n", block); // 打印回收块信息
        return;
    }

    free(block);
}
```

- 回收池中的块会在下次插入新块时被复用，但不会用于填充已有块内的空洞
- 这样，就每次可以不需要分配内存，从而减少内存分配带来的性能损失

### 找到第一个空闲位

```c
int find_free_slot(Block *block)
{
    if (!block) return -1;
    if (block->occupied == 0xFF) return -1; // 如果所有位都被占用，就返回-1

    return __builtin_ctz((unsigned int)(~block->occupied & 0xFF)); // 找到第一个空闲位
}
```

- `~`: 取反操作，在二进制上，`0`变为 `1`，`1` 变为 `0`
- `~block->occupied & 0xFF`: 强制截断，只取低8位
- `__builtin_ctz`: 表示末尾零的个数
    - `__builtin_` 表示 GCC 或 Clang 的内置函数，即由编译器直接生成汇编代码

#### 通用实现

有时候 `__builtin_ctz` 不可用 (如 MSVC)，可以使用通用实现

```c
int ctz(uint8_t x)
{
    int count = 0;
    while ((x & 1) == 0)
    {
        count++;
        x >>= 1; // 右移一位
    }
    return count;
}
```

- 这个跟 `__builtin_ctz` 一样，传入 `0` 属于未定义行为

```c
#ifdef __has_builtin
    #if __has_builtin(__builtin_ctz)
        return __builtin_ctz((unsigned int)(~block->occupied & 0xFF));// 返回第一个空闲位
    #endif
#endif
        return ctz(~block->occupied & 0xFF);
```

### 遍历字符

```c
void traverse_char(Hive *hive)
{
    if (!hive || !hive->head) return;

    Block *block = hive->head;
    while (block)
    {
        for (int i = 0; i < BLOCK_SIZE; i++) // 遍历当前块的每个字符
        {
            if (block->occupied & (1 << i)) // 如果块数据位被占用，就打印出来
                printf("%c", block->data[i]);
        }
        block = block->next;
    }
}
```

- 总迭代次数与总槽位数成正比 (有效字符 + 空洞)，整体为 O(N)，但常数因子比连续存储容器大

### 追加字符

```c
void append_char(Hive *hive, const char *str)
{
    if (!hive || !str) return;

    size_t char_len = strlen(str);
    size_t curr_char = 0;
}
```

#### 处理 hive 为空的情况

如果 hive 为空，就需要先插入新的块

```c
void append_char(Hive *hive, const char *str)
{
    if (!hive || !str) return;

    size_t char_len = strlen(str);
    size_t curr_char = 0;

    if (!hive->tail)
    {
        assert(hive->head == NULL); // 需要保证整个Hive为空
        insert_block(hive); // 插入一个新块
    }

    while (curr_char < char_len) {    }
}
```
#### 找到第一个空闲位

```c
void append_char(Hive *hive, const char *str)
{
    if (!hive || !str) return;

    size_t char_len = strlen(str);
    size_t curr_char = 0;

    if (!hive->tail)
    {
        assert(hive->head == NULL); // 需要保证整个Hive为空
        insert_block(hive); // 插入一个新块
    }

    while (curr_char < char_len)
    {
        int slot = find_free_slot(hive->tail); // 找到第一个空闲位
    }
}
```

#### 写入字符

- 如果找到空闲位，就写入字符
- 如果没有找到空闲位，在写入字符之前，插入新的块

```c
void append_char(Hive *hive, const char *str)
{
    if (!hive || !str) return;

    size_t char_len = strlen(str);
    size_t curr_char = 0;

    if (!hive->tail)
    {
        assert(hive->head == NULL); // 需要保证整个Hive为空
        insert_block(hive); // 插入一个新块
    }

    while (curr_char < char_len)
    {
        int slot = find_free_slot(hive->tail); // 找到第一个空闲位
        
        if (slot == -1)
        {
            insert_block(hive);
            slot = 0; // 新块的第一个槽位
        }

        /* 写入字符 */
        hive->tail->data[slot] = str[curr_char];
        hive->tail->occupied |= (1 << slot);
        curr_char++;
    }
}
```

### 删除字符

这里实现所有匹配字符的删除

```c
void delete_char(Hive *hive, const char c)
{
    if (!hive || !c) return;

    Block *block = hive->head;
    while (block) {    }
}
```

#### 遍历并匹配

```c
void delete_char(Hive *hive, const char c)
{
    if (!hive || !c) return;

    Block *block = hive->head;
    while (block)
    {
        for (int i = 0; i < BLOCK_SIZE; i++) // 遍历当前块的每个字符
        {
            if (block->data[i] == c) // 如果当前字符匹配，就删除
                block->occupied &= ~(1 << i); // 标记为未占用
        }
    }
}
```

- 通过标记目标位未占用，从而避免了如 `std::vector` 一样的问题，即需要移动后面的元素和后面元素迭代器失效的问题
    - 这就是 `std::hive` 中 `skipfield` 逻辑删除的优势

#### 回收可能为空的块

如果当前块的 `occupied` 为 0，说明这个块可以回收

```c
void delete_char(Hive *hive, const char c)
{
    if (!hive || !c) return;

    Block *block = hive->head;
    while (block)
    {
        Block *next = block->next;
        for (int i = 0; i < BLOCK_SIZE; i++) // 遍历当前块的每个字符
        {
            if (block->data[i] == c) // 如果当前字符匹配，就删除
                block->occupied &= ~(1 << i); // 标记为未占用
        }
        
        Block *curr = block;
        block = block->next;

        if (curr->occupied == 0) // 如果当前块所有位都空闲，就回收
            recycle_block(hive, curr);
    }
}
```

### 清除 hive

```c
void clear_hive(Hive *hive)
{
    if (!hive || !hive->head) return;

    Block *block = hive->head;
    while (block)
    {
        Block *next = block->next;
        free(block);
        block = next;
    }

    block = hive->recycled_list;
    while (block)
    {
        Block *next = block->next;
        free(block);
        block = next;
    }

    hive->head = hive->tail = NULL;
    hive->block_count = 0;
}
```

## 局限性和标准差异

| 特性 | 本实现 | `std::hive` |
| :--: | :---: | :-------------: |
| 元素类型 | 仅支持 `char` 泛型模板 | 支持任意类型 (T) |
| 插入位置 | 总是追加到尾部块的第一个空闲槽；若尾块满则新建块 | 支持通过迭代器在任意位置插入，且优先填充已有的空洞 (内存复用) |
| 删除操作 | 按值扫描所有元素，删除所有匹配项 (时间复杂度 O(N)) | 通过迭代器进行 O(1) 的单个元素删除，不保证删除所有相同值 |
| 迭代器稳定性 | 部分稳定: 删除后若块被回收，指向该块的迭代器失效；但其他块内元素不受影响 | 完全稳定: 插入和删除操作不会使任何已有迭代器、指针或引用失效 (除非指向被删除的元素本身) |
| 空块回收策略 | 块变空后立即物理回收 (进入回收池或释放) | 保留空块（仅逻辑删除），由实现决定何时释放，以最大化迭代器稳定性和避免频繁内存操作 |
| 块大小 | 固定为 8 字节 (硬编码) | 通常为 64KB 量级，且允许通过分配器或实现策略调整 |
| 内存分配器 | 使用 `malloc`/`free` 内置 | 支持自定义分配器 (Allocator 模板参数)，并提供 `std::pmr::hive` 多态内存资源版本 |
| 容量管理 | 仅提供 `clear_hive` 支持 | `reserve()`、`shrink_to_fit()`、`max_size()` 等容量控制接口 |
| 追加 | O(1) | O(1) |
| 删除 | O(N) | O(1) |
| 遍历 | O(N) | O(N) |

## 补充说明

- 本实现旨在展示核心思想：分块存储、位图标记、空闲块复用，代码简洁易懂，适合学习底层原理
- 本示例为简化，插入时仅使用尾块；标准库会扫描所有块以填充空洞，从而实现更好的空间复用
- 标准库 `std::hive` 的设计目标是在大规模、频繁修改、需要迭代器稳定的场景 (如游戏实体管理、实时系统) 中提供最优性能，其实现细节远比示例复杂 (如跳字段、块大小动态调整、内存回收策略等)
- 若要在实际项目中使用，请直接采用 C++26 标准库，或参考成熟的开源实现 (如 LLVM 的 `libc++` 或 GCC 的 `libstdc++` 中的 hive 实现)
