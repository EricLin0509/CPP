#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#define BLOCK_SIZE 8
#define RECYCLE_THRESHOLD 10 // 最多只保存10个回收块

typedef struct Block {
    char data[BLOCK_SIZE]; // 数据层 + skipfield
    uint8_t l1_bitset; // 第一层位图: 精确标记槽位是否被占用
    uint8_t l2_bitset; // 第二层位图: 标记块是否已满
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
    {
        perror("malloc");
        return NULL;
    }

    it->block = NULL;
    it->index = 0;
    return it;
}

void iterator_destroy(HiveIterator *it)
{
    if (!it) return;

    free(it);
}

/*
 * 这只是取Hive的头节点首个元素的位置
 * 并不是取第一个有效元素
*/
HiveIterator *iterator_begin(Hive *hive)
{
    if (!hive) return NULL;

    HiveIterator *it = iterator_create();
    if (!it) return NULL;

    it->block = hive->head;
    it->index = 0;

    return it;
}

/* 
  * 返回Hive的最后一个元素的位置 
  * 并不是取最后一个有效元素
*/
HiveIterator *iterator_end(Hive *hive)
{
    if (!hive) return NULL;

    HiveIterator *it = iterator_create();
    if (!it) return NULL;

    it->block = hive->tail;
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
        /* 如果是空块，直接跳过 */
        if (it->block->l1_bitset == 0) // L1位图全为0，表示块为空
        {
            it->block = it->block->next;
            if (!it->block) return; // 末尾
            it->index = 0; // 重置索引
            continue;
        }

        /* 如果索引超出当前块，移到下一个块 */
        if (it->index >= BLOCK_SIZE)
        {
            it->block = it->block->next;
            if (!it->block) return; // 末尾
            it->index = 0; // 重置索引
            continue;
        }

        /* 检查当前位置 */
        if ((it->block->l1_bitset & (1 << it->index))) // 被占用，结束
            return;

        uint8_t skip = (uint8_t)it->block->data[it->index];
        it->index += skip;
    }
}

void mark_occupied(Block *block, size_t index)
{
    if (!block || index >= BLOCK_SIZE) return;

    block->l1_bitset |= (1 << index); // 标记槽位被占用
    if (block->l1_bitset == 0xFF) // 全满的情况
        block->l2_bitset = 0;
}

void mark_free(Block *block, size_t index)
{
    if (!block || index >= BLOCK_SIZE) return;
    block->l1_bitset &= ~(1 << index); // 标记槽位未被占用
    block->l2_bitset = 1; // 标记块未满
}

void block_reset(Block *block)
{
    if (!block) return;

    /* 初始化 `skipfield` */
    memset(block->data, 0, BLOCK_SIZE);
    block->data[0]= block->data[BLOCK_SIZE - 1] = BLOCK_SIZE;

    block->l1_bitset = 0; // 第一层位图: 全部置为0，表示所有槽位都未被占用
    block->l2_bitset = 1; // 第二层位图: 标记块未满

    block->next = NULL;
    block->prev = NULL;
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

void destroy_hive(Hive *hive)
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
    uint8_t total_len = (uint8_t)it->block->data[i]; // 当前空洞长度
    
    if (total_len == 0) return;

    /*
    * 由于 hive_insert 使用 __builtin_ctz 从头部开始找空洞，
    * 插入位置一定是空洞的头部，左侧不可能有残留空洞。
    * 因此只需将原空洞拆分为“插入点”和“右侧残留空洞”，
    * 并更新右侧空洞的头尾标记。
    */
    size_t tail = i + total_len - 1;

    // 算出右侧残留空洞的长度
    size_t right_len = tail - i; // i 之后剩余的空洞长度

    /* 只需要更新右侧的空洞长度 */
    if (right_len > 0)
    {
        it->block->data[i + 1] = (char)right_len;
        it->block->data[tail] = (char)right_len;
    }
}

HiveIterator *hive_insert(Hive *hive, const HiveIterator *it, const char c)
{
    if (!hive || !it || !it->block) return NULL;

    HiveIterator *curr = iterator_create();
    if (!curr) return NULL;

    curr->block = it->block;
    curr->index = it->index;

    if (!(it->block->l2_bitset & 1)) // 如果当前块已满，插入新块
    {
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
        curr->block->data[1] = curr->block->data[BLOCK_SIZE - 1] = (char)(BLOCK_SIZE - 1);

        mark_occupied(curr->block, curr->index);

        return curr;
    }

    /* 处理空洞情况 */
    uint8_t first_empty = __builtin_ctz(~(it->block->l1_bitset)); // 找到第一个空闲的槽位
    curr->index = first_empty;

    /* 插入 */
    mark_occupied(curr->block, curr->index);
    insert_update_skipfield(curr);
    curr->block->data[curr->index] = c;

    return curr;
}

static void erase_update_skipfield(HiveIterator *it)
{
    if (!it || !it->block) return;

    size_t left_skip = 0;
    size_t right_skip = 0;

    /* 
      * 由于迭代器从不落在空洞中间，因此 index-1/index+1 如果是空洞，
      * 它一定是指向空洞头尾，因此只需读取更新相邻的空洞长度即可
    */

    if (it->index > 0 &&
        !(it->block->l1_bitset & (1 << (it->index - 1)))) // 判断左边是否有空洞
        left_skip = (size_t)it->block->data[it->index - 1];
    
    if (it->index < BLOCK_SIZE - 1 &&
        !(it->block->l1_bitset & (1 << (it->index + 1)))) // 判断右边是否有空洞
        right_skip = (size_t)it->block->data[it->index + 1];

    /* 空洞头尾索引 */
    size_t head = it->index - left_skip;
    size_t tail = it->index + right_skip;

    uint8_t new_skip = (uint8_t)(left_skip + right_skip + 1);
    it->block->data[head] = (char)new_skip;
    it->block->data[tail] = (char)new_skip;
}

void hive_erase(Hive *hive, HiveIterator *it)
{
    if (!hive || !it || !it->block) return;

    if (!(it->block->l1_bitset & 1 << it->index)) return; // 已经是空槽，不需要删除

    mark_free(it->block, it->index);

    erase_update_skipfield(it);

    if (it->block->l1_bitset == 0) // 如果当前块已空，回收块
        recycle_block(hive, it->block);

    /* 迭代器失效 */
    it->block = NULL;
}

void hive_traverse(const HiveIterator *it)
{
    if (!it || !it->block) return;

    HiveIterator it_copy = {it->block, it->index}; // 复制当前迭代器

    while (it_copy.block)
    {
        printf("%c", it_copy.block->data[it_copy.index]);
        iterator_next(&it_copy);
    }
}

void clear_hive(Hive *hive)
{
    if (!hive) return;

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

int main(void) {
    Hive *hive = create_hive();
    if (!hive) return 1; // 创建 Hive 失败

    HiveIterator *it = iterator_begin(hive);

    /* 第一个块插入 */
    hive_insert(hive, it, 'H');
    HiveIterator *test_erase1 = hive_insert(hive, it, 'e');
    HiveIterator *test_erase2 = hive_insert(hive, it, 'l');
    HiveIterator *test_erase3 = hive_insert(hive, it, 'l');
    hive_insert(hive, it, 'o');
    hive_insert(hive, it, ' ');
    hive_insert(hive, it, 'W');
    hive_insert(hive, it, 'o');

    /* 预期输出: Hello Wo */
    hive_traverse(it);
    printf("\n");

    /* 测试空块影响，这已经改变了默认实现，只用于测试 */
    HiveIterator *test_empty = hive_insert(hive, it, ' ');
    block_reset(test_empty->block); // 重置空块数据

    /* 测试空块回收 */
    HiveIterator *test_erase4 = hive_insert(hive, it, 'r');
    hive_erase(hive, test_erase4);

    /* 测试空块复用 */
    hive_insert(hive, it, 'l');
    hive_insert(hive, it, 'd');
    hive_insert(hive, it, '!');

    /* 测试块内空洞影响 */
    hive_erase(hive, test_erase1);
    hive_erase(hive, test_erase2);
    hive_erase(hive, test_erase3);

    /* 预期输出: Ho Wo!dl */
    hive_traverse(it);
    printf("\n");

    clear_hive(hive); // 清空 Hive

    return 0;
}
