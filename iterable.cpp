#include <iostream>
#include <vector>

template <typename T>
class Container
{
    T arr[10];
    size_t index;
public:
    T * begin() { return &arr[0]; }
    T* end()  { return (int *)(&arr[0] + index); }
    Container(T elem)
    {
        arr[0] = elem;
        index = 1;
    }
    void push_back(T elem)
    {
        arr[index++] = elem;
    }
};


int main()
{
    Container<int> v(10);
    v.push_back(5);
    v.push_back(1);
    for(auto a : v)
        std::cout << a << std::endl;
}
