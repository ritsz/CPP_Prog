
#include <iostream>
#include <atomic>

using namespace std;

template <typename T>
class lock_free_stack {
private:
/* Data structure the represent a node */
  struct node {
    T data;
    node* next;

    node (T const& _data) {
      data = _data;
    }
  };
/* Atomic pointer that represents the top of a stack */
  atomic<node *> head;
};

int main(int argc, char const *argv[]) {
  /* code */
  return 0;
}
