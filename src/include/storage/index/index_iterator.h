//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/index/index_iterator.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
 public:
  // you may define your own constructor based on your member variables
  IndexIterator();
  IndexIterator(page_id_t page_id, page_id_t index_in_leaf, BufferPoolManager *bpm);
  ~IndexIterator();  // NOLINT

  auto IsEnd() -> bool;

  auto operator*() -> const MappingType &;

  auto operator++() -> IndexIterator &;

  auto operator==(const IndexIterator &itr) const -> bool {
      return page_id_ == itr.page_id_ && index_in_leaf_ == itr.index_in_leaf_;
  }

  auto operator!=(const IndexIterator &itr) const -> bool {
      return page_id_ != itr.page_id_ || index_in_leaf_ != itr.index_in_leaf_;
  }
  auto Get_page_id() const -> page_id_t {
      return page_id_;
  }
  auto Get_index() const -> page_id_t {
        return index_in_leaf_;
  }
    auto Get_size() const -> page_id_t {
        return leaf_page_->GetSize();
    }

 private:
  // add your own private member variables here
  // 页id
  page_id_t  page_id_ = INVALID_PAGE_ID;
  Page *page_ = nullptr;
  B_PLUS_TREE_LEAF_PAGE_TYPE *leaf_page_ = nullptr;
  // 页内id
  int index_in_leaf_ = 0;
  BufferPoolManager *buffer_pool_manager_ = nullptr;
};

}  // namespace bustub
