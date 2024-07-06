//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_index_join_executor.cpp
//
// Identification: src/execution/nested_index_join_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_index_join_executor.h"

namespace bustub {

NestIndexJoinExecutor::NestIndexJoinExecutor(ExecutorContext *exec_ctx, const NestedIndexJoinPlanNode *plan,
                                             std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), left_executor_(std::move(child_executor)) {
  if (!(plan->GetJoinType() == JoinType::LEFT || plan->GetJoinType() == JoinType::INNER)) {
    // Note for 2022 Fall: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
}

void NestIndexJoinExecutor::Init() {
  left_executor_->Init();
  IndexInfo *index_info = exec_ctx_->GetCatalog()->GetIndex(plan_->index_oid_);
  auto tree = reinterpret_cast<BPlusTreeIndexForOneIntegerColumn *>(index_info->index_.get());
  // auto iter_ = tree_->GetBeginIterator();
  Tuple tuple{};
  RID rid{};
  while (left_executor_->Next(&tuple, &rid)) {
    Value left_key_value = plan_->KeyPredicate()->Evaluate(&tuple, left_executor_->GetOutputSchema());
    Tuple left_key_tuple = Tuple(std::vector<Value>{left_key_value}, &index_info->key_schema_);
    std::vector<RID> result_rids;
    tree->ScanKey(left_key_tuple, &result_rids, nullptr);
    std::vector<Tuple> right_tuples;
    auto table_heap = exec_ctx_->GetCatalog()->GetTable(index_info->table_name_)->table_.get();
    for (auto &right_rid : result_rids) {
      Tuple t_right_tuple;
      table_heap->GetTuple(right_rid, &t_right_tuple, exec_ctx_->GetTransaction());
      right_tuples.push_back(t_right_tuple);
    }
    if (!right_tuples.empty()) {
      for (auto &right_tuple : right_tuples) {
        std::vector<Value> values;
        for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(tuple.GetValue(&left_executor_->GetOutputSchema(), i));
        }
        for (uint32_t i = 0; i < plan_->InnerTableSchema().GetColumnCount(); ++i) {
          values.emplace_back(right_tuple.GetValue(&plan_->InnerTableSchema(), i));
        }
        results_.emplace(values, &GetOutputSchema());
      }
    } else {
      if (plan_->GetJoinType() == JoinType::LEFT) {
        std::vector<Value> values;
        for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(tuple.GetValue(&left_executor_->GetOutputSchema(), i));
        }
        for (uint32_t i = 0; i < plan_->InnerTableSchema().GetColumnCount(); ++i) {
          values.emplace_back(ValueFactory::GetNullValueByType(plan_->InnerTableSchema().GetColumn(i).GetType()));
        }
        results_.emplace(values, &GetOutputSchema());
      }
    }
  }
}

auto NestIndexJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (results_.empty()) {
    return false;
  }
  *tuple = results_.front();
  results_.pop();
  return true;
}

}  // namespace bustub
