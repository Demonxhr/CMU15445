//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_leaf_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <sstream>

#include "common/exception.h"
#include "common/rid.h"
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  // 设置page类型为叶页面
  SetPageType(IndexPageType::LEAF_PAGE);
  // 设置pageid
  SetPageId(page_id);
  // 暂时下一个节点为空
  SetNextPageId(INVALID_PAGE_ID);
  // 设置父节点
  SetParentPageId(parent_id);
  // 设置页面中键和值对的数量 初始为0，没有键值对
  SetSize(0);
  // 设置页面中最大键值对数量
  SetMaxSize(max_size);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const -> page_id_t { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
// 查找页面中对应下标的key
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  return array_[index].first;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::ValueAt(int index) const -> ValueType {
  // replace with your own code
  return array_[index].second;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyValueAt(int index) const -> const MappingType & { return array_[index]; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetKeyValueAt(int index, KeyType key, ValueType value) {
  array_[index] = MappingType{key, value};
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key, const ValueType &value, const KeyComparator &comp) {
  int size = GetSize();
  if (size > GetMaxSize()) {
    std::cout << "leaf "
              << "size: " << GetSize() << ", "
              << "maxsize: " << GetMaxSize() << std::endl;
    throw std::logic_error("B+ Tree leaf page already reached max_size before insert.Should have splitted before");
  }
  int ins_at = 0;
  while (ins_at < size && comp(array_[ins_at].first, key) < 0) {
    ++ins_at;
  }

  for (int i = size; i > ins_at; --i) {
    array_[i] = array_[i - 1];
  }
  IncreaseSize(1);
  SetKeyValueAt(ins_at, key, value);
}
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveDataTo(B_PLUS_TREE_LEAF_PAGE_TYPE *new_page, int count) {
  int size = GetSize();
  int start_index = size - count;
  for (int i = start_index, j = 0; i < size; ++i, ++j) {
    new_page->SetKeyValueAt(j, array_[i].first, array_[i].second);
  }
  new_page->IncreaseSize(count);
  SetSize(size - count);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::LowerBound(const KeyType &key, const KeyComparator &comp) -> int {
  int index = 0;
  int size = GetSize();
  while (index < size && comp(array_[index].first, key) < 0) {
    ++index;
  }
  return index;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Remove(const KeyType &key, const KeyComparator &comp) {
  int index = 0;
  int size = GetSize();
  while (index < size && comp(array_[index].first, key) < 0) {
    ++index;
  }
  // 没找到的情况 直接返回  啥也不改变
  if (index >= size || comp(array_[index].first, key) > 0) {
    return;
  }

  for (int i = index; i < size - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  DecreaseSize(1);
}
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::RemoveAt(const int index) {
  int size = GetSize();
  for (int i = index; i < size - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  DecreaseSize(1);
}

template class BPlusTreeLeafPage<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
