
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>

using namespace std;

std::atomic<int> cnt = ATOMIC_VAR_INIT(0);

template <typename T>
class lock_free_stack {
private:
/* Data structure the represent a node */
  std::mutex m_mutex;
  struct node {
    T data;
    node* next;

    node (T const& _data) {
      data = _data;
      next = NULL;
    }
  };
/* Atomic pointer that represents the top of a stack */
  //atomic<node *> head;
  node *head;
public:
  void push(T const& _data) {
    node * new_node = new node(_data);
    new_node->next = head.load();
    while(!head.compare_exchange_weak(new_node->next, new_node));
  }

  ~lock_free_stack()
  {
    cout << "COUNT  " << cnt << endl;
  }
  void push_lock(T const& _data) {
    node *new_node = new node(_data);
    m_mutex.lock();
    new_node->next = head;
    head = new_node;
    m_mutex.unlock();
  }

  T& head_data() {
    //cout << "LOCK " << head.is_lock_free() << endl;
    return head->data;
  }
};

#define ENTRIES 10000000

void add(lock_free_stack<int> *stack)
{
   for (size_t i = 0; i < ENTRIES; i++) {
     stack->push_lock(i);
     atomic_fetch_add(&cnt, 1);
   }
}

int main(int argc, char const *argv[]) {
  /* code */
  std::chrono::time_point<std::chrono::system_clock> start, end;
  cout<<"Start"<<endl;
  start = std::chrono::system_clock::now();
  lock_free_stack<int> obj;

  std::thread first (add, &obj);     // spawn new thread that calls foo()
  std::thread second (add, &obj);
  std::thread third (add, &obj);
  std::thread forth (add, &obj);
  std::thread fifth (add, &obj);
  std::thread sixth (add, &obj);
  std::thread seventh (add, &obj);
  std::thread eighth (add, &obj);
  first.join();
  second.join();
  third.join();
  forth.join();
  fifth.join();
  sixth.join();
  seventh.join();
  eighth.join();
  cout << obj.head_data() << endl;

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "  elapsed time: " << elapsed_seconds.count() << "s\n";
  return 0;
}
