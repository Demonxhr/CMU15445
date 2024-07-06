//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  child_executor_ = std::move(child_executor);
}

void InsertExecutor::Init() {
  child_executor_->Init();
  has_inserted_ = false;
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());

  index_infos_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
}

auto InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (has_inserted_) {
    return false;
  }
  has_inserted_ = true;
  int count = 0;
  // auto table_info = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  auto schema = table_info_->schema_;
  // auto index = exec_ctx_->GetCatalog()->GetTableIndexes(table_info->name_);
  while (child_executor_->Next(tuple, rid)) {
    count++;
    table_info_->table_->InsertTuple(*tuple, rid, exec_ctx_->GetTransaction());
    for (const auto &index_info : index_infos_) {
      auto key_tuple = tuple->KeyFromTuple(schema, index_info->key_schema_, index_info->index_->GetKeyAttrs());
      index_info->index_->InsertEntry(key_tuple, *rid, exec_ctx_->GetTransaction());
    }
  }
  std::vector<Value> result = {{TypeId::INTEGER, count}};
  *tuple = Tuple(result, &GetOutputSchema());
  return true;
}

}  // namespace bustub
