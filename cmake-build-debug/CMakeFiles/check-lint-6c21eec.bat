@echo off
cd /D C:\Users\34279\Desktop\leveldb\cmu15445\cmake-build-debug || (set FAIL_LINE=2& goto :ABORT)
echo 'C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/bind_create.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/bind_insert.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/bind_select.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/bind_variable.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/binder.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/bound_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/fmt_impl.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/keyword_helper.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/node_tag_to_string.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/create_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/delete_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/explain_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/index_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/insert_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/select_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/statement/update_statement.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/binder/transformer.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/buffer/buffer_pool_manager_instance.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/buffer/clock_replacer.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/buffer/lru_k_replacer.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/buffer/lru_replacer.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/catalog/column.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/catalog/schema.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/catalog/table_generator.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/cmake-build-debug/CMakeFiles/3.24.2/CompilerIdCXX/CMakeCXXCompilerId.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/common/bustub_instance.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/common/config.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/common/util/string_util.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/concurrency/lock_manager.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/concurrency/transaction_manager.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/container/disk/hash/disk_extendible_hash_table.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/container/disk/hash/linear_probe_hash_table.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/container/hash/extendible_hash_table.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/aggregation_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/delete_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/executor_factory.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/filter_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/fmt_impl.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/hash_join_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/index_scan_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/insert_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/limit_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/mock_scan_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/nested_index_join_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/nested_loop_join_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/plan_node.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/projection_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/seq_scan_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/sort_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/topn_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/update_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/execution/values_executor.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/binder.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/bound_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/bound_order_by.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/bound_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/bound_table_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_agg_call.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_alias.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_binary_op.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_column_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_constant.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_star.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/expressions/bound_unary_op.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/keyword_helper.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/simplified_token.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/create_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/delete_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/explain_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/index_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/insert_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/select_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/set_show_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/statement/update_statement.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/table_ref/bound_base_table_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/table_ref/bound_cross_product_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/table_ref/bound_cte_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/table_ref/bound_expression_list_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/table_ref/bound_join_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/table_ref/bound_subquery_ref.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/binder/tokens.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/buffer/buffer_pool_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/buffer/buffer_pool_manager_instance.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/buffer/clock_replacer.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/buffer/lru_k_replacer.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/buffer/lru_replacer.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/buffer/replacer.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/catalog/catalog.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/catalog/column.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/catalog/schema.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/catalog/table_generator.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/bustub_instance.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/config.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/enums/statement_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/exception.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/hash_util.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/logger.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/macros.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/rid.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/rwlatch.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/util/hash_util.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/common/util/string_util.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/concurrency/lock_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/concurrency/transaction.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/concurrency/transaction_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/container/disk/hash/disk_extendible_hash_table.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/container/disk/hash/disk_hash_table.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/container/disk/hash/linear_probe_hash_table.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/container/hash/extendible_hash_table.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/container/hash/hash_function.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/container/hash/hash_table.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/execution_engine.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executor_context.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executor_factory.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/abstract_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/aggregation_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/delete_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/filter_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/hash_join_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/index_scan_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/insert_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/limit_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/mock_scan_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/nested_index_join_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/nested_loop_join_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/projection_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/seq_scan_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/sort_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/topn_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/update_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/executors/values_executor.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/expressions/abstract_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/expressions/arithmetic_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/expressions/column_value_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/expressions/comparison_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/expressions/constant_value_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/expressions/logic_expression.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/abstract_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/aggregation_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/delete_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/filter_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/hash_join_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/index_scan_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/insert_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/limit_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/mock_scan_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/nested_index_join_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/nested_loop_join_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/projection_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/seq_scan_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/sort_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/topn_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/update_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/execution/plans/values_plan.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/optimizer/optimizer.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/planner/planner.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/primer/p0_trie.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/recovery/checkpoint_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/recovery/log_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/recovery/log_record.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/recovery/log_recovery.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/disk/disk_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/disk/disk_manager_memory.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/b_plus_tree.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/b_plus_tree_index.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/extendible_hash_table_index.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/generic_key.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/hash_comparator.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/index.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/index_iterator.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/int_comparator.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/linear_probe_hash_table_index.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/index/stl_comparator_wrapper.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/b_plus_tree_internal_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/b_plus_tree_leaf_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/b_plus_tree_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/hash_table_block_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/hash_table_bucket_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/hash_table_directory_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/hash_table_header_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/hash_table_page_defs.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/header_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/table_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/page/tmp_tuple_page.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/table/table_heap.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/table/table_iterator.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/table/tmp_tuple.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/storage/table/tuple.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/abstract_pool.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/bigint_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/boolean_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/decimal_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/integer_parent_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/integer_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/limits.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/numeric_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/smallint_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/timestamp_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/tinyint_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/type_id.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/type_util.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/value.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/value_factory.h C:/Users/34279/Desktop/leveldb/cmu15445/src/include/type/varlen_type.h C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/eliminate_true_filter.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/merge_filter_nlj.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/merge_filter_scan.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/merge_projection.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/nlj_as_hash_join.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/nlj_as_index_join.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/optimizer.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/optimizer_custom_rules.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/order_by_index_scan.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/optimizer/sort_limit_as_topn.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/expression_factory.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/plan_aggregation.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/plan_expression.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/plan_insert.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/plan_select.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/plan_table_ref.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/planner/planner.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/primer/p0_trie.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/recovery/checkpoint_manager.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/recovery/log_manager.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/recovery/log_recovery.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/disk/disk_manager.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/disk/disk_manager_memory.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/index/b_plus_tree.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/index/b_plus_tree_index.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/index/extendible_hash_table_index.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/index/index_iterator.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/index/linear_probe_hash_table_index.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/b_plus_tree_internal_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/b_plus_tree_leaf_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/b_plus_tree_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/hash_table_block_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/hash_table_bucket_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/hash_table_directory_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/hash_table_header_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/header_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/page/table_page.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/table/table_heap.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/table/table_iterator.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/storage/table/tuple.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/bigint_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/boolean_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/decimal_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/integer_parent_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/integer_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/smallint_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/timestamp_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/tinyint_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/value.cpp C:/Users/34279/Desktop/leveldb/cmu15445/src/type/varlen_type.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/binder/binder_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/buffer/buffer_pool_manager_instance_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/buffer/clock_replacer_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/buffer/counter.h C:/Users/34279/Desktop/leveldb/cmu15445/test/buffer/lru_k_replacer_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/buffer/lru_replacer_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/buffer/mock_buffer_pool_manager.h C:/Users/34279/Desktop/leveldb/cmu15445/test/catalog/catalog_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/common/rwlatch_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/concurrency/deadlock_detection_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/concurrency/lock_manager_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/concurrency/transaction_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/container/disk/hash/hash_table_page_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/container/disk/hash/hash_table_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/container/hash/extendible_hash_table_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/include/logging/common.h C:/Users/34279/Desktop/leveldb/cmu15445/test/include/test_util.h C:/Users/34279/Desktop/leveldb/cmu15445/test/primer/starter_trie_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/recovery/recovery_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/storage/b_plus_tree_concurrent_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/storage/b_plus_tree_contention_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/storage/b_plus_tree_delete_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/storage/b_plus_tree_insert_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/storage/disk_manager_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/storage/tmp_tuple_page_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/table/tuple_test.cpp C:/Users/34279/Desktop/leveldb/cmu15445/test/type/type_test.cpp' | xargs -n12 -P8 C:/Users/34279/Desktop/leveldb/cmu15445/build_support/cpplint.py --verbose=2 --quiet --linelength=120 --filter=-legal/copyright,-build/header_guard,-runtime/references || (set FAIL_LINE=3& goto :ABORT)
goto :EOF

:ABORT
set ERROR_CODE=%ERRORLEVEL%
echo Batch file failed at line %FAIL_LINE% with errorcode %ERRORLEVEL%
exit /b %ERROR_CODE%