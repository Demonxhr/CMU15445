//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {
// exec_ctx保存执行引擎需要的所有东西
SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan) {}

void SeqScanExecutor::Init() {
  // 获取表
  // ExecutorContext *exec_ctx = GetExecutorContext();
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid());
  txn_ = exec_ctx_->GetTransaction();
  lkm_ = exec_ctx_->GetLockManager();
  rvec_.clear();

  // 如果级别是读未提交  则不用加锁
  if (txn_->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED) {
      // 对于读提交和可重复读都是先加意向共享表锁 再加共享行锁
      // 如果加锁失败
      if (!lkm_->LockTable(txn_,LockManager::LockMode::INTENTION_SHARED,table_info_->oid_)) {
          txn_->SetState(TransactionState::ABORTED);
          throw Exception(ExceptionType::INVALID,"Cant lock table");
      }
  }

  table_heap_ = table_info_->table_.get();
  table_iterator_ = table_heap_->Begin(exec_ctx_->GetTransaction());
}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (table_iterator_ != table_heap_->End()) {
    *tuple = *table_iterator_;
    *rid = table_iterator_->GetRid();
    rvec_.push_back(*rid);
    if (txn_->GetIsolationLevel()!=IsolationLevel::READ_UNCOMMITTED) {
        if (!lkm_->LockRow(txn_,LockManager::LockMode::SHARED,table_info_->oid_,*rid)) {
            txn_->SetState(TransactionState::ABORTED);
            throw ExecutionException("Cant lock row");
        }
    }
    table_iterator_++;
    return true;
  }

  // 如果遍历完所有数据  在读提交时 需要释放所有锁 可重复读不释放锁
  if (txn_->GetIsolationLevel() == IsolationLevel::READ_COMMITTED) {
      for (auto &i : rvec_) {
        if (!lkm_->UnlockRow(txn_,table_info_->oid_,i)) {
            throw ExecutionException("cant unlock row");
        }
      }
      if (!lkm_->UnlockTable(txn_,table_info_->oid_)) {
          throw ExecutionException("cant unlock table");
      }
  }
  return false;
}

}  // namespace bustub
