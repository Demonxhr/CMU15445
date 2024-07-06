//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"
#include "binder/table_ref/bound_join_ref.h"
#include "common/exception.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      left_executor_(std::move(left_executor)),
      right_executor_(std::move(right_executor)) {
  if (!(plan->GetJoinType() == JoinType::LEFT || plan->GetJoinType() == JoinType::INNER)) {
    // Note for 2022 Fall: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
}

void NestedLoopJoinExecutor::Init() {
  left_executor_->Init();
  right_executor_->Init();
  Tuple tuple;
  RID rid;
  while (left_executor_->Next(&tuple, &rid)) {
    left_tuples_.emplace_back(std::move(tuple));
  }
  while (right_executor_->Next(&tuple, &rid)) {
    right_tuples_.emplace_back(std::move(tuple));
  }
  //    left_bool_ = left_executor_->Next(&left_tuple_, &left_rid_);
  //    has_done_ = false;
  for (auto &left_tuple : left_tuples_) {
    bool has_done = false;
    for (auto &right_tuple : right_tuples_) {
      auto join_result = plan_->Predicate().EvaluateJoin(&left_tuple, left_executor_->GetOutputSchema(), &right_tuple,
                                                         right_executor_->GetOutputSchema());
      if (join_result.GetAs<bool>()) {
        std::vector<Value> values;
        for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(left_tuple.GetValue(&left_executor_->GetOutputSchema(), i));
        }
        for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(right_tuple.GetValue(&right_executor_->GetOutputSchema(), i));
        }
        results_.emplace(values, &GetOutputSchema());
        has_done = true;
      }
    }
    if (!has_done) {
      if (plan_->GetJoinType() == JoinType::LEFT) {
        std::vector<Value> values;
        for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(left_tuple.GetValue(&left_executor_->GetOutputSchema(), i));
        }
        for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(
              ValueFactory::GetNullValueByType(right_executor_->GetOutputSchema().GetColumn(i).GetType()));
        }
        results_.emplace(values, &GetOutputSchema());
      }
    }
  }
}

auto NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (results_.empty()) {
    return false;
  }
  *tuple = results_.front();
  results_.pop();
  return true;
  //        int size = left_tuples_.size();
  //        while(index < size) {
  //            for (auto &right_tuple: right_tuples_) {
  //                // 判断连接条件
  //                auto join_result = plan_->Predicate().EvaluateJoin(&left_tuples_[index],
  //                left_executor_->GetOutputSchema(),
  //                                                                   &right_tuple,
  //                                                                   right_executor_->GetOutputSchema());
  //                if (join_result.GetAs<bool>()) {
  //                    std::vector<Value> values;
  //                    for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
  //                        values.emplace_back(left_tuples_[index].GetValue(&left_executor_->GetOutputSchema(), i));
  //                    }
  //                    for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
  //                        values.emplace_back(right_tuple.GetValue(&right_executor_->GetOutputSchema(), i));
  //                    }
  //                    *tuple = Tuple{values,&GetOutputSchema()};
  //                    ++index;
  //                    return true;
  //                }
  //            }
  //            if (plan_->GetJoinType() == JoinType::LEFT) {
  //                std::vector<Value> values;
  //                for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
  //                    values.emplace_back(left_tuples_[index].GetValue(&left_executor_->GetOutputSchema(), i));
  //                }
  //                for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
  //                    values.emplace_back(ValueFactory::GetNullValueByType(right_executor_->GetOutputSchema().GetColumn(i).GetType()));
  //                }
  //                *tuple = Tuple{values,&GetOutputSchema()};
  //                ++index;
  //                return true;
  //            }
  //            ++index;
  //        }
  //
  //    return false;
}
// auto NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
//    Tuple right_tuple{};
//    RID right_rid{};
//    auto &predicate = plan_->Predicate();
//    // 左连接
//    if (plan_->GetJoinType() == JoinType::LEFT) {
//        while (left_bool_) {
//            while (right_executor_->Next(&right_tuple, &right_rid)) {
//                // 判断连接条件
//                auto join_result = predicate.EvaluateJoin(&left_tuple_, left_executor_->GetOutputSchema(),
//                                                          &right_tuple, right_executor_->GetOutputSchema());
//                if (join_result.GetAs<bool>()) {
//                    std::vector<Value> values;
//                    for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
//                        values.emplace_back(left_tuple_.GetValue(&left_executor_->GetOutputSchema(), i));
//                    }
//                    for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
//                        values.emplace_back(right_tuple.GetValue(&right_executor_->GetOutputSchema(), i));
//                    }
//                    *tuple = Tuple{values,&GetOutputSchema()};
//                    has_done_ = true;
//                    return true;
//                }
//
//            }
//            if (!has_done_) {
//                std::vector<Value> values;
//                for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
//                    values.emplace_back(left_tuple_.GetValue(&left_executor_->GetOutputSchema(), i));
//                }
//                for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
//                    values.emplace_back(ValueFactory::GetNullValueByType(right_executor_->GetOutputSchema().GetColumn(i).GetType()));
//                }
//                *tuple = Tuple{values,&GetOutputSchema()};
//                has_done_ = true;
//                return true;
//            }
//            left_bool_ = left_executor_->Next(&left_tuple_, &left_rid_);
//            right_executor_->Init();
//            has_done_ = false;
//        }
//        return false;
//    }
//    // 内连接
//    if (plan_->GetJoinType() == JoinType::INNER) {
//        while (left_bool_) {
//            while (right_executor_->Next(&right_tuple, &right_rid)) {
//                // 判断连接条件
//                auto join_result = predicate.EvaluateJoin(&left_tuple_, left_executor_->GetOutputSchema(),
//                                                          &right_tuple, right_executor_->GetOutputSchema());
//                if (join_result.GetAs<bool>()) {
//                    std::vector<Value> values;
//                    for (uint32_t i = 0; i < left_executor_->GetOutputSchema().GetColumnCount(); ++i) {
//                        values.emplace_back(left_tuple_.GetValue(&left_executor_->GetOutputSchema(), i));
//                    }
//                    for (uint32_t i = 0; i < right_executor_->GetOutputSchema().GetColumnCount(); ++i) {
//                        values.emplace_back(right_tuple.GetValue(&right_executor_->GetOutputSchema(), i));
//                    }
//                    *tuple = Tuple{values, &GetOutputSchema()};
//                    has_done_ = true;
//                    return true;
//                }
//            }
//            left_bool_ = left_executor_->Next(&left_tuple_, &left_rid_);
//            right_executor_->Init();
//            has_done_ = false;
//        }
//    }
//    return false;
//}
}  // namespace bustub
