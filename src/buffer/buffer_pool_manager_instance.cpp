//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"

#include "common/exception.h"
#include "common/macros.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  page_table_ = new ExtendibleHashTable<page_id_t, frame_id_t>(bucket_size_);
  replacer_ = new LRUKReplacer(pool_size, replacer_k);

  // Initially, every page is in the free list.
  // for (size_t i = 0; i < pool_size_; ++i) {
  //   free_list_.emplace_back(static_cast<int>(i));
  // }

  free_list_.resize(pool_size);
  std::iota(free_list_.begin(), free_list_.end(), 0);
  // TODO(students): remove this line after you have implemented the buffer pool manager
  // throw NotImplementedException(
  //     "BufferPoolManager is not implemented yet. If you have finished implementing BPM, please remove the throw "
  //     "exception line in `buffer_pool_manager_instance.cpp`.");
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete page_table_;
  delete replacer_;
}

auto BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t available_frame_id = -1;
  // 找不到有效的
  if (!FindVictim(&available_frame_id)) {
    return nullptr;
  }
  page_id_t new_page_id = AllocatePage();
  pages_[available_frame_id].ResetMemory();
  pages_[available_frame_id].page_id_ = new_page_id;
  pages_[available_frame_id].pin_count_ = 1;
  pages_[available_frame_id].is_dirty_ = false;
  replacer_->RecordAccess(available_frame_id);
  replacer_->SetEvictable(available_frame_id, false);
  *page_id = new_page_id;
  page_table_->Insert(new_page_id, available_frame_id);
  return &pages_[available_frame_id];
}

auto BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t exist_frame_id = -1;
  // 如果在缓存中找到了 直接返回找到的page
  if (page_table_->Find(page_id, exist_frame_id)) {
    replacer_->RecordAccess(exist_frame_id);
    replacer_->SetEvictable(exist_frame_id, false);
    pages_[exist_frame_id].pin_count_++;
    return &pages_[exist_frame_id];
  }

  frame_id_t available_frame_id = -1;
  // 没有可用的frame分配 返回空
  if (!FindVictim(&available_frame_id)) {
    return nullptr;
  }

  pages_[available_frame_id].ResetMemory();
  pages_[available_frame_id].page_id_ = page_id;
  pages_[available_frame_id].pin_count_ = 1;
  pages_[available_frame_id].is_dirty_ = false;
  // 从磁盘中读入数据
  disk_manager_->ReadPage(page_id, pages_[available_frame_id].GetData());
  replacer_->RecordAccess(available_frame_id);
  // 有正在使用改page的用户 设置不可弹出
  replacer_->SetEvictable(available_frame_id, false);
  page_table_->Insert(page_id, available_frame_id);
  return &pages_[available_frame_id];
}

auto BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t frame_id = -1;
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }
  if (pages_[frame_id].GetPinCount() == 0) {
    return false;
  }
  if (--pages_[frame_id].pin_count_ == 0) {
    // 没人使用  可以被换出
    replacer_->SetEvictable(frame_id, true);
  }

  if (is_dirty) {
    pages_[frame_id].is_dirty_ = is_dirty;
  }
  return true;
}

auto BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t frame_id = -1;
  if (!page_table_->Find(page_id, frame_id)) {
    // page id is not present in the buffer pool
    return false;
  }
  // flush regardless of dirty or not
  disk_manager_->WritePage(page_id, pages_[frame_id].GetData());
  pages_[frame_id].is_dirty_ = false;  // reset dirty flag regardless

  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  std::scoped_lock<std::mutex> lock(latch_);
  for (size_t i = 0; i < pool_size_; i++) {
    if (pages_[i].page_id_ != INVALID_PAGE_ID) {
      // there is a valid physical page resides, regardless of dirty or not
      disk_manager_->WritePage(pages_[i].page_id_, pages_[i].GetData());
      pages_[i].is_dirty_ = false;
    }
  }
}

auto BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t frame_id = -1;
  if (!page_table_->Find(page_id, frame_id)) {
    return true;
  }
  if (pages_[frame_id].pin_count_ != 0) {
    return false;
  }

  if (pages_[frame_id].is_dirty_) {
    disk_manager_->WritePage(pages_[frame_id].page_id_, pages_[frame_id].GetData());
    pages_[frame_id].is_dirty_ = false;
  }

  page_table_->Remove(page_id);
  replacer_->Remove(frame_id);
  free_list_.push_back(frame_id);
  pages_[frame_id].ResetMemory();
  pages_[frame_id].page_id_ = INVALID_PAGE_ID;
  pages_[frame_id].pin_count_ = 0;
  pages_[frame_id].is_dirty_ = false;
  DeallocatePage(page_id);
  return true;
}

auto BufferPoolManagerInstance::AllocatePage() -> page_id_t { return next_page_id_++; }

// 查找有效的frame   如果free_list为空且缓存中的都是不可evict的  则返回false
auto BufferPoolManagerInstance::FindVictim(frame_id_t *available_frame_id) -> bool {
  if (!free_list_.empty()) {
    *available_frame_id = free_list_.back();
    free_list_.pop_back();
    return true;
  }
  if (replacer_->Evict(available_frame_id)) {
    if (pages_[*available_frame_id].IsDirty()) {
      disk_manager_->WritePage(pages_[*available_frame_id].page_id_, pages_[*available_frame_id].GetData());
      pages_[*available_frame_id].is_dirty_ = false;
    }
    page_table_->Remove(pages_[*available_frame_id].page_id_);
    return true;
  }
  return false;
}

}  // namespace bustub
