#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <cstdint>
#include <fstream>
typedef int64_t pos_t;

constexpr pos_t nullpos = -1;
constexpr size_t header_size = 56;


struct MemoryriverHeader {
  char data[header_size];
  pos_t nxt;
};

static_assert(sizeof(MemoryriverHeader) == 64);

template <class T>
inline std::pair<T, T> cross(T l1, T r1, T l2, T r2) {
  return { std::max(l1, l2), std::min(r1, r2) };
}

class Memoryriver {
public:
  Memoryriver() = delete;

  ~Memoryriver();

  explicit Memoryriver(std::string filename, size_t cache_size_ = 1024 * 1024, pos_t cache_start = 0);

  template<class T>
  void getT(pos_t pos, T &x);

  void get(pos_t pos, char *x, ssize_t size);

  template<class T>
  pos_t allocT(const T &x);

  pos_t alloc(const char *x, ssize_t size);

  pos_t allocEmpty(ssize_t size);

  void memcpy(pos_t dest, pos_t src, ssize_t size);

  template<class T>
  void putT(pos_t pos, const T &x);

  void put(pos_t pos, const char *x, ssize_t size);

  void sync();

  void erase(pos_t pos, ssize_t size);

  template<class T>
  void getHeaderT(int id, T &x);

  void getHeader(int id, char *x, ssize_t sz);

  pos_t getHeaderPos(int id);
  pos_t end;

private:
  const std::string name;
  std::fstream f;
  const size_t cache_size;
  char *ca;
  pos_t ca_start;
};


template<class T>
void Memoryriver::getT(pos_t pos, T &x) {
  get(pos, reinterpret_cast<char *>(&x), sizeof(T));
}

template<class T>
pos_t Memoryriver::allocT(const T &x) {
  return alloc(reinterpret_cast<const char *>(&x), sizeof(T));
}

template<class T>
void Memoryriver::putT(pos_t pos, const T &x) {
  put(pos, reinterpret_cast<const char *>(&x), sizeof(T));
}

template<class T>
void Memoryriver::getHeaderT(int id, T &x) {
  getHeader(id, reinterpret_cast<char *>(&x), sizeof(T));
}

#endif //BPT_MEMORYRIVER_HPP

