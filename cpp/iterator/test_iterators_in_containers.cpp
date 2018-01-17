#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>
using namespace std;

/* print will print every element in collection */
template <class T> void print(const T &col) {
  for (auto &x : col) {
    std::cout << x << ' ';
  }
  std::cout << std::endl;
}

void test_vector_iterator() {
  vector<int> col = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::cout << "cap:" << col.capacity() << std::endl;
  print<vector<int>>(col);

  auto ite = col.begin() + 3;
  col.insert(ite, -1); // ite会失效,因为容量改变了,整体元素都移动了
  print<vector<int>>(col);

  // for (; ite != col.end(); ++ite) { // error,ite已经失效
  //   std::cout << *ite << ' ';
  // }
  // cout << endl;
}

void test_deque_iterator() {
  deque<int> col = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  print<deque<int>>(col);

  auto ite = col.begin();
  ite += 3;

  for (auto i = 0; i != 10000; ++i)
    col.insert(col.begin(), i); // ite已经失效,可能已经丢失了索引节点的有效性.
  for (auto i = 0; i != 10000; ++i)
    col.insert(col.end(), i); // ite已经失效,可能已经丢失了索引节点的有效性.

  print<deque<int>>(col);

  std::cout << *ite << std::endl; // no error!原来的ite还可以用来dereference

  for (; ite != col.end(); ++ite) // error!但不能用来迭代所有元素!
    std::cout << *ite << ' ';
  std::cout << std::endl;
}

void test_list_iterator() {
  list<int> col = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  print<list<int>>(col);

  auto ite = col.begin();
  ++ite;
  ++ite;
  ++ite;
  col.insert(ite, 4);

  print<list<int>>(col);
}

void test_map_iterator() {
  map<int, bool> col{{1, true}, {2, true}, {3, true}, {4, false}};
  auto ite = col.begin();
  std::cout << ite->first << std::endl;
}

void test_set_iterator() {
  set<int> col = {0, 1, 2, 3, 4};
  auto ite = col.begin();
  std::cout << *ite << std::endl;
}

void test_priorityqueue() {
  // error! set没有random access iterator
  // priority_queue<int, set<int>> col;

  priority_queue<int, vector<int>> col;
  col.push(1);
  col.push(1);
}
int main(int argc, char **argv) {
  // test_vector_iterator();
  // test_deque_iterator();
  // test_list_iterator();
  // test_map_iterator();
  // test_set_iterator();
  test_priorityqueue();
  return 0;
}