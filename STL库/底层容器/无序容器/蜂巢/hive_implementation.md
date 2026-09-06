# `std::hive` 技术拆解

> [!WARNING]
> 本文代码经过了大量精简，不等同于 `std::hive` 容器的实现，请勿**直接用于生产环境!**

我们以字符为基本单位，来详细讲解 `std::hive` 容器的底层设计

## 链表的痛点

通常我们希望数据存储可以扩展大小

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
    - 实际 `std::hive` 是根据分配情况自动增大块大小

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

但是这样有个问题，如果这个块的大小为 1024 个字符，那么这个块分配需要 1024 * 2 = 2048 个字节

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

但使用位掩码同样有一个缺点，就是如果在块中删除了大量数据，那么在遍历标记位数组时，依旧需要依次遍历标记位数组，效率较低

例如在块中连续删除了 500 个元素，那么依旧需要依次遍历这些位

那有什么办法可以一口气跳过这些位呢

### 跳跃字段

使用跳跃字段，可以标记当前位后面有多少是空闲的

示意图如下

```
# 位掩码
| 0 | _ | 1 | _ | _ | _ | 2 |
| 1 | 0 | 1 | 0 | 0 | 0 | 1 |

# 跳跃字段
| 0 | _ | 1 | _ | _ | _ | 2 |
| 0 | 1 | 0 | 3 | X | 3 | 0 |
```

- `_`: 表示这里的数据被删除
- `X`: 空洞中间槽位 (示例中的值为 2)
    - 中间槽位迭代器不会读取它 (因为头部或尾部会直接跳过去)
    - 但插入时它作为空洞长度标记的一部分
- 采用头尾双标记设计，正向遍历时读取头部标记跳过空洞，反向遍历时读取尾部标记回退，从而实现双向迭代
    - 上图中两个 3 只是这个设计的自然结果

这样当遍历到空闲位置时，就通过块中额外的跳跃字段数组直接跳过这些位，从而提高遍历效率

```c
typedef struct Block {
    char data[8];
    uint8_t skipfield[8]; // 由于每个块为 8 个字节，所以最大跳跃距离为 8
    struct Block* next;
    struct Block* prev;
} Block;
```

- 这样会像标记位数组那样，需要额外的内存，但可以提高遍历效率
    - 这就是典型的"用空间换时间"的思想

## 实现

那么数据结构如下

```c
#define BLOCK_SIZE 8
#define RECYCLE_THRESHOLD 10 // 最多只保存10个回收块

typedef struct Block {
    char data[BLOCK_SIZE];
    uint8_t skipfield[BLOCK_SIZE]; // 跳跃字段
    size_t ref_count; // 引用计数
    size_t element_count; // 元素计数
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

    /* 初始化 `skipfield` */
    memset(block->skipfield, 0, BLOCK_SIZE);
    block->skipfield[0]= block->skipfield[BLOCK_SIZE - 1] = BLOCK_SIZE;

    block->next = NULL;
    block->prev = NULL;
    block->ref_count = 0;
    block->element_count = 0;
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
        return NULL;
    }

    block_reset(block); // 重置块数据
    return block;
}
```

### 迭代器

在 `std::hive` 中，迭代器主要由两个部分组成

- 块指针: 指向当前块
- 索引: 当前块中元素的索引

```c
typedef struct {
    Block *block;
    size_t index;
} HiveIterator;
```

### 创建迭代器

```c
HiveIterator *iterator_create(void)
{
    HiveIterator *it = malloc(sizeof(HiveIterator));
    if (it == NULL)
    {
        perror("malloc");
        return NULL;
    }

    it->block = NULL;
    it->index = 0;
    return it;
}
```

### 销毁迭代器

```c
void iterator_destroy(HiveIterator *it)
{
    if (!it) return;

    if (it->block)    
        it->block->ref_count--; // 释放迭代器所指向的块的引用

    free(it);
}
```

### 找到下一个元素

```c
void iterator_next(HiveIterator *it)
{
    if (!it || !it->block) return;
}
```

#### 索引指向下一个位置

通过 `index` 自增，可以读取下一个 `skipfield` 大小或判断是否跨块

```c
void iterator_next(HiveIterator *it)
{
    if (!it || !it->block) return;

    /* 移动到下一个位置 */
    it->index++;
}
```

#### 遍历块表

遍历整个块表，直到找到下一个非空块

```c
void iterator_next(HiveIterator *it)
{
    if (!it || !it->block) return;

    /* 移动到下一个位置 */
    it->index++;

    /* 找到有效元素或到达末尾 */
    while (it->block) {    }
}
```

#### 跨块判断

如果 `index` 超过了块大小，说明需要跨块

```c
void iterator_next(HiveIterator *it)
{
    if (!it || !it->block) return;

    /* 移动到下一个位置 */
    it->index++;

    /* 找到有效元素或到达末尾 */
    while (it->block)
    {
        /* 如果超出当前块，移到下一个块 */
        if (it->index >= BLOCK_SIZE)
        {
            Block *next = it->block->next;
            it->block->ref_count--;
            it->block = next;
            if (!it->block) return; // 末尾
            it->block->ref_count++;
            it->index = 0; // 重置索引
        }
    }
}
```

#### 读取 `skipfield`

读取下一个位置的 `skipfield` 大小，判断是否为有效位

- 如果 `skipfield` 大于 0，说明该位无效，需要跳过当前所有无效位 (`index + skipfield`)
- 如果 `skipfield` 等于 0，说明该位有效，返回该位

```c
void iterator_next(HiveIterator *it)
{
    if (!it || !it->block) return;

    /* 移动到下一个位置 */
    it->index++;

    /* 找到有效元素或到达末尾 */
    while (it->block)
    {
        /* 如果超出当前块，移到下一个块 */
        if (it->index >= BLOCK_SIZE)
        {
            Block *next = it->block->next;
            it->block->ref_count--;
            it->block = next;
            if (!it->block) return; // 末尾
            it->block->ref_count++;
            it->index = 0; // 重置索引
        }

        /* 检查当前位置 */
        uint8_t skip = it->block->skipfield[it->index];
        if (skip == 0)
            return; // 找到有效元素
            
        it->index += skip; // 跳过无效位
    }
}
```

### 遍历 Hive

只需要使用迭代器和 `iterator_next` 函数来遍历 Hive

```c
void hive_traverse(HiveIterator *it)
{
    if (!it || !it->block) return;

    while (it->block)
    {
        printf("%c", it->block->data[it->index]);
        iterator_next(it);
    }
}
```

### 在迭代器后面插入新块

```c
Block *insert_block_after(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return NULL;

    Block *new_block = NULL;
}
```

#### 尝试复用回收块

如果 `recycled_count` 大于 0，尝试复用回收块

```c
Block *insert_block_after(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return NULL;

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

    if (!new_block) return NULL;
}
```

#### 插入新块

```c
Block *insert_block_after(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return NULL;

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

    if (!new_block) return NULL;

    /* 将新块插入到当前块之后 */
    new_block->next = it->block->next;
    new_block->prev = it->block;
    if (it->block->next)
        it->block->next->prev = new_block;
    it->block->next = new_block;
}
```

#### 处理边界情况

由于是在后面插入，所以只需要处理是否是在尾部插入

```c
Block *insert_block_after(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return NULL;

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

    if (!new_block) return NULL; // 如果创建失败，返回 NULL

    /* 将新块插入到当前块之后 */
    new_block->next = it->block->next;
    new_block->prev = it->block;
    if (it->block->next)
        it->block->next->prev = new_block;
    it->block->next = new_block;

    /* 更新Hive的边界节点 */
    if (it->block == hive->tail) // 只有 tail 需要处理
        hive->tail = new_block;

    hive->block_count++;

    return new_block;
}
```

### 回收块

```c
void recycle_block(Hive *hive, Block *block)
{
    if (!hive || !block) return;
}
```

#### 处理边界情况

- 如果删除头块，需要更新 `head` 指针
- 如果删除尾块，需要更新 `tail` 指针

```c
void recycle_block(Hive *hive, Block *block)
{
    if (!hive || !block) return;

    if (block->prev) block->prev->next = block->next;
    else hive->head = block->next; // 头节点边界处理

    if (block->next) block->next->prev = block->prev;
    else hive->tail = block->prev; // 尾节点边界处理

    hive->block_count--;
}
```

#### 处理清空情况

如果整个 Hive 被清空，只需要把当前块重置成一个新 root 块

```c
void recycle_block(Hive *hive, Block *block)
{
    if (!hive || !block) return;

    if (block->prev) block->prev->next = block->next;
    else hive->head = block->next; // 头节点边界处理

    if (block->next) block->next->prev = block->prev;
    else hive->tail = block->prev; // 尾节点边界处理

    hive->block_count--;

    if (hive->block_count == 0) // 如果 Hive 中没有块了，只有 root 一个块，就重置为 root 空块
    {
        block_reset(block);
        hive->head = hive->tail = block;
        hive->block_count = 1;
        printf("Block %p reset as root empty block.\n", block);
        return;
    }
}
```

#### 尝试回收块

如果 `block_count` 小于等于 `RECYCLE_THRESHOLD`，则直接回收块，否则将块释放

```c
void recycle_block(Hive *hive, Block *block)
{
    if (!hive || !block) return;

    if (block->prev) block->prev->next = block->next;
    else hive->head = block->next; // 头节点边界处理

    if (block->next) block->next->prev = block->prev;
    else hive->tail = block->prev; // 尾节点边界处理

    hive->block_count--;

    if (hive->block_count == 0) // 如果 Hive 中没有块了，只有 root 一个块，就重置为 root 空块
    {
        block_reset(block);
        hive->head = hive->tail = block;
        hive->block_count = 1;
        printf("Block %p reset as root empty block.\n", block);
        return;
    }

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

- 在 `std::hive` 中，它会检测系统内存使用情况，如果内存使用率过高，就销毁块
- 这里为了方便演示，这里我们假设 RECYCLE_THRESHOLD 为 10
    - 实际可以通过一些表达式改变这个 RECYCLE_THRESHOLD 的值

### 插入元素

```c
HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;
}
```

#### 复制当前迭代器

我们不希望传入迭代器被修改，所以需要复制当前迭代器

```c
HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;

    HiveIterator *curr = iterator_create();
    if (!curr) return NULL;

    curr->block = it->block;
    curr->index = it->index;
}
```

#### 在当前块查找空闲位置

```c
HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;

    HiveIterator *curr = iterator_create();
    if (!curr) return NULL;

    curr->block = it->block;
    curr->index = it->index;

    /* 在当前块内找个空洞 */
    while (curr->index < BLOCK_SIZE) {    }
}
```

- 这里为了方便演示，我们只在当前块向后查找空闲位置
- 实际 `std::hive` 会构造一个空闲链表保存空闲位位置

#### 处理空洞情况

如果找到空洞，就插入元素

```c
HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;

    HiveIterator *curr = iterator_create();
    if (!curr) return NULL;

    curr->block = it->block;
    curr->index = it->index;

    /* 在当前块内找个空洞 */
    while (curr->index < BLOCK_SIZE)
    {
        if (curr->block->skipfield[curr->index] > 0)
        {
            curr->block->data[curr->index] = c;
            curr->block->element_count++;

            // 更新 skipfield (拆分空洞)
            insert_update_skipfield(curr);

            curr->block->ref_count++; // 返回的迭代器持有引用
            return curr;
        }
        curr->index++;
    }
}
```

#### 更新 `skipfield`

由于空闲位被占用，需要更新 `skipfield` 大小 (拆分空洞)

例如现在我们的块数据如下

```
# 插入前
| 1 | _ | _ | _ | 2 |
| 0 | 2 | X | 2 | 0 |
```

假设现在在第 3 位插入元素

```
# 插入后 (未更新空洞)
| 1 | _ | 3 | _ | 2 |
| 0 | 2 | X | 2 | 0 |
```

这时需要更新相邻的 `skipfield` 大小

```
# 插入后 (已更新空洞)
| 1 | _ | 3 | _ | 2 |
| 0 | 1 | 0 | 1 | 0 |
```

```c
static void insert_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;
}
```

##### 尝试读取当前 `skipfield` 大小

```c
static void insert_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;

    size_t i = it->index;
    uint8_t total_len = it->block->skipfield[i]; // 当前空洞长度 (假定插入发生在 head 或 tail)

    if (total_len == 0) return;
}
```

##### 算出左侧和右侧残留空洞的长度

```c
static void insert_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;

    size_t i = it->index;
    uint8_t total_len = it->block->skipfield[i]; // 当前空洞长度 (假定插入发生在 head 或 tail)

    if (total_len == 0) return;

    // 假设当前插入点 i 是空洞的 head
    size_t head = i;
    size_t tail = i + total_len - 1;

    // 算出左侧和右侧残留空洞的长度
    size_t left_len  = i - head; // i 之前剩余的空洞长度 (若在 head 插入则为 0)
    size_t right_len = tail - i; // i 之后剩余的空洞长度
}
```

##### 清空当前位的 `skipfield` 大小

因为已经被占用，所以当前位的 `skipfield` 为 0

```c
static void insert_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;

    size_t i = it->index;
    uint8_t total_len = it->block->skipfield[i]; // 当前空洞长度 (假定插入发生在 head 或 tail)
    
    if (total_len == 0) return;

    // 假设当前插入点 i 是空洞的 head
    size_t head = i;
    size_t tail = i + total_len - 1;

    // 算出左侧和右侧残留空洞的长度
    size_t left_len  = i - head; // i 之前剩余的空洞长度 (若在 head 插入则为 0)
    size_t right_len = tail - i; // i 之后剩余的空洞长度

    // 槽位 i 被占用，Skipfield 清零
    it->block->skipfield[i] = 0;
}
```

- 因为演示代码默认在空洞头部或尾部插入，所以当前槽位的 `skipfield` 值就是该空洞的总长度
- 若发生在空洞中间，需先通过首尾标记推导 `head` 和 `tail`

##### 更新左右 `skipfield`

```c
static void insert_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;

    size_t i = it->index;
    uint8_t total_len = it->block->skipfield[i]; // 当前空洞长度 (假定插入发生在 head 或 tail)
    
    if (total_len == 0) return;

    // 假设当前插入点 i 是空洞的 head
    size_t head = i;
    size_t tail = i + total_len - 1;

    // 算出左侧和右侧残留空洞的长度
    size_t left_len  = i - head; // i 之前剩余的空洞长度 (若在 head 插入则为 0)
    size_t right_len = tail - i; // i 之后剩余的空洞长度

    // 槽位 i 被占用，Skipfield 清零
    it->block->skipfield[i] = 0;

    /* 左边空洞长度 > 0，更新 Skipfield */
    if (left_len > 0)
    {
        it->block->skipfield[i - 1] = (uint8_t)left_len;
        it->block->skipfield[head] = (uint8_t)left_len;
    }

    /* 右边空洞长度 > 0，更新 Skipfield */
    if (right_len > 0)
    {
        it->block->skipfield[i + 1] = (uint8_t)right_len;
        it->block->skipfield[tail] = (uint8_t)right_len;
    }
}
```

#### 处理块满的情况

如果当前块满了，需要插入新块

```c
HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;

    HiveIterator *curr = iterator_create();
    if (!curr) return NULL;

    curr->block = it->block;
    curr->index = it->index;

    /* 在当前块内找个空洞 */
    while (curr->index < BLOCK_SIZE)
    {
        if (curr->block->skipfield[curr->index] > 0)
        {
            curr->block->data[curr->index] = c;
            curr->block->element_count++;

            // 更新 skipfield (拆分空洞)
            insert_update_skipfield(curr);

            curr->block->ref_count++; // 返回的迭代器持有引用
            return curr;
        }
        curr->index++;
    }

    /* 
      当前块没空洞了 (满了)，直接在它后面插一个新块！
      因为是无序容器，我们不关心新块插在链表中间还是尾部，只要链接上就行 
    */
    Block *new_block = insert_block_after(hive, curr); 
    if (!new_block)
    {
        iterator_destroy(curr);
        return NULL;
    }

    // 指向新块，插在索引0
    curr->block = new_block;
    curr->index = 0;

    // 插入数据
    curr->block->data[curr->index] = c;
    curr->block->element_count++;

    // 初始化新块：0号位有效，1~7全是空洞
    curr->block->skipfield[0] = 0;
    curr->block->skipfield[1] = BLOCK_SIZE - 1;
    curr->block->skipfield[BLOCK_SIZE - 1] = BLOCK_SIZE - 1;

    curr->block->ref_count++; // 返回的迭代器持有引用
    return curr;
}
```

### 删除元素

```c
void hive_erase(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return;

    if (it->block->skipfield[it->index] > 0) return; // 已经是空槽，不需要删除

    it->block->element_count--; // 减少元素计数
}
```

#### 标记该位为已被删除

直接更新当前位的 `skipfield` 表示 (合并空洞)

例如现在我们的块数据如下

```
# 删除前
| 1 | 2 | _ | 3 |
| 0 | 0 | 1 | 0 |
```

删除第 2 位的元素

```
# 删除后 (未更新空洞)
| 1 | _ | _ | 3 |
| 0 | 0 | 1 | 0 |
```

这时需要更新相邻的 `skipfield` 大小

```
# 删除后 (已更新空洞)
| 1 | _ | _ | 3 |
| 0 | 2 | 2 | 0 |
```

##### 读取左右 `skipfield` 大小

```c
void hive_erase(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return;

    if (it->block->skipfield[it->index] > 0) return; // 已经是空槽，不需要删除

    it->block->element_count--; // 减少元素计数

    size_t left_skip = (it->index > 0) ? it->block->skipfield[it->index - 1] : 0;
    size_t right_skip = (it->index < BLOCK_SIZE - 1) ? it->block->skipfield[it->index + 1] : 0;
}
```

##### 计算左右新空洞大小

```c
void hive_erase(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return;

    if (it->block->skipfield[it->index] > 0) return; // 已经是空槽，不需要删除

    it->block->element_count--; // 减少元素计数

    size_t left_skip = (it->index > 0) ? it->block->skipfield[it->index - 1] : 0;
    size_t right_skip = (it->index < BLOCK_SIZE - 1) ? it->block->skipfield[it->index + 1] : 0;

    /* 这里更新空洞的跳转信息 */
    size_t head = it->index - left_skip;
    size_t tail = it->index + right_skip;
    uint8_t new_skip = (uint8_t)(left_skip + right_skip + 1);
}
```

##### 更新左右 `skipfield` 大小

此时左右两侧的 `skipfield` 应该都等于 `new_skip`，即合并了左右两侧的空洞

```c
void hive_erase(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return;

    if (it->block->skipfield[it->index] > 0) return; // 已经是空槽，不需要删除

    it->block->element_count--; // 减少元素计数

    size_t left_skip = (it->index > 0) ? it->block->skipfield[it->index - 1] : 0;
    size_t right_skip = (it->index < BLOCK_SIZE - 1) ? it->block->skipfield[it->index + 1] : 0;

    /* 这里更新空洞的跳转信息 */
    size_t head = it->index - left_skip;
    size_t tail = it->index + right_skip;
    uint8_t new_skip = (uint8_t)(left_skip + right_skip + 1);

    it->block->skipfield[head] = new_skip;
    it->block->skipfield[tail] = new_skip;
}
```

- 通过更新空洞的跳转信息，从而避免了如 `std::vector` 一样的问题，即需要移动后面的元素和后面元素迭代器失效的问题
    - 这就是 `std::hive` 中 `skipfield` 逻辑删除的优势

#### 释放引用计数并回收块

```c
void hive_erase(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return;

    if (it->block->skipfield[it->index] > 0) return; // 已经是空槽，不需要删除

    it->block->element_count--; // 减少元素计数

    size_t left_skip = (it->index > 0) ? it->block->skipfield[it->index - 1] : 0;
    size_t right_skip = (it->index < BLOCK_SIZE - 1) ? it->block->skipfield[it->index + 1] : 0;

    /* 这里更新空洞的跳转信息 */
    size_t head = it->index - left_skip;
    size_t tail = it->index + right_skip;
    uint8_t new_skip = (uint8_t)(left_skip + right_skip + 1);

    it->block->skipfield[head] = new_skip;
    it->block->skipfield[tail] = new_skip;

    it->block->ref_count--; // 释放当前块的引用

    /* 当没有引用计数且元素计数为0时，回收块 */
    if (it->block->ref_count == 0 && it->block->element_count == 0)
        recycle_block(hive, it->block);

    /* 迭代器失效 */
    it->block = NULL;
}
```

## 局限性和标准差异

| 特性 | 本实现 | std::hive |
| :--: | :---: | :-------: |
| 元素类型 | 仅支持 `char` | 支持任意类型 (T) |
| 插入位置 | 从迭代器指向的块向后扫描找空洞；若满则在当前块后插入新块 | 从迭代器位置开始，优先填充任意块中的空洞，实现全局空间复用 |
| 删除 (通过迭代器) | O(1) 逻辑删除 + 合并空洞 | O(1) 迭代器稳定删除 |
| 迭代器稳定性 | 部分稳定 (块被回收则指向它的迭代器失效) | 完全稳定 (除非指向被删除元素本身) |
| 空块回收策略 | 立即回收 (进回收池或释放) | 保留空块 (仅逻辑删除)，由实现决定何时释放 |
| 块大小 | 固定 8 字节 | 通常 64KB 量级，可调整 |
| 内存分配器 | `malloc`/`free` | 支持自定义分配器 + PMR |
| 容量管理 | 仅 `clear_hive()` | `reserve()`、`shrink_to_fit()` 等 |
| 插入 | O(1) (固定块大小) | O(1) (均摊) |
| 按值查找 | 未实现 | 需手动遍历 |
| 遍历 | O(N) | O(N) |

### 真实实现的"冰山之下"

本演示代码仅用 300 行展示了 Skipfield 的核心机制。但真实的 `std::hive` 标准库实现远不止于此，它还必须在以下方面进行严密设计：

- 迭代器失效隔离：删除元素时，必须保证指向其他元素的迭代器完全不受影响。这要求每个槽位维护"版本号"，迭代器解引用时校验版本，而不是简单地用引用计数管理块
    - 本演示为了**简化块生命周期管理使用了引用计数**
- 异常安全：当元素类型 T 的拷贝/移动构造函数抛出异常时，容器必须保持状态不变 (强异常保证)。这涉及大量 try-catch 和 std::move_if_noexcept 的模板元编程
- 全局空洞查找：本演示只扫描当前块，真实实现需要能快速定位任意块中的空闲槽位，同时不能破坏迭代器稳定性。常见的方案是为每个块维护一个"空闲计数器"，并配合全局空闲块链表
- 块容量动态调整：块大小需要根据分配情况自动增长 (如 1.69 倍)，但块一旦分配就不能移动 (否则指向它的迭代器会失效)，这对内存分配器提出了苛刻的要求
- 多态内存资源 (PMR, Polymorphic Memory Resource) 支持：允许用户自定义内存池，减少频繁 `malloc`/`free` 带来的开销

`std::hive` 的前身 `plf::colony` 于 2014 年首次提案，历经标准委员会对迭代器稳定性、异常安全、内存模型的多轮反复讨论，直到 C++26 才正式落地，历时整整十二年

## 补充说明

- 本实现旨在展示核心思想：分块存储、空闲块复用、跳转字段
- 为了保持迭代器稳定和内存紧凑，真实 `std::hive` 通常不会为空洞建立全局索引，而是采用惰性扫描策略；部分高性能实现会为每个块维护一个‘空闲计数器’以加速查找
- 标准库 `std::hive` 的设计目标是在大规模、频繁修改、需要迭代器稳定的场景 (如游戏实体管理、实时系统) 中提供最优性能，其实现细节远比示例复杂 (如跳字段、块大小动态调整、内存回收策略等)
- 若要在实际项目中使用，请直接采用 C++26 标准库，或参考成熟的开源实现 (如 LLVM 的 `libc++` 或 GCC 的 `libstdc++` 中的 hive 实现)
