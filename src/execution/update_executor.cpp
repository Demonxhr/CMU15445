//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {
  // As of Fall 2022, you DON'T need to implement update executor to have perfect score in project 3 / project 4.
}

void UpdateExecutor::Init() {
  child_executor_->Init();
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  auto txn = exec_ctx_->GetTransaction();
  auto lkm = exec_ctx_->GetLockManager();
  has_updated_ = false;

  if (!lkm->LockTable(txn, LockManager::LockMode::INTENTION_EXCLUSIVE, table_info_->oid_)) {
    // txn->SetState(TransactionState::ABORTED);
    throw Exception(ExceptionType::INVALID, "Cant lock table");
  }
}

auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (has_updated_) {
    return false;
  }
  auto txn = exec_ctx_->GetTransaction();
  auto lkm = exec_ctx_->GetLockManager();
  has_updated_ = true;
  int count = 0;
  auto schema = table_info_->schema_;
  while (child_executor_->Next(tuple, rid)) {
    try {
      if (!lkm->LockRow(txn, LockManager::LockMode::EXCLUSIVE, plan_->table_oid_, *rid)) {
        txn->SetState(TransactionState::ABORTED);
        throw ExecutionException("Cant lock row");
      }
    } catch (TransactionAbortException &e) {
      throw ExecutionException(e.GetInfo());
    }

    Tuple old_tuple;
    if (!table_info_->table_->GetTuple(*rid, &old_tuple, txn)) {
      LOG_ERROR("can't get old tuple");
      exit(-1);
    }
    if (table_info_->table_->UpdateTuple(*tuple, *rid, exec_ctx_->GetTransaction())) {
      count++;
      auto index_infos = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
      for (const auto &index_info : index_infos) {
        auto old_key =
            old_tuple.KeyFromTuple(table_info_->schema_, index_info->key_schema_, index_info->index_->GetKeyAttrs());
        index_info->index_->DeleteEntry(old_key, *rid, exec_ctx_->GetTransaction());
        auto key_tuple =
            tuple->KeyFromTuple(table_info_->schema_, index_info->key_schema_, index_info->index_->GetKeyAttrs());

        index_info->index_->InsertEntry(key_tuple, *rid, exec_ctx_->GetTransaction());
        txn->AppendIndexWriteRecord(IndexWriteRecord(*rid, plan_->table_oid_, WType::UPDATE, *tuple,
                                                     index_info->index_oid_, exec_ctx_->GetCatalog()));
      }
    }
  }
  std::vector<Value> result = {{TypeId::INTEGER, count}};
  *tuple = Tuple(result, &GetOutputSchema());
  return true;
}

}  // namespace bustub
