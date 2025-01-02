//
// Created by lenovo on 2024/12/18.
//

#ifndef DATABASE_H
#define DATABASE_H
#include "Memoryriver.h";
#include "c.h"
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <functional>
#include <cstring>

constexpr size_t child_cnt = 16;

// TODO: Add more static assertions to type Key and Val
template <class Key, class Val, auto KeyCmp = std::less<Key>{}, auto KeyEq = std::equal_to<Key>{}, int header_id = 0>
class Database {
static_assert(std::is_convertible_v<decltype(KeyCmp), std::function<bool(Key, Key)>>
              && std::is_convertible_v<decltype(KeyEq), std::function<bool(Key, Key)>>);

public:
  Database() = delete;
  explicit Database(std::string filename);
  explicit Database(Memoryriver &bf_);

  void insert(Key key, Val val);
  void insertLow(Key key, pos_t pos);
  Val get(Key key);
  bool exist(Key key);
  std::pair<pos_t, Val> getLow(Key key);
  void modify(Key key, Val val);
  void erase(Key key);
  Memoryriver &m;

	std::vector<Val> getAll();
  void printKeys();

private:

  struct BHeader {
    int depth;
    pos_t root;
    int timestamp;
  } header;

  struct Node {
    Key key[child_cnt - 1];
    size_t size;
    pos_t fa, chd[child_cnt];
  };

  std::unique_ptr<Memoryriver> m_ptr;
};

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
std::vector<Val> Database<Key, Val, KeyCmp, KeyEq, header_id>::getAll() {
  std::vector<Val> ret;
  std::function<void(const Node &, int)> dfs = [&](const Node &n, int dep) {
    if (dep == header.depth) {
      for (int i = 0; i < n.size; ++i) {
        if (n.chd[i] == nullpos)
          continue;
        Val tmp{}; m.getT(n.chd[i], tmp);
        ret.push_back(tmp);
      }
      return;
    } else
      for (int i = 0; i <= n.size; ++i) {
        Node tmp{};
        m.getT(n.chd[i], tmp);
        dfs(tmp, dep + 1);
      }
  };
  Node tmp{};
  m.getT(header.root, tmp);
  dfs(tmp, 0);
  return ret;
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
Database<Key, Val, KeyCmp, KeyEq, header_id>::Database(Memoryriver &m_) : m(m_) {
  m.getHeaderT(header_id, header);
  if (header.root == 0) {
    Node node{{}, 0, nullpos, {}};
    header.root = m.allocT(node);
    header.depth = 0;
    m.putT(m.getHeaderPos(header_id), header);
  }
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
void Database<Key, Val, KeyCmp, KeyEq, header_id>::insert(Key key, Val val) {
    pos_t now = header.root;
  int depth = 0;
  Node node{};
  while (depth < header.depth) {
    // WITH_ETR(now, node, );
    m.getT(now, node);
    size_t k{};
    for (k = 0; k + 1 < node.size; ++k)
      if (!KeyCmp(node.key[k], key))
        break;
    now = node.chd[k];
    ++depth;
  }
  // WITH_ETR(now, node, );
  m.getT(now, node);
  size_t k;
  for (k = 0; k < node.size; ++k) {
    if (!KeyCmp(node.key[k], key))
      break;
  }
  if (k < node.size && KeyEq(node.key[k], key)) {  // exist
    if (node.chd[k] == nullpos) {
      node.chd[k] = m.allocT(val);
      m.putT(now, node);
    } else
      throw Error("Key already exists");
  } else if (node.size + 1 < child_cnt) {
    for (int i = node.size; i > k; --i) {
      memcpy(&node.key[i], &node.key[i - 1], sizeof(node.key[i]));
      memcpy(&node.chd[i], &node.chd[i - 1], sizeof(node.chd[i]));
    }
    // strcpy(node.key[k], key.c_str());
    node.key[k] = key;
    ++header.timestamp;
    m.putT(m.getHeaderPos(header_id), header);
    node.chd[k] = m.allocT(val);
    ++node.size;
    // WITH_ETW(now, node, );
    m.putT(now, node);
  } else {
    assert(node.size == child_cnt - 1);
    ++header.timestamp;
    m.putT(m.getHeaderPos(header_id), header);
    pos_t dhd_pos = m.allocT(val);

    Key ktmp[child_cnt + 1]{};
    pos_t kchd[child_cnt + 1]{};
    memcpy(&ktmp[0], &node.key[0], sizeof(Key) * k);
    memcpy(&kchd[0], &node.chd[0], sizeof(kchd[0]) * k);
    ktmp[k] = key;
    memcpy(&kchd[k], &dhd_pos, sizeof(kchd[0]));
    memcpy(&ktmp[k + 1], &node.key[k], sizeof(Key) * (node.size - k));
    memcpy(&kchd[k + 1], &node.chd[k], sizeof(kchd[0]) * (node.size - k));

    size_t mid = child_cnt / 2;
    Node rnode{};
    node.size = mid;
    rnode.size = child_cnt - mid;
    rnode.fa = node.fa;
    memcpy(&node.key[0], &ktmp[0], sizeof(Key) * mid);
    memcpy(&rnode.key[0], &ktmp[mid], sizeof(Key) * (child_cnt - mid));
    memcpy(&node.chd[0], &kchd[0], sizeof(kchd[0]) * mid);
    memcpy(&rnode.chd[0], &kchd[mid], sizeof(kchd[0]) * (child_cnt - mid));
    pos_t rnode_pos = m.allocT(rnode);
    pos_t node_pos = now;
    m.putT(now,node);
    // WITH_ETW(now, node, );
    m.putT(now, node);
    // std::string old_key = key;
    key = ktmp[mid - 1];

    for (; depth > 0; --depth) {
      Node fa{};
      m.getT(node.fa,fa);
      //WITH_ETR(node.fa, fa, );
      if (fa.size + 1 <= child_cnt) {
        pos_t k{};
        for (k = 0; k + 1 < fa.size; ++k)
          if (!KeyCmp(fa.key[k], key))
            break;
        for (int i = fa.size; i > k; --i) {
          if (i != fa.size)
            memcpy(&fa.key[i], &fa.key[i - 1], sizeof(Key));
          memcpy(&fa.chd[i], &fa.chd[i - 1], sizeof(fa.chd[i]));
        }
        // strcpy(fa.key[k], key.c_str());
        fa.key[k] = key;
        memcpy(&fa.chd[k + 1], &rnode_pos, sizeof(rnode_pos));

        // WITH_ETW(node.fa, fa, fa.size++);
        ++fa.size;
        m.putT(node.fa, fa);
        break;
      } else {
        assert(fa.size == child_cnt);
        pos_t k{};
        for (k = 0; k + 1 < fa.size; ++k)
          if (!KeyCmp(fa.key[k], key))
            break;
        memcpy(&ktmp[0], &fa.key[0], sizeof(Key) * k);
        memcpy(&kchd[0], &fa.chd[0], sizeof(fa.chd[0]) * (k + 1));
        // if (k + 1 < child_cnt)
          // strcpy(ktmp[k], key.c_str());
        ktmp[k] = key;
        memcpy(&kchd[k + 1], &rnode_pos, sizeof(rnode_pos));
        assert(kchd[k] == node_pos);
        if (k + 1 < child_cnt) {
          memcpy(&kchd[k + 2], &fa.chd[k + 1], sizeof(fa.chd[0]) * ((int)child_cnt - k - 1));
          memcpy(&ktmp[k + 1], &fa.key[k], sizeof(Key) * ((int)child_cnt - 1 - k));
        }

        mid = (child_cnt + 1) / 2;
        key = ktmp[mid - 1];
        pos_t old_node_pos = node_pos, old_rnode_pos = rnode_pos;
        node_pos = node.fa;
        node = fa;
        node.size = mid;
        memcpy(&node.key[0], &ktmp[0], sizeof(Key) * (node.size - 1));
        memcpy(&node.chd[0], &kchd[0], sizeof(kchd[0]) * node.size);
        rnode.fa = node.fa;
        rnode.size = child_cnt + 1 - mid;
        memcpy(&rnode.key[0], &ktmp[mid], sizeof(Key) * (rnode.size - 1));
        memcpy(&rnode.chd[0], &kchd[mid], sizeof(kchd[0]) * rnode.size);
        rnode_pos = m.allocT(rnode);
        m.putT(node_pos,node);
        //WITH_ETW(node_pos, node, );
        for (int i = 0; i < rnode.size; ++i) {
          // WITH_T(rnode.chd[i], Node, ntmp, ntmp.fa = rnode_pos);
          Node ntmp{};
          m.getT(rnode.chd[i], ntmp);
          ntmp.fa = rnode_pos;
          m.putT(rnode.chd[i], ntmp);
        }
        // WITH_T(old_node_pos, Node, ntmp, ntmp.fa = k < mid ? node_pos : rnode_pos);
        Node ntmp{};
        m.getT(old_node_pos, ntmp);
        ntmp.fa = k < mid ? node_pos : rnode_pos;
        m.putT(old_node_pos, ntmp);
        // WITH_T(old_rnode_pos, Node, ntmp, ntmp.fa = (k + 1) < mid ? node_pos : rnode_pos);
        m.getT(old_rnode_pos, ntmp);
        ntmp.fa = (k + 1) < mid ? node_pos : rnode_pos;
        m.putT(old_rnode_pos, ntmp);
      }
    }
    if (depth == 0) {
      Node newroot{{}, 2, nullpos, {node_pos, rnode_pos}};
      // strcpy(newroot.key[0], key.c_str());
      newroot.key[0] = key;
      pos_t newroot_pos = m.allocT(newroot);
      // WITH_ET(node_pos, node, node.fa = newroot_pos);
      node.fa = newroot_pos;
      m.putT(node_pos, node);
      // WITH_ET(rnode_pos, rnode, rnode.fa = newroot_pos);
      rnode.fa = newroot_pos;
      m.putT(rnode_pos, rnode);
      ++header.depth;
      header.root = newroot_pos;
      m.putT(m.getHeaderPos(header_id), header);
    }
  }
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
Val Database<Key, Val, KeyCmp, KeyEq, header_id>::get(Key key) {
  int depth = 0;
  pos_t pos = header.root;
  Node node{};
  m.getT(pos,node);
  //WITH_ETR(pos, node,)//;
  pos_t k{};
  while (depth <= header.depth) {
    m.getT(pos,node);
    //WITH_ETR(pos, node,);
    for (k = 0; k + 1 < node.size; ++k)
      if (!KeyCmp(node.key[k], key))
        break;
    pos = node.chd[k];
    ++depth;
  }

  if (k < child_cnt - 1 && KeyEq(node.key[k], key)) {
    if (pos == nullpos)
      throw Error("");
    Val val;
    m.getT(pos, val);
    return val;
  }
  throw Error("");
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
bool Database<Key, Val, KeyCmp, KeyEq, header_id>::exist(Key key) {
  int depth = 0;
  pos_t pos = header.root;
  Node node{};
  m.getT(pos,node);
  //WITH_ETR(pos, node,);
  pos_t k{};
  while (depth <= header.depth) {
    m.getT(pos,node);
    //WITH_ETR(pos, node,);
    for (k = 0; k + 1 < node.size; ++k)
      if (!KeyCmp(node.key[k], key))
        break;
    pos = node.chd[k];
    ++depth;
  }
  if (k < child_cnt - 1 && KeyEq(node.key[k], key))
    return node.chd[k] != nullpos;
  return false;
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
std::pair<pos_t, Val> Database<Key, Val, KeyCmp, KeyEq, header_id>::getLow(Key key) {
  int depth = 0;
  pos_t pos = header.root;
  Node node{};
  m.getT(pos,node);
  //WITH_ETR(pos, node,);
  pos_t k{};
  while (depth <= header.depth) {
    m.getT(pos,node);
    //WITH_ETR(pos, node,);
    for (k = 0; k + 1 < node.size; ++k)
      if (!KeyCmp(node.key[k], key))
        break;
    pos = node.chd[k];
    ++depth;
  }
  if (k < child_cnt - 1 && KeyEq(node.key[k], key)) {
    if (!(pos != nullpos)) throw Error("");;
    Val val;
    m.getT(pos, val);
    return {pos, val};
  }
  throw Error("getLow: not found");
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
void Database<Key, Val, KeyCmp, KeyEq, header_id>::modify(Key key, Val val) {
  int depth = 0;
  pos_t pos = header.root;
  Node node{};
  m.getT(pos,node);
  //WITH_ETR(pos, node,);
  pos_t k{};
  while (depth <= header.depth) {
    m.getT(pos,node);
    //WITH_ETR(pos, node,);
    for (k = 0; k + 1 < node.size; ++k)
      if (!KeyCmp(node.key[k], key))
        break;
    pos = node.chd[k];
    ++depth;
  }

  if (k < child_cnt - 1 && KeyEq(node.key[k], key)) {
    if (!(pos != nullpos)) throw Error("");;
    m.putT(pos, val);
  } else
    throw Error("");
}

template<class Key, class Val, auto KeyCmp, auto KeyEq, int header_id>
void Database<Key, Val, KeyCmp, KeyEq, header_id>::erase(Key key) {
  int depth = 0;
  pos_t pos = header.root, pre = nullpos;
  Node node{};
  m.getT(pos,node);
  //WITH_ETR(pos, node,);
  pos_t k{};
  while (depth <= header.depth) {
    m.getT(pos,node);
    //WITH_ETR(pos, node,);
    for (k = 0; k + 1 < node.size; ++k)
      if (!KeyCmp(node.key[k], key))
        break;
    pre = pos, pos = node.chd[k];
    ++depth;
  }
  if (k < child_cnt - 1 && KeyEq(node.key[k], key) && node.chd[k] != nullpos) {
    node.chd[k] = nullpos;
    m.putT(pre, node);
  } else
    throw Error("");
}
#endif //DATABASE_H
