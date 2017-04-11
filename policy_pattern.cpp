#include <cstdlib>
#include <iostream>

using num = int;
typedef int number;

template <typename T>
struct NewAlloc {
    T * Create()
    {
        return new T;
    }
};

template <typename T>
struct PNewAlloc {
    T * Create()
    {
        void *v = malloc(sizeof(T));
        return new(v)T();
    }
};

template <class Policy>
class Host
{
    Policy p;
    public:
        void CreatePolicy()
        {
            auto ptr = p.Create();
        }
};

int main()
{
    Host< NewAlloc <number> > Obj;
    Obj.CreatePolicy();
}
