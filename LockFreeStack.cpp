
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

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
  atomic<node *> head;
  //node *head;
public:
  void push(T const& _data) {
    node * new_node = new node(_data);
    new_node->next = head.load();
    while(!head.compare_exchange_weak(new_node->next, new_node,
                                      std::memory_order_release,
                                      std::memory_order_relaxed));
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

  T pop()
  {
    node *old_head = head.load();
    while(old_head &&
          !head.compare_exchange_weak(old_head, old_head->next));
    return old_head ? old_head->data : T();
  }

  T& head_data() {
    //cout << "LOCK " << head.is_lock_free() << endl;
    return head.load()->data;
  }
};

#define ENTRIES 10000000
#define THREADS 16

void add(lock_free_stack<int> *stack)
{
   for (size_t i = 0; i < ENTRIES; i++) {
     stack->push(i);
     atomic_fetch_add(&cnt, 1);
   }

   for (size_t i = 0; i < cnt; i++) {
     stack->pop();
   }
}

int main(int argc, char const *argv[]) {
  /* code */
  std::chrono::time_point<std::chrono::system_clock> start, end;
  cout<<"Start"<<endl;
  vector<thread> workers(THREADS);
  start = std::chrono::system_clock::now();
  lock_free_stack<int> obj;

  for (size_t i = 0; i < THREADS; i++) {
    workers.at(i) = std::thread(add, &obj);
  }

  for (size_t i = 0; i < THREADS; i++) {
    workers.at(i).join();
  }

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "  elapsed time: " << elapsed_seconds.count() << "s\n";
  return 0;
}
