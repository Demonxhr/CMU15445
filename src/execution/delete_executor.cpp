//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/delete_executor.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  child_executor_ = std::move(child_executor);
}

void DeleteExecutor::Init() {
  child_executor_->Init();
  has_inserted_ = false;
  auto txn = exec_ctx_->GetTransaction();
  auto lkm = exec_ctx_->GetLockManager();

  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());

  if (!lkm->LockTable(txn, LockManager::LockMode::INTENTION_EXCLUSIVE, table_info_->oid_)) {
    txn->SetState(TransactionState::ABORTED);
    throw Exception(ExceptionType::INVALID, "Cant lock table");
  }
}

auto DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (has_inserted_) {
    return false;
  }
  auto txn = exec_ctx_->GetTransaction();
  auto lkm = exec_ctx_->GetLockManager();
  has_inserted_ = true;
  int count = 0;
  // auto table_info = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  auto schema = table_info_->schema_;
  // auto index = exec_ctx_->GetCatalog()->GetTableIndexes(table_info->name_);
  while (child_executor_->Next(tuple, rid)) {
    try {
      if (!lkm->LockRow(txn, LockManager::LockMode::EXCLUSIVE, plan_->table_oid_, *rid)) {
        txn->SetState(TransactionState::ABORTED);
        throw ExecutionException("Cant lock row");
      }
    } catch (TransactionAbortException &e) {
      throw ExecutionException(e.GetInfo());
    }
    if (table_info_->table_->MarkDelete(*rid, exec_ctx_->GetTransaction())) {
      count++;
      auto index_infos = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
      for (const auto &index_info : index_infos) {
        auto key_tuple =
            tuple->KeyFromTuple(table_info_->schema_, index_info->key_schema_, index_info->index_->GetKeyAttrs());
        index_info->index_->DeleteEntry(key_tuple, *rid, exec_ctx_->GetTransaction());
        txn->AppendIndexWriteRecord(IndexWriteRecord(*rid, plan_->table_oid_, WType::DELETE, *tuple,
                                                     index_info->index_oid_, exec_ctx_->GetCatalog()));
      }
    }
  }
  std::vector<Value> result = {{TypeId::INTEGER, count}};
  *tuple = Tuple(result, &GetOutputSchema());
  return true;
}

}  // namespace bustub
