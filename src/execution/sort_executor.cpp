#include "execution/executors/sort_executor.h"

namespace bustub {

SortExecutor::SortExecutor(ExecutorContext *exec_ctx, const SortPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void SortExecutor::Init() {
  child_executor_->Init();
  sort_tuples_.clear();
  Tuple tuple{};
  RID rid{};
  while (child_executor_->Next(&tuple, &rid)) {
    sort_tuples_.push_back(std::move(tuple));
  }

  std::sort(sort_tuples_.begin(), sort_tuples_.end(), [this](const Tuple &a, const Tuple &b) {
    const auto &schema = child_executor_->GetOutputSchema();
    for (auto [order_by_type, expr] : plan_->GetOrderBy()) {
      Value val_a = expr->Evaluate(&a, schema);
      Value val_b = expr->Evaluate(&b, schema);
      if (val_a.CompareEquals(val_b) == CmpBool::CmpTrue) {
        continue;
      }
      if (order_by_type == OrderByType::ASC || order_by_type == OrderByType::DEFAULT) {
        return val_a.CompareLessThan(val_b) == CmpBool::CmpTrue;
      }
      return val_a.CompareGreaterThan(val_b) == CmpBool::CmpTrue;
    }
    return false;
  });

  iter_ = sort_tuples_.begin();
}

auto SortExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (iter_ == sort_tuples_.end()) {
    return false;
  }
  *tuple = *iter_;
  ++iter_;
  return true;
}

}  // namespace bustub
