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
  ExecutorContext *exec_ctx = GetExecutorContext();
  table_info_ = exec_ctx->GetCatalog()->GetTable(plan_->GetTableOid());
  table_heap_ = table_info_->table_.get();
  table_iterator_ = table_heap_->Begin(exec_ctx->GetTransaction());
}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (table_iterator_ != table_heap_->End()) {
    *tuple = *table_iterator_;
    *rid = table_iterator_->GetRid();
    table_iterator_++;
    return true;
  }
  return false;
}

}  // namespace bustub
