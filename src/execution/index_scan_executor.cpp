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
  auto txn = exec_ctx_->GetTransaction();
  auto lkm = exec_ctx_->GetLockManager();
  auto table_info = exec_ctx_->GetCatalog()->GetTable(plan_->table_name_);
  try {
    if (!txn->IsTableIntentionSharedLocked(table_info->oid_) &&
        txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED &&
        !lkm->LockTable(exec_ctx_->GetTransaction(), LockManager::LockMode::INTENTION_SHARED, table_info->oid_)) {
      throw ExecutionException("lock index table fail");
    }
  } catch (TransactionAbortException &e) {
    throw ExecutionException("execute lock table fail");
  }
  auto index = exec_ctx_->GetCatalog()->GetIndex(plan_->index_oid_)->index_.get();
  Tuple key({plan_->val_}, index->GetKeySchema());
  result_.clear();
  index->ScanKey(key, &result_, txn);
  iter_begin_ = result_.begin();
  iter_end_ = result_.end();
  //  index_info_ = exec_ctx_->GetCatalog()->GetIndex(plan_->index_oid_);
  //  tree_ = reinterpret_cast<BPlusTreeIndexForOneIntegerColumn *>(index_info_->index_.get());
  //  iter_ = tree_->GetBeginIterator();
}

auto IndexScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  auto txn = exec_ctx_->GetTransaction();
  auto lkm = exec_ctx_->GetLockManager();
  auto table_info = exec_ctx_->GetCatalog()->GetTable(plan_->table_name_);
  if (iter_begin_ != iter_end_) {
    try {
      *rid = *iter_begin_;
      if (!txn->IsRowExclusiveLocked(table_info->oid_, *rid) && !txn->IsRowSharedLocked(table_info->oid_, *rid) &&
          txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED) {
        if (!lkm->LockRow(txn, LockManager::LockMode::SHARED, table_info->oid_, *rid)) {
          throw ExecutionException("index lock row fail");
        }
      }
      // if (!exec_ctx_->GetCatalog()->GetTable())
      if (txn->IsRowSharedLocked(table_info->oid_, *rid)) {
        if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED) {
          if (!lkm->UnlockRow(txn, table_info->oid_, *rid)) {
            throw ExecutionException("index unlock row shared fail");
          }
        }
      }
    } catch (TransactionAbortException &e) {
      throw ExecutionException("index execute lock fail");
    }
    table_info->table_->GetTuple(*rid, tuple, exec_ctx_->GetTransaction());
    ++iter_begin_;
    return true;
  }
  if (txn->IsTableIntentionSharedLocked(table_info->oid_)) {
    try {
      if (txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED && !txn->GetSharedLockSet()->empty()) {
        if (!lkm->UnlockTable(txn, table_info->oid_)) {
          throw ExecutionException("index unlock table fail");
        }
      }
    } catch (TransactionAbortException &e) {
      throw ExecutionException("index execute lock fail");
    }
  }
  return false;
  //  if (iter_ != tree_->GetEndIterator()) {
  //    auto table_heap = exec_ctx_->GetCatalog()->GetTable(index_info_->table_name_)->table_.get();
  //    *rid = (*iter_).second;
  //    table_heap->GetTuple(*rid, tuple, exec_ctx_->GetTransaction());
  //    ++iter_;
  //    return true;
  //  }
  //  return false;
}

}  // namespace bustub
