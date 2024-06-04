//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_hash_table.cpp
//
// Identification: src/container/hash/extendible_hash_table.cpp
//
// Copyright (c) 2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdlib>
#include <functional>
#include <list>
#include <utility>

#include "container/hash/extendible_hash_table.h"
#include "storage/page/page.h"

namespace bustub {

template <typename K, typename V>
ExtendibleHashTable<K, V>::ExtendibleHashTable(size_t bucket_size)
    : global_depth_(0), bucket_size_(bucket_size), num_buckets_(1) {
  if (dir_.empty()) {
    std::shared_ptr<Bucket> ibptr = std::make_shared<Bucket>(bucket_size_);
    dir_.push_back(ibptr);
  }
}

// template <typename K, typename V>
// auto ExtendibleHashTable<K, V>::RedistributeBucket(std::shared_ptr<Bucket> bucket) -> void {
//   std::list<std::pair<K, V>> &list = bucket->GetItems();

//   for (auto it = list.begin(); it != list.end();) {
//     auto next = it;
//     next++;
//     int index = IndexOf((*it).first);
//     if (dir_[index] == bucket) {
//       it = next;
//       continue;
//     }
//     std::shared_ptr<Bucket> bptr = dir_[index];
//     bptr->Insert((*it).first, (*it).second);
//     list.erase(it);
//     it = next;
//   }
// }

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::IndexOf(const K &key) -> size_t {
  int mask = (1 << global_depth_) - 1;
  return std::hash<K>()(key) & mask;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetGlobalDepth() const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetGlobalDepthInternal();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetGlobalDepthInternal() const -> int {
  return global_depth_;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetLocalDepth(int dir_index) const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetLocalDepthInternal(dir_index);
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetLocalDepthInternal(int dir_index) const -> int {
  return dir_[dir_index]->GetDepth();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetNumBuckets() const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetNumBucketsInternal();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetNumBucketsInternal() const -> int {
  return num_buckets_;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Find(const K &key, V &value) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  std::shared_ptr<Bucket> bptr = dir_[IndexOf(key)];
  return bptr->Find(key, value);
  // UNREACHABLE("not implemented");
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Remove(const K &key) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  std::shared_ptr<Bucket> bptr = dir_[IndexOf(key)];
  return bptr->Remove(key);
  // UNREACHABLE("not implemented");
}

template <typename K, typename V>
void ExtendibleHashTable<K, V>::Insert(const K &key, const V &value) {
  std::scoped_lock<std::mutex> lock(latch_);

  while (dir_[IndexOf(key)]->IsFull()) {  // 需要递归判断是否能将要插入的桶是否为满
    size_t index = IndexOf(key);

    auto target_bucket = dir_[index];

    int bucket_localdepth = target_bucket->GetDepth();

    if (global_depth_ == bucket_localdepth) {
      int capacity = dir_.size();

      dir_.resize(capacity << 1);

      for (int i = 0; i < capacity; i++) {  // 调整索引
        dir_[i + capacity] = dir_[i];
      }

      global_depth_++;
    }

    int mask = 1 << bucket_localdepth;

    // 声明两个桶，废除原来目录中指向的那个桶。

    auto bucket_0 = std::make_shared<Bucket>(bucket_size_, bucket_localdepth + 1);

    auto bucket_1 = std::make_shared<Bucket>(bucket_size_, bucket_localdepth + 1);

    // 重新调整桶中数据

    for (auto &item : target_bucket->GetItems()) {
      size_t hash_key = std::hash<K>()(item.first);

      if ((hash_key & mask) != 0U) {
        bucket_1->Insert(item.first, item.second);

      } else {
        bucket_0->Insert(item.first, item.second);
      }
    }

    num_buckets_++;

    // 调整桶中数据

    for (size_t i = 0; i < dir_.size(); i++) {
      if (dir_[i] == target_bucket) {
        if ((i & mask) == 0U) {
          dir_[i] = bucket_0;

        } else {
          dir_[i] = bucket_1;
        }
      }
    }
  }

  // 将数据插入桶中

  auto index = IndexOf(key);

  auto target_bucket = dir_[index];

  target_bucket->Insert(key, value);
  // int index = IndexOf(key);

  // std::shared_ptr<Bucket> bptr = dir_[index];
  // while (bptr->IsFull()) {
  //   if (bptr->GetDepth() == global_depth_) {
  //     ++global_depth_;
  //     int vsize = dir_.size();
  //     for (int i = 0; i < vsize; ++i) {
  //       dir_.push_back(dir_[i]);
  //     }
  //   }
  //   bptr->IncrementDepth();
  //   std::shared_ptr<Bucket> ibptr = std::make_shared<Bucket>(bucket_size_, bptr->GetDepth());
  //   num_buckets_++;
  //   int mask1 = (1 << (bptr->GetDepth() - 1));
  //   int mask2 = (1 << (bptr->GetDepth() - 1)) - 1;
  //   // 重定向dir
  //   for (int i = index; i < static_cast<int>(dir_.size()); ++i) {
  //     if ((i & mask2) == (index & mask2) && (i & mask1) != 0) {
  //       dir_[i] = ibptr;
  //     }
  //   }

  //   // 重新在Bucket种安排kv对
  //   RedistributeBucket(bptr);
  //   // 重新确定bucket
  //   index = IndexOf(key);
  //   bptr = dir_[index];
  // }
  // bptr->Insert(key, value);
  // // 输出数据库
  // for (int i = 0; i < static_cast<int>(dir_.size()); ++i) {
  //   std::cout << i << ": ";
  //   std::list<std::pair<K, V>> &list = dir_[i]->GetItems();
  //   for (auto &j : list) {
  //     std::cout << j.first << " ";
  //   }
  //   std::cout << "   gloabal: " << global_depth_ << "   loacal: " << dir_[i]->GetDepth();
  //   std::cout << std::endl;
  // }
  // UNREACHABLE("not implemented");
}

//===--------------------------------------------------------------------===//
// Bucket
//===--------------------------------------------------------------------===//
template <typename K, typename V>
ExtendibleHashTable<K, V>::Bucket::Bucket(size_t array_size, int depth) : size_(array_size), depth_(depth) {}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Find(const K &key, V &value) -> bool {
  // UNREACHABLE("not implemented");
  // 自动上锁解锁
  // std::unique_lock<std::mutex> lock(mtx_);
  // mtx_.lock();
  for (auto it = list_.begin(); it != list_.end(); ++it) {
    if ((*it).first == key) {
      value = (*it).second;
      return true;
    }
  }
  // for (std::pair<K, V> &i : list_) {
  //   if (i.first == key) {
  //     value = i.second;
  //     // mtx_.unlock();
  //     return true;
  //   }
  // }
  // mtx_.unlock();
  return false;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Remove(const K &key) -> bool {
  // UNREACHABLE("not implemented");
  // std::unique_lock<std::mutex> lock(mtx_);
  for (auto it = list_.begin(); it != list_.end(); ++it) {
    if ((*it).first == key) {
      list_.erase(it);
      return true;
    }
  }
  // for (auto &i : list_) {
  //   if (i.first == key) {
  //     list_.remove(i);
  //     return true;
  //   }
  // }
  return false;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Insert(const K &key, const V &value) -> bool {
  // UNREACHABLE("not implemented");
  // std::unique_lock<std::mutex> lock(mtx_);
  if (IsFull()) {
    return false;
  }
  for (auto it = list_.begin(); it != list_.end(); ++it) {
    if ((*it).first == key) {
      (*it).second = value;
      return true;
    }
  }
  // for (std::pair<K, V> &i : list_) {
  //   if (i.first == key) {
  //     i.second = value;
  //     return true;
  //   }
  // }
  list_.emplace_back(key, value);
  return true;
}

template class ExtendibleHashTable<page_id_t, Page *>;
template class ExtendibleHashTable<Page *, std::list<Page *>::iterator>;
template class ExtendibleHashTable<int, int>;
// test purpose
template class ExtendibleHashTable<int, std::string>;
template class ExtendibleHashTable<int, std::list<int>::iterator>;

}  // namespace bustub
