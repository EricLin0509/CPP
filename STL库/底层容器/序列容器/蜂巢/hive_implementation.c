#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

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

void block_reset(Block *block)
{
    if (!block) return;

    block->occupied = 0;
    block->next = NULL;
    block->prev = NULL;
}

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


int find_free_slot(Block *block)
{
    if (!block) return -1;
    if (block->occupied == 0xFF) return -1; // 如果所有位都被占用，就返回-1

#ifdef __has_builtin
    #if __has_builtin(__builtin_ctz)
        return __builtin_ctz((unsigned int)(~block->occupied & 0xFF));// 返回第一个空闲位
    #endif
#endif
        return ctz(~block->occupied & 0xFF);
}

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
        int slot = find_free_slot(hive->tail); // 找到当前块的第一个空闲位
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
        
        Block *curr = block;
        block = block->next;

        if (curr->occupied == 0) // 如果当前块所有位都空闲，就回收
            recycle_block(hive, curr);
    }
}

char *to_string(Hive *hive)
{
    if (!hive || hive->head == NULL) return NULL;

    size_t total_size = 0;
    Block *block = hive->head;

    while (block) // 快速遍历——只遍历块，不遍历每个字符
    {
        total_size += BLOCK_SIZE;
        block = block->next;
    }

    char *str = calloc(1, (total_size + 1)); // 最多需要 `total_size + 1` 个字符
    
    block = hive->head;
    size_t index = 0;
    while (block)
    {
        for (int i = 0; i < BLOCK_SIZE; i++) // 遍历当前块的每个字符
        {
            if (block->occupied & (1 << i)) // 如果块数据位被占用，就复制
                str[index++] = block->data[i];
        }
        block = block->next;
    }

    str[index] = '\0'; // 添加字符串结束符
    return str;
}

int main() {
    Hive hive = {NULL, NULL, NULL, 0, 0};
    append_char(&hive, "aaaaaaaa");
    append_char(&hive, "bbbbbbbb");
    append_char(&hive, "cccccccc");
    delete_char(&hive, 'b');
    delete_char(&hive, 'c');
    delete_char(&hive, 'a');
    append_char(&hive, "dddddddd");
    append_char(&hive, "eeeeeeee");
    append_char(&hive, "ffffffff");
    append_char(&hive, "\nHello World!");

    traverse_char(&hive);
    printf("\n");

    clear_hive(&hive); // 清空Hive

    return 0;
}
