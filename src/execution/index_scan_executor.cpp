//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// index_scan_executor.cpp
//
// Identification: src/execution/index_scan_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/index_scan_executor.h"

namespace bustub {
IndexScanExecutor::IndexScanExecutor(ExecutorContext *exec_ctx, const IndexScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan) {}

void IndexScanExecutor::Init() {
  index_info_ = exec_ctx_->GetCatalog()->GetIndex(plan_->index_oid_);
  tree_ = reinterpret_cast<BPlusTreeIndexForOneIntegerColumn *>(index_info_->index_.get());
  iter_ = tree_->GetBeginIterator();
}

auto IndexScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (iter_ != tree_->GetEndIterator()) {
    auto table_heap = exec_ctx_->GetCatalog()->GetTable(index_info_->table_name_)->table_.get();
    *rid = (*iter_).second;
    table_heap->GetTuple(*rid, tuple, exec_ctx_->GetTransaction());
    ++iter_;
    return true;
  }
  return false;
}

}  // namespace bustub
