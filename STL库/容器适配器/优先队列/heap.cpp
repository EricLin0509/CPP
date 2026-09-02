#include <iostream>
#include <assert.h>
#include <vector>

/* int 版本最小堆实现 */
/* 左子树查找: 2a + 1 */
/* 右子树查找: 2a + 2 */

/* push 操作 */
static void swap(int &a, int &b)
{
    if (a == b) return;

    a ^= b;
    b ^= a;
    a ^= b;
}

static int find_parent_index(int index)
{
    if (index == 0) return 0; // 0没有父节点

    int parent = (index - 1) / 2; // 这里不需要考虑左子树和右子树，因为整数除法会自动向下取整
    return parent;
}

void push(std::vector<int> &heap, int value)
{
    if (heap.size() == 0)
    {
        heap.push_back(value);
        return;
    } 

    heap.push_back(value);
    int current = heap.size() - 1;
    int parent = find_parent_index(current);
    while (current > 0 && heap[current] < heap[parent])
    {
        swap(heap[current], heap[parent]);
        current = parent;
        parent = find_parent_index(current);
    }
}

/* pop 操作 */
static int find_min_child_index(std::vector<int> &heap, int index)
{
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left >= heap.size()) return -1; // 没有左子树，意味着没有子节点
    if (right >= heap.size()) return left; // 没有右子树，意味着只有左子树

    /* 大小比较 */
    if (heap[left] < heap[right]) return left;
    return right;
}

static void heapify(std::vector<int> &heap) // 从0开始，向下堆化
{
    int current = 0;
    int min_child_index = find_min_child_index(heap, current);
    while (min_child_index != -1 && heap[current] > heap[min_child_index])
    {
        swap(heap[current], heap[min_child_index]);
        current = min_child_index;
        min_child_index = find_min_child_index(heap, current);
    }
}

void pop(std::vector<int> &heap)
{
    if (heap.size() == 0) return;

    swap(heap[0], heap[heap.size() - 1]);
    heap.pop_back();
    heapify(heap);
}

void build_test_heap(std::vector<int> &heap)
{
    assert(heap.size() == 0);
    for (int i = 1; i <= 5; i++)
    {
        if (i == 1) continue; // 跳过1
        heap.push_back(i);
    }
}

void print_heap(std::vector<int> &heap)
{
    for(int num : heap)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";
}

int main() {
    std::vector<int> heap;
    build_test_heap(heap);

    push(heap, 1);
    assert(heap == std::vector<int>({1, 2, 4, 5, 3}));
    print_heap(heap);

    pop(heap);
    assert(heap == std::vector<int>({2, 3, 4, 5}));
    print_heap(heap);

    std::vector<int> heap2 = {1, 3, 2, 5, 4};
    push(heap2, 6);
    assert(heap2 == std::vector<int>({1, 3, 2, 5, 4, 6}));
    print_heap(heap2);

    pop(heap2);
    assert(heap2 == std::vector<int>({2, 3, 6, 5, 4}));
    print_heap(heap2);
    return 0;
}
