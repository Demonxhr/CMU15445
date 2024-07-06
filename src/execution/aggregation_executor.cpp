//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>
#include <vector>

#include "execution/executors/aggregation_executor.h"

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child)
    : AbstractExecutor(exec_ctx), plan_(plan), child_(std::move(child)) {}

void AggregationExecutor::Init() {
  child_->Init();
  // 获取聚合方式
  auto agg_exprs = plan_->GetAggregates();
  // 获取聚合类型
  auto agg_types = plan_->GetAggregateTypes();
  aht_ = std::make_unique<SimpleAggregationHashTable>(plan_->GetAggregates(), plan_->GetAggregateTypes());
  Tuple tuple{};
  RID rid{};
  while (child_->Next(&tuple, &rid)) {
    auto agg_key = MakeAggregateKey(&tuple);
    auto agg_value = MakeAggregateValue(&tuple);
    // 插入hash表
    aht_->InsertCombine(agg_key, agg_value);
  }
  aht_iterator_ = std::make_unique<SimpleAggregationHashTable::Iterator>(aht_->Begin());
  has_aggregation_ = false;
}

auto AggregationExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (aht_->Begin() != aht_->End()) {
    if (*aht_iterator_ == aht_->End()) {
      return false;
    }
    // 获取聚合key和value
    auto agg_key = aht_iterator_->Key();
    auto agg_value = aht_iterator_->Val();

    std::vector<Value> values;
    // 根据文件要求，有groupby和aggregate两个部分的情况下，groupby也要算上，都添加到value中
    values.reserve(agg_key.group_bys_.size() + agg_value.aggregates_.size());
    for (auto &group_by : agg_key.group_bys_) {
      values.emplace_back(group_by);
    }
    for (auto &aggregate : agg_value.aggregates_) {
      values.emplace_back(aggregate);
    }
    *tuple = {values, &GetOutputSchema()};
    ++*aht_iterator_;
    return true;
  }
  if (has_aggregation_) {
    return false;
  }
  has_aggregation_ = true;

  if (plan_->GetGroupBys().empty()) {
    std::vector<Value> values;
    Tuple tuple_buffer{};
    for (auto &agg_value : aht_->GenerateInitialAggregateValue().aggregates_) {
      values.emplace_back(agg_value);
    }
    *tuple = {values, &GetOutputSchema()};
    return true;
  }
  return false;
}

auto AggregationExecutor::GetChildExecutor() const -> const AbstractExecutor * { return child_.get(); }
}  // namespace bustub
