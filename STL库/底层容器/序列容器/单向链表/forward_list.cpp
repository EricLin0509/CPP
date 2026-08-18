#include <iostream>
#include <forward_list>

bool cmp(const int& a, const int& b)
{
    return a > b;
}

void printList(const std::forward_list<int>& list)
{
    for (int num : list)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";
}

int main() {
    std::forward_list<int> list1 = {1, 5, 3, 2, 4};
    std::forward_list<int> list2 = {7, 8, 6, 9, 10};
    list1.sort(cmp);
    list2.sort(cmp);
    list1.merge(list2, cmp);

    printList(list1);

    std::forward_list<int> list3 = {1, 3, 2, 5, 4, 6};
    list3.sort();
    
    printList(list3);

    std::forward_list<int> list4 = {1, 2, 3, 4, 5};
    list4.reverse();
    
    printList(list4);

    std::forward_list<int> list5 = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5};
    list5.unique();

    printList(list5);

    std::forward_list<int> list6 = {1, 2, 3, 4, 5, 6};
    list6.remove(3);

    printList(list6);

    std::forward_list<int> list7 = {1, 2, 3, 4, 5, 6};
    list7.remove_if([](int num){ return (num & 1) == 0; }); // 移除所有偶数

    printList(list7);

    return 0;
}