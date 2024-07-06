#include "execution/executors/topn_executor.h"

namespace bustub {

TopNExecutor::TopNExecutor(ExecutorContext *exec_ctx, const TopNPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void TopNExecutor::Init() {
  child_executor_->Init();
  auto order_by = plan_->GetOrderBy();
  std::function<bool(Tuple, Tuple)> cmp = [this](const Tuple &a, const Tuple &b) {
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
  };
  std::priority_queue<Tuple, std::vector<Tuple>, std::function<bool(Tuple, Tuple)>> heap(cmp);
  Tuple tuple;
  RID rid;
  while (child_executor_->Next(&tuple, &rid)) {
    heap.push(tuple);
    if (heap.size() > plan_->GetN()) {
      heap.pop();
    }
  }
  while (!heap.empty()) {
    topn_tuples_.push(heap.top());
    heap.pop();
  }
}

auto TopNExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (topn_tuples_.empty()) {
    return false;
  }
  *tuple = topn_tuples_.top();
  topn_tuples_.pop();
  return true;
}

}  // namespace bustub
