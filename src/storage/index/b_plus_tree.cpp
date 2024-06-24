#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/header_page.h"

namespace bustub {

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {
  //    std::cout << "leaf_max" << leaf_max_size_ << std::endl;
  //    std::cout << "internal_max" << internal_max_size_ << std::endl;
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetLeafPage(const KeyType &key) -> Page * {
  page_id_t next_page_id = root_page_id_;
  while (true) {
    Page *page = buffer_pool_manager_->FetchPage(next_page_id);
    auto tree_page = reinterpret_cast<BPlusTreePage *>(page->GetData());
    if (tree_page->IsLeafPage()) {
      return page;
    }
    auto internal_page = reinterpret_cast<InternalPage *>(tree_page);
    // 设置为最后一个,保证可以遍历到叶节点
    next_page_id = internal_page->ValueAt(internal_page->GetSize() - 1);
    // 第一个为空  因为n个key 可以划分n+1的区域
    for (int i = 1; i < internal_page->GetSize(); ++i) {
      if (comparator_(internal_page->KeyAt(i), key) > 0) {
        next_page_id = internal_page->ValueAt(i - 1);
        break;
      }
    }
    buffer_pool_manager_->UnpinPage(internal_page->GetPageId(), false);
  }
}
/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() const -> bool { return root_page_id_ == INVALID_PAGE_ID; }
/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *transaction) -> bool {
  // std::cout << "get: " << key << std::endl;
  bool found = false;
  Page *page = GetLeafPage(key);
  auto leaf_page = reinterpret_cast<LeafPage *>(page->GetData());
  for (int i = 0; i < leaf_page->GetSize(); ++i) {
    // std::cout << leaf_page->KeyAt(i) <<std::endl;
    if (comparator_(leaf_page->KeyAt(i), key) == 0) {
      result->emplace_back(leaf_page->ValueAt(i));
      found = true;
    }
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), false);

  return found;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction) -> bool {
  // std::cout << "insert: " << key << std::endl;
  // 如果为空树 创建叶节点为根节点
  if (IsEmpty()) {
    Page *page = buffer_pool_manager_->NewPage(&root_page_id_);
    UpdateRootPageId(1);
    auto leaf_page = reinterpret_cast<LeafPage *>(page->GetData());
    // 设置本节点id    设置父节点为不合法  没有父节点    设置页面中最大键值对数量
    leaf_page->Init(root_page_id_, INVALID_PAGE_ID, leaf_max_size_);
    // 设置下一个节点为-1
    leaf_page->SetNextPageId(INVALID_PAGE_ID);
    leaf_page->SetKeyValueAt(0, key, value);
    leaf_page->IncreaseSize(1);
    buffer_pool_manager_->UnpinPage(root_page_id_, true);
    return true;
  }
  Page *page = GetLeafPage(key);
  auto leaf_page = reinterpret_cast<LeafPage *>(page->GetData());
  // 叶子节点从0个点开始遍历  因为叶子节点不需要指向下一个节点
  for (int i = 0; i < leaf_page->GetSize(); ++i) {
    // 如果要插入的节点存在
    if (comparator_(leaf_page->KeyAt(i), key) == 0) {
      buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), false);
      return false;
    }
  }

  // key不在树中
  leaf_page->Insert(key, value, comparator_);

  // 插入key不会发生分裂时
  if (leaf_page->GetSize() < leaf_max_size_) {
    buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), true);
    return true;
  }

  // 插入树中发生分裂
  page_id_t new_leave_page_id;
  Page *new_page_t = buffer_pool_manager_->NewPage(&new_leave_page_id);
  LeafPage *new_leaf_page = reinterpret_cast<LeafPage *>(new_page_t);
  new_leaf_page->Init(new_leave_page_id, leaf_page->GetParentPageId(), leaf_max_size_);

  // 设置链表
  new_leaf_page->SetNextPageId(leaf_page->GetNextPageId());
  leaf_page->SetNextPageId(new_leave_page_id);
  // 移动当前页面(leaf_max_size_+1)/2后的元素到目标页面
  leaf_page->MoveDataTo(new_leaf_page, (leaf_max_size_ + 1) / 2);

  BPlusTreePage *old_tree_page = leaf_page;
  BPlusTreePage *new_tree_page = new_leaf_page;
  KeyType split_key = new_leaf_page->KeyAt(0);

  while (true) {
    // 如果old_tree_page为根节点 则创建一个新的根节点
    if (old_tree_page->IsRootPage()) {
      Page *new_page = buffer_pool_manager_->NewPage(&root_page_id_);
      auto new_root_page = reinterpret_cast<InternalPage *>(new_page);
      new_root_page->Init(root_page_id_, INVALID_PAGE_ID, internal_max_size_);
      // 指向小的，key的值不起作用  不进入判断
      new_root_page->SetKeyValueAt(0, split_key, old_tree_page->GetPageId());
      // 指向大的
      new_root_page->SetKeyValueAt(1, split_key, new_tree_page->GetPageId());
      new_root_page->IncreaseSize(1);
      old_tree_page->SetParentPageId(root_page_id_);
      new_tree_page->SetParentPageId(root_page_id_);
      // 根节点变了需要更新
      UpdateRootPageId();
      // std::cout << "new_root_page "
      //           << "size: " << new_root_page->GetSize() << ", "
      //           << "maxsize: " << new_root_page->GetMaxSize() << std::endl;
      // std::cout << "old_tree_page "
      //           << "size: " << old_tree_page->GetSize() << ", "
      //           << "maxsize: " << old_tree_page->GetMaxSize() << std::endl;
      // std::cout << "new_tree_page "
      //           << "size: " << new_tree_page->GetSize() << ", "
      //           << "maxsize: " << new_tree_page->GetMaxSize() << std::endl;
      // unpin 根节点
      buffer_pool_manager_->UnpinPage(new_root_page->GetPageId(), true);
      break;
    }

    page_id_t parent_page_id = old_tree_page->GetParentPageId();
    Page *parent_page = buffer_pool_manager_->FetchPage(parent_page_id);
    auto parent_internal_page = reinterpret_cast<InternalPage *>(parent_page->GetData());
    // 索引节点Key设置为新页面的第一个KeY value设置为新页面id
    parent_internal_page->Insert(split_key, new_tree_page->GetPageId(), comparator_);
    // 设置新页面 父节点
    new_tree_page->SetParentPageId(parent_internal_page->GetPageId());

    // 判断父节点是否溢出
    if (parent_internal_page->GetSize() <= internal_max_size_) {
      buffer_pool_manager_->UnpinPage(parent_page_id, true);
      break;
    }

    // 父节点溢出
    page_id_t new_internal_page_id;
    Page *new_page = buffer_pool_manager_->NewPage(&new_internal_page_id);
    auto new_internal_page = reinterpret_cast<InternalPage *>(new_page);
    new_internal_page->Init(new_internal_page_id, parent_internal_page->GetParentPageId(), internal_max_size_);
    int new_page_size = (internal_max_size_ + 1) / 2;
    size_t start_index = parent_internal_page->GetSize() - new_page_size;
    // 将元素移动到新page
    for (int i = start_index, j = 0; i < parent_internal_page->GetSize(); ++i, ++j) {
      new_internal_page->SetKeyValueAt(j, parent_internal_page->KeyAt(i), parent_internal_page->ValueAt(i));
      // 取出该元素指向的page， 修改该page的父节点
      Page *page_t = buffer_pool_manager_->FetchPage(parent_internal_page->ValueAt(i));
      auto tree_page = reinterpret_cast<BPlusTreePage *>(page_t);
      // 修改该page的父节点
      tree_page->SetParentPageId(new_internal_page_id);
      buffer_pool_manager_->UnpinPage(tree_page->GetPageId(), true);
    }
    parent_internal_page->SetSize(internal_max_size_ - new_page_size + 1);
    new_internal_page->SetSize(new_page_size);

    buffer_pool_manager_->UnpinPage(old_tree_page->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_tree_page->GetPageId(), true);
    old_tree_page = parent_internal_page;
    new_tree_page = new_internal_page;
    split_key = new_internal_page->KeyAt(0);
  }
  buffer_pool_manager_->UnpinPage(old_tree_page->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(new_tree_page->GetPageId(), true);
  return true;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
  // std::cout << "remove: " << key << std::endl;
  if (IsEmpty()) {
    return;
  }

  Page *page = GetLeafPage(key);
  auto leaf_page = reinterpret_cast<LeafPage *>(page->GetData());
  leaf_page->Remove(key, comparator_);

  if (leaf_page->IsRootPage()) {
    return;
  }

  // 发生下溢出
  if (leaf_page->GetSize() < leaf_page->GetMinSize()) {
    // 递归调用
    HandleUnderflow(leaf_page, transaction);
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UnpinSiblings(page_id_t left_sibling_id, page_id_t right_sibling_id) {
  if (left_sibling_id != INVALID_PAGE_ID) buffer_pool_manager_->UnpinPage(left_sibling_id, true);
  if (right_sibling_id != INVALID_PAGE_ID) buffer_pool_manager_->UnpinPage(right_sibling_id, true);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::SetPageParentId(page_id_t child_pageid, page_id_t parent_pageid) {
  Page *page = buffer_pool_manager_->FetchPage(child_pageid);
  BPlusTreePage *treepage = reinterpret_cast<BPlusTreePage *>(page->GetData());
  treepage->SetParentPageId(parent_pageid);
  buffer_pool_manager_->UnpinPage(child_pageid, true);
}

// 当无法从左右兄弟处借节点时选择和他们合并
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::MergePage(BPlusTreePage *left_page, BPlusTreePage *right_page, InternalPage *parent_page) {
  // 先从叶节点开始
  if (left_page->IsLeafPage()) {
    auto left_leaf_page = static_cast<LeafPage *>(left_page);
    auto right_leaf_page = static_cast<LeafPage *>(right_page);

    for (int i = 0; i < right_leaf_page->GetSize(); ++i) {
      left_leaf_page->Insert(right_leaf_page->KeyAt(i), right_leaf_page->ValueAt(i), comparator_);
    }
    left_leaf_page->SetNextPageId(right_leaf_page->GetNextPageId());
    // 从父节点中删除合并完后的右节点
    parent_page->RemoveAt(parent_page->FindValue(right_page->GetPageId()));
  } else {
    auto left_internal_page = static_cast<InternalPage *>(left_page);
    auto right_internal_page = static_cast<InternalPage *>(right_page);
    // 父节点下移
    left_internal_page->Insert(parent_page->KeyAt(parent_page->FindValue(right_page->GetPageId())),
                               right_internal_page->ValueAt(0), comparator_);
    SetPageParentId(right_internal_page->ValueAt(0), left_internal_page->GetPageId());
    // 从父节点中删除合并完后的右节点
    parent_page->RemoveAt(parent_page->FindValue(right_page->GetPageId()));
    for (int i = 1; i < right_internal_page->GetSize(); ++i) {
      left_internal_page->Insert(right_internal_page->KeyAt(i), right_internal_page->ValueAt(i), comparator_);
      SetPageParentId(right_internal_page->ValueAt(i), left_internal_page->GetPageId());
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::HandleUnderflow(BPlusTreePage *page, Transaction *transaction) {
  // 处理是根节点的情况  （递归到根节点的情况）
  if (page->IsRootPage()) {
    if (page->IsLeafPage() || page->GetSize() > 1) {
      return;
    }

    // 如果根节点不是叶节点且根节点的大小小于等于1时  需要节点上移 把唯一的子节点作为根节点
    auto old_root_page = static_cast<InternalPage *>(page);
    root_page_id_ = old_root_page->ValueAt(0);
    auto new_root_page = reinterpret_cast<InternalPage *>(buffer_pool_manager_->FetchPage(root_page_id_)->GetData());
    new_root_page->SetParentPageId(INVALID_PAGE_ID);
    buffer_pool_manager_->UnpinPage(root_page_id_, true);
    UpdateRootPageId();
    return;
  }
  page_id_t left_sibling_id;
  page_id_t right_sibling_id;
  GetSiblings(page, left_sibling_id, right_sibling_id);
  if (left_sibling_id == INVALID_PAGE_ID && right_sibling_id == INVALID_PAGE_ID) {
    throw std::logic_error("Non-root page * " + std::to_string(page->GetPageId()) + " has no sibling.");
  }
  BPlusTreePage *left_sibling_page = nullptr;
  BPlusTreePage *right_sibling_page = nullptr;
  // 优先左边
  if (left_sibling_id != INVALID_PAGE_ID) {
    left_sibling_page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(left_sibling_id)->GetData());
  }
  if (right_sibling_id != INVALID_PAGE_ID) {
    right_sibling_page =
        reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(right_sibling_id)->GetData());
  }
  auto parent_page =
      reinterpret_cast<InternalPage *>(buffer_pool_manager_->FetchPage(page->GetParentPageId())->GetData());

  if (TryBorrow(page, left_sibling_page, parent_page, true) ||
      TryBorrow(page, right_sibling_page, parent_page, false)) {
    UnpinSiblings(left_sibling_id, right_sibling_id);
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
    return;
  }
  BPlusTreePage *left_page;
  BPlusTreePage *right_page;
  if (left_sibling_page != nullptr) {
    left_page = left_sibling_page;
    right_page = page;
  } else {
    left_page = page;
    right_page = right_sibling_page;
  }
  MergePage(left_page, right_page, parent_page);
  UnpinSiblings(left_sibling_id, right_sibling_id);
  // 子节点下溢出解决后导致父节点下溢出  递归解决
  if (parent_page->GetSize() < parent_page->GetMinSize()) {
    HandleUnderflow(parent_page, transaction);
  }
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::TryBorrow(BPlusTreePage *page, BPlusTreePage *sibling_page, InternalPage *parent_page,
                               bool sibling_at_left) -> bool {
  // 兄弟也借不了
  if (sibling_page == nullptr || sibling_page->GetSize() <= sibling_page->GetMinSize()) {
    return false;
  }
  // 兄弟节点在左边  从最后面借  兄弟节点在右边 从第一个借   第一个需要判断是不是内部节点  内部节点的第一个为0 没有意义
  // 不报错key
  int sibling_borrow_at = sibling_at_left ? sibling_page->GetSize() - 1 : (page->IsLeafPage() ? 0 : 1);
  // 从左侧偷取时 是更新父节点中当前page的索引    从右侧偷取时  是更新当前page的下一个page的索引
  int parent_update_at = parent_page->FindValue(page->GetPageId()) + (sibling_at_left ? 0 : 1);
  KeyType update_key;
  // 如果是叶子节点
  if (page->IsLeafPage()) {
    auto leaf_page = static_cast<LeafPage *>(page);
    auto leaf_sibling_page = static_cast<LeafPage *>(sibling_page);
    leaf_page->Insert(leaf_sibling_page->KeyAt(sibling_borrow_at), leaf_sibling_page->ValueAt(sibling_borrow_at),
                      comparator_);
    leaf_sibling_page->RemoveAt(sibling_borrow_at);
    update_key = sibling_at_left ? leaf_page->KeyAt(0) : leaf_sibling_page->KeyAt(0);
  } else {  // 非叶子节点
    auto internal_page = static_cast<InternalPage *>(page);
    auto internal_sibling_page = static_cast<InternalPage *>(sibling_page);
    update_key = internal_sibling_page->KeyAt(sibling_borrow_at);
    page_id_t child_id;
    if (sibling_at_left) {
      // 将父节点的key插到本节点
      internal_page->Insert(parent_page->KeyAt(parent_update_at), internal_page->ValueAt(0), comparator_);
      // 设置第一个节点为兄弟节点的最后一个索引
      internal_page->SetValueAt(0, internal_sibling_page->ValueAt(sibling_borrow_at));
      child_id = internal_page->ValueAt(0);
    } else {
      // 在最后插入  下溢出肯定可以插入
      internal_page->SetKeyValueAt(internal_page->GetSize(), parent_page->KeyAt(parent_update_at),
                                   internal_sibling_page->ValueAt(0));
      internal_page->IncreaseSize(1);
      internal_sibling_page->SetValueAt(0, internal_sibling_page->ValueAt(1));
      child_id = internal_page->ValueAt(internal_page->GetSize() - 1);
    }
    internal_sibling_page->RemoveAt(sibling_borrow_at);
    Page *page1 = buffer_pool_manager_->FetchPage(child_id);
    auto child_page = reinterpret_cast<BPlusTreePage *>(page1->GetData());
    child_page->SetParentPageId(internal_page->GetPageId());
    buffer_pool_manager_->UnpinPage(child_id, true);
  }
  parent_page->SetKeyAt(parent_update_at, update_key);
  return true;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::GetSiblings(BPlusTreePage *page, page_id_t &left_sibling_id, page_id_t &right_sibling_id) {
  // 根节点没有兄弟节点
  if (page->IsRootPage()) {
    throw std::invalid_argument("Cannot get sibling of the root page");
  }
  auto parent_page =
      reinterpret_cast<InternalPage *>(buffer_pool_manager_->FetchPage(page->GetParentPageId())->GetData());
  int index = parent_page->FindValue(page->GetPageId());
  left_sibling_id = right_sibling_id = INVALID_PAGE_ID;
  if (index != 0) {
    left_sibling_id = parent_page->ValueAt(index - 1);
  }
  if (index != parent_page->GetSize() - 1) {
    right_sibling_id = parent_page->ValueAt(index + 1);
  }
  buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), false);
}
/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE {
  // 从根节点向下找到第一个叶子节点 返回该节点的迭代器
  page_id_t next_page_id = root_page_id_;
  while (true) {
    Page *page = buffer_pool_manager_->FetchPage(next_page_id);
    auto tree_page = reinterpret_cast<BPlusTreePage *>(page->GetData());
    if (tree_page->IsLeafPage()) {
      // 迭代器设计 为初始化pageid index_in_leaf_ 缓存管理器
      return INDEXITERATOR_TYPE(tree_page->GetPageId(), 0, buffer_pool_manager_);
    }
    auto internal_page = static_cast<InternalPage *>(tree_page);
    if (internal_page == nullptr) {
      throw std::bad_cast();
    }
    // 指向下一层第一个节点
    next_page_id = internal_page->ValueAt(0);
    buffer_pool_manager_->UnpinPage(internal_page->GetPageId(), false);
  }
}

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE {
  Page *page = GetLeafPage(key);
  auto leaf_page = reinterpret_cast<LeafPage *>(page->GetData());

  return INDEXITERATOR_TYPE(page->GetPageId(), leaf_page->LowerBound(key, comparator_), buffer_pool_manager_);
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE {
  // 返回一个无效迭代器表示end
  return INDEXITERATOR_TYPE(INVALID_PAGE_ID, 0, buffer_pool_manager_);
}

/**
 * @return Page id of the root of this tree
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t { return root_page_id_; }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      defualt value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
  auto *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  if (insert_record != 0) {
    // create a new record<index_name + root_page_id> in header_page
    header_page->InsertRecord(index_name_, root_page_id_);
  } else {
    // update root_page_id in header_page
    header_page->UpdateRecord(index_name_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name, Transaction *transaction) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid, transaction);
  }
}
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name, Transaction *transaction) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key, transaction);
  }
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Draw(BufferPoolManager *bpm, const std::string &outf) {
  if (IsEmpty()) {
    LOG_WARN("Draw an empty tree");
    return;
  }
  std::ofstream out(outf);
  out << "digraph G {" << std::endl;
  ToGraph(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm, out);
  out << "}" << std::endl;
  out.flush();
  out.close();
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Print(BufferPoolManager *bpm) {
  if (IsEmpty()) {
    LOG_WARN("Print an empty tree");
    return;
  }
  ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm);
}

/**
 * This method is used for debug only, You don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 * @param out
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out) const {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    auto *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> " << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

/**
 * This function is for debug only, you don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    auto *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId() << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
