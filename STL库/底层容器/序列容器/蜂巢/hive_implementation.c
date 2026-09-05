#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

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

typedef struct {
    Block *block;
    size_t index;
} HiveIterator;

HiveIterator *iterator_create(void)
{
    HiveIterator *it = malloc(sizeof(HiveIterator));
    if (it == NULL)
        perror("malloc");

    return it;
}

void iterator_destroy(HiveIterator *it)
{
    if (!it) return;

    if (it->block)    
        it->block->ref_count--; // 释放迭代器所指向的块的引用

    free(it);
}

HiveIterator *iterator_begin(Hive *hive)
{
    if (!hive) return NULL;

    HiveIterator *it = iterator_create();
    if (!it) return NULL;

    it->block = hive->head;
    it->block->ref_count++; // 增加引用计数
    it->index = 0;

    return it;
}

HiveIterator *iterator_end(Hive *hive)
{
    if (!hive) return NULL;

    HiveIterator *it = iterator_create();
    if (!it) return NULL;

    it->block = hive->tail;
    it->block->ref_count++; // 增加引用计数
    it->index = BLOCK_SIZE - 1;

    return it;
}

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
            
        it->index += skip;
    }
}

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

Hive *create_hive(void)
{
    Hive *hive = malloc(sizeof(Hive));
    if (hive == NULL)
    {
        perror("malloc");
        return NULL;
    }

    hive->head = hive->tail = create_block(); // 创建第一个块
    hive->recycled_list = NULL;
    hive->block_count = 1;
    hive->recycled_count = 0;

    return hive;
}

void destory_hive(Hive *hive)
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

    free(hive);
}

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

/* 这里用于更新插入情况时空洞的跳转信息 */
static void insert_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;

    size_t i = it->index;
    uint8_t total_len = it->block->skipfield[i]; // 当前空洞长度（假定插入发生在 head 或 tail）
    
    // 如果插入前这里 skipfield 记录了长度 L (L > 0)
    if (total_len > 0)
    {
        // 假设当前插入点 i 是空洞的 head
        size_t head = i;
        size_t tail = i + total_len - 1;

        // 算出左侧和右侧残留空洞的长度
        size_t left_len  = i - head; // i 之前剩余的空洞长度 (若在 head 插入则为 0)
        size_t right_len = tail - i; // i 之后剩余的空洞长度

        // 1. 槽位 i 被占用，Skipfield 清零
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
}

HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;

    HiveIterator *curr = iterator_create();
    if (!curr) return NULL;

    curr->block = it->block;
    curr->index = it->index;

    // 1. 在当前块内找个空洞（从当前位置往后看，没有就放弃）
    while (curr->index < BLOCK_SIZE)
    {
        if (curr->block->skipfield[curr->index] > 0)
        {
            curr->block->data[curr->index] = c;
            curr->block->element_count++;

            // 更新 skipfield（拆分空洞）
            insert_update_skipfield(curr);

            curr->block->ref_count++; // 返回的迭代器持有引用
            return curr;
        }
        curr->index++;
    }

    // 2. 当前块没空洞了（满了），直接在它后面插一个新块！
    //    因为是无序容器，我们不关心新块插在链表中间还是尾部，只要链接上就行。
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

void hive_treverse(HiveIterator *it)
{
    if (!it || !it->block) return;

    while (it->block)
    {
        printf("%c", it->block->data[it->index]);
        iterator_next(it);
    }
}

int main(void) {
    Hive *hive = create_hive();
    if (!hive) return 1; // 创建 Hive 失败

    HiveIterator *it = iterator_begin(hive);

    /* 第一个块插入 */
    hive_insert(hive, it, 'H');
    HiveIterator *test_erase1 = hive_insert(hive, it, 'e');
    hive_insert(hive, it, 'l');
    hive_insert(hive, it, 'l');
    hive_insert(hive, it, 'o');
    hive_insert(hive, it, ' ');
    hive_insert(hive, it, 'W');
    hive_insert(hive, it, 'o');

    /* 测试空块影响，这已经改变了默认实现，只用于测试 */
    HiveIterator *test_empty = hive_insert(hive, it, ' ');
    block_reset(test_empty->block); // 重置空块数据
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        test_empty->block->data[i] = 'X';
    }

    /* 测试空块回收 */
    HiveIterator *test_erase2 = hive_insert(hive, it, 'r');
    hive_erase(hive, test_erase2);

    /* 测试空块复用 */
    hive_insert(hive, it, 'l');
    hive_insert(hive, it, 'd');
    hive_insert(hive, it, '!');

    /* 测试块内空洞影响 */
    hive_erase(hive, test_erase1);

    hive_treverse(it);
    printf("\n");

    return 0;
}
