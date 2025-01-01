#include "Memoryriver.h"

Memoryriver::~Memoryriver() {
  sync();
  delete[] ca;
}

Memoryriver::Memoryriver(std::string name_, size_t cache_size_, pos_t ca_start_) : name(name_),
                                    f(name, std::fstream::in | std::fstream::out | std::fstream::binary),
                                    cache_size(cache_size_), ca(new char[cache_size]), ca_start(ca_start_) {

  if (!f.is_open()) {
    f.clear();
    f.open(name, std::fstream::out);
    f.close();
    f.open(name, std::fstream::in | std::fstream::out | std::fstream::binary);
  }
  assert(f.is_open());
  f.seekp(0, std::fstream::end);
  if (!f.tellp()) {
    std::unique_ptr<MemoryriverHeader> hd(new MemoryriverHeader);
    for (char &i: hd->data)
      i = 0;
    hd->nxt = nullpos;
    alloc(reinterpret_cast<const char*>(hd.get()), sizeof(MemoryriverHeader));
  }
  assert(f.is_open());
  assert(!f.fail());
  end = f.tellp();

  if (ca_start < end) {
    f.seekg(ca_start);
    f.read(ca, std::min(end - ca_start, (pos_t)cache_size));
  }
  assert(!f.fail());
}

void Memoryriver::get(pos_t pos, char *x, ssize_t size) {
  if (ca_start <= pos && pos + size <= ca_start + cache_size) {
    ::memcpy(x, ca + pos - ca_start, size);
    return;
  }
  assert(!f.fail());
  f.seekg(pos);
  f.read(x, size);
  assert(!f.fail());
  auto [l, r] = cross(pos, pos + (pos_t)size, ca_start, ca_start + (pos_t)cache_size);
  if (l < r)
    ::memcpy(x + l - pos, ca + l - ca_start, r - l);
}

pos_t Memoryriver::alloc(const char *x, ssize_t size) {
  assert(!f.fail());
  f.seekp(0, std::fstream::end);
  pos_t ret = f.tellp();
  f.write(x, size);
  end = f.tellp();
  assert(!f.fail());
  auto [l, r] = cross(ret, ret + (pos_t)size, ca_start, ca_start + (pos_t)cache_size);
  if (l < r)
    ::memcpy(ca + l - ca_start, x + l - ret, r - l);
  return ret;
}

pos_t Memoryriver::allocEmpty(ssize_t size) {
  assert(!f.fail());
  f.seekp(0, std::fstream::end);
  pos_t ret = f.tellp();
  for (int i = 0; i < size; ++i)
    f.put('\0');
  end = f.tellp();
  assert(end - ret == size);
  assert(!f.fail());

  auto [l, r] = cross(ret, ret + (pos_t)size, ca_start, ca_start + (pos_t)cache_size);
  if (l < r)
    ::memset(ca + l - ca_start, 0x00, r - l);
  return ret;
}

void Memoryriver::memcpy(pos_t dest, pos_t src, ssize_t size) {
  assert(!f.fail());
  static constexpr ssize_t bu_size = 4096;
  static char bu[bu_size];
  ssize_t i;
  for (i = 0; i + bu_size <= size; i += bu_size) {
    get(src + i, bu, bu_size);
    put(dest + i, bu, bu_size);
  }
  assert(i <= size);
  if (i < size) {
    get(src + i, bu, size - i);
    put(dest + i, bu, size - i);
  }
  assert(!f.fail());
}

void Memoryriver::put(pos_t pos, const char *x, ssize_t size) {
  if (ca_start <= pos && pos + size <= ca_start + cache_size) {
    ::memcpy(ca + pos - ca_start, x, size);
    return;
  }
  assert(!f.fail());
  f.seekp(pos);
  f.write(x, size);
  assert(!f.fail());
  auto [l, r] = cross(pos, pos + (pos_t)size, ca_start, ca_start + (pos_t)cache_size);
  if (l < r)
    ::memcpy(ca + l - ca_start, x + l - pos, r - l);
}

void Memoryriver::sync() {
  if (ca_start < end) {
    f.seekp(ca_start);
    f.write(ca, std::min((pos_t)cache_size, end - ca_start));
  }
  f.sync();
}


void Memoryriver::getHeader(int id, char *x, ssize_t sz) {
  assert(0 <= sz && sz <= header_size);
  get(getHeaderPos(id), x, sz);
}

pos_t Memoryriver::getHeaderPos(int id) {
  assert(0 <= id);
  pos_t now = 0;
  int cnt = 0;
  std::unique_ptr<MemoryriverHeader> hd(new MemoryriverHeader);
  while ((cnt++) < id) {
    getT(now, *hd);
    now = hd->nxt;
    assert(now != nullpos);
  }
  return now;
}
