#include <iostream>
#include <vector>

using std::vector;

template <typename Type>

class Stack {
    private:
        vector<Type> stack;
    public:
        bool isEmpty() const
        {
            return this->stack.empty();
        }

        int size() const
        {
            return this->stack.size();
        }

        void push(Type item)
        {
            this->stack.push_back(item);
        }

        Type pop()
        {
            if (this->isEmpty())
            {
                std::cerr << "Stack is empty!" << "\n";
                return Type();
            }

            Type poppedItem = this->stack.back();
            this->stack.erase(this->stack.end());
            return poppedItem;
        }

        void displayStack() const
        {   
            if (this->isEmpty())
            {
                std::cout << "Stack is empty!" << "\n";
                return;
            }

            for (const auto& item : this->stack)
            {
                std::cout << item << " ";
            }
            std::cout << "\n";
        }
};

int main() {
    Stack<int> intStack;

    for (int i = 1; i <= 5; ++i)
    {
        intStack.push(i);
    }

    intStack.displayStack();
    
    int item = intStack.pop();
    intStack.displayStack();

    std::cout << "Popped item: " << item << "\n";

    return 0;
}