/**
 * deadlock_detection_test.cpp
 */

#include <atomic>
#include <random>
#include <thread>  // NOLINT

#include "common/config.h"
#include "concurrency/lock_manager.h"
#include "concurrency/transaction_manager.h"
#include "gtest/gtest.h"
#define TEST_TIMEOUT_BEGIN                           \
  std::promise<bool> promisedFinished;               \
  auto futureResult = promisedFinished.get_future(); \
                              std::thread([](std::promise<bool>& finished) {
#define TEST_TIMEOUT_FAIL_END(X)                                                                  \
  finished.set_value(true);                                                                       \
  }, std::ref(promisedFinished)).detach();                                                        \
  EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(X)) != std::future_status::timeout) \
      << "Test Failed Due to Time Out";

namespace bustub {
TEST(LockManagerDeadlockDetectionTest, DISABLED_EdgeTest) {
  LockManager lock_mgr{};

  const int num_nodes = 100;
  const int num_edges = num_nodes / 2;
  const int seed = 15445;
  std::srand(seed);

  // Create txn ids and shuffle
  std::vector<txn_id_t> txn_ids;
  txn_ids.reserve(num_nodes);
  for (int i = 0; i < num_nodes; i++) {
    txn_ids.push_back(i);
  }
  EXPECT_EQ(num_nodes, txn_ids.size());
  auto rng = std::default_random_engine{};
  std::shuffle(txn_ids.begin(), txn_ids.end(), rng);
  EXPECT_EQ(num_nodes, txn_ids.size());

  // Create edges by pairing adjacent txn_ids
  std::vector<std::pair<txn_id_t, txn_id_t>> edges;
  for (int i = 0; i < num_nodes; i += 2) {
    EXPECT_EQ(i / 2, lock_mgr.GetEdgeList().size());
    auto t1 = txn_ids[i];
    auto t2 = txn_ids[i + 1];
    lock_mgr.AddEdge(t1, t2);
    edges.emplace_back(t1, t2);
    EXPECT_EQ((i / 2) + 1, lock_mgr.GetEdgeList().size());
  }

  auto lock_mgr_edges = lock_mgr.GetEdgeList();
  EXPECT_EQ(num_edges, lock_mgr_edges.size());
  EXPECT_EQ(num_edges, edges.size());

  std::sort(lock_mgr_edges.begin(), lock_mgr_edges.end());
  std::sort(edges.begin(), edges.end());

  for (int i = 0; i < num_edges; i++) {
    EXPECT_EQ(edges[i], lock_mgr_edges[i]);
  }
}

TEST(LockManagerDeadlockDetectionTest, DISABLED_BasicDeadlockDetectionTest) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};

  table_oid_t toid{0};
  RID rid0{0, 0};
  RID rid1{1, 1};
  auto *txn0 = txn_mgr.Begin();
  auto *txn1 = txn_mgr.Begin();
  EXPECT_EQ(0, txn0->GetTransactionId());
  EXPECT_EQ(1, txn1->GetTransactionId());

  std::thread t0([&] {
    // Lock and sleep
    bool res = lock_mgr.LockTable(txn0, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(true, res);
    res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid0);
    EXPECT_EQ(true, res);
    EXPECT_EQ(TransactionState::GROWING, txn1->GetState());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // This will block
    res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid1);
    EXPECT_EQ(true, res);

    lock_mgr.UnlockRow(txn0, toid, rid1);
    lock_mgr.UnlockRow(txn0, toid, rid0);
    lock_mgr.UnlockTable(txn0, toid);

    txn_mgr.Commit(txn0);
    EXPECT_EQ(TransactionState::COMMITTED, txn0->GetState());
  });

  std::thread t1([&] {
    // Sleep so T0 can take necessary locks
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool res = lock_mgr.LockTable(txn1, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
    EXPECT_EQ(res, true);

    res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid1);
    EXPECT_EQ(TransactionState::GROWING, txn1->GetState());

    // This will block
    res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid0);
    EXPECT_EQ(res, false);

    EXPECT_EQ(TransactionState::ABORTED, txn1->GetState());
    txn_mgr.Abort(txn1);
  });

  // Sleep for enough time to break cycle
  std::this_thread::sleep_for(cycle_detection_interval * 2);

  t0.join();
  t1.join();

  delete txn0;
  delete txn1;
}

TEST(LockManagerDeadlockDetectionTest, GraphTest) {
  LockManager lock_mgr{};
  TransactionManager txn_mgr{&lock_mgr};

  //        table_oid_t toid{0};
  //        RID rid0{0, 0};
  //        RID rid1{1, 1};
  //        RID rid2{2, 2};
  auto *txn0 = txn_mgr.Begin();
  auto *txn1 = txn_mgr.Begin();
  auto *txn2 = txn_mgr.Begin();
  auto *txn3 = txn_mgr.Begin();
  auto *txn4 = txn_mgr.Begin();

  EXPECT_EQ(0, txn0->GetTransactionId());
  EXPECT_EQ(1, txn1->GetTransactionId());
  EXPECT_EQ(2, txn2->GetTransactionId());
  EXPECT_EQ(3, txn3->GetTransactionId());
  EXPECT_EQ(4, txn4->GetTransactionId());

  lock_mgr.AddEdge(txn0->GetTransactionId(), txn1->GetTransactionId());
  lock_mgr.AddEdge(txn1->GetTransactionId(), txn0->GetTransactionId());

  EXPECT_EQ(2, lock_mgr.GetEdgeList().size());

  lock_mgr.AddEdge(txn2->GetTransactionId(), txn3->GetTransactionId());
  lock_mgr.AddEdge(txn3->GetTransactionId(), txn4->GetTransactionId());
  lock_mgr.AddEdge(txn4->GetTransactionId(), txn2->GetTransactionId());

  txn_id_t txn;
  lock_mgr.HasCycle(&txn);
  EXPECT_EQ(1, txn);

  //        std::thread t0([&] {
  //            // Lock and sleep
  //            bool res = lock_mgr.LockTable(txn0, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
  //            EXPECT_EQ(true, res);
  //            res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid0);
  //            EXPECT_EQ(true, res);
  //            // EXPECT_EQ(TransactionState::GROWING, txn1->GetState());
  //            std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //
  //            // This will block
  //            res = lock_mgr.LockRow(txn0, LockManager::LockMode::EXCLUSIVE, toid, rid1);
  //            EXPECT_EQ(true, res);
  //
  //            lock_mgr.UnlockRow(txn0, toid, rid1);
  //            lock_mgr.UnlockRow(txn0, toid, rid0);
  //            lock_mgr.UnlockTable(txn0, toid);
  //
  //            txn_mgr.Commit(txn0);
  //            EXPECT_EQ(TransactionState::COMMITTED, txn0->GetState());
  //        });
  //
  //        std::thread t1([&] {
  //            std::this_thread::sleep_for(std::chrono::milliseconds(50));
  //            // Lock and sleep
  //            bool res = lock_mgr.LockTable(txn1, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
  //            EXPECT_EQ(true, res);
  //            res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid1);
  //            EXPECT_EQ(true, res);
  //            // EXPECT_EQ(TransactionState::GROWING, txn1->GetState());
  //            std::this_thread::sleep_for(std::chrono::milliseconds(200));
  //
  //            // This will block
  //            res = lock_mgr.LockRow(txn1, LockManager::LockMode::EXCLUSIVE, toid, rid2);
  //            EXPECT_EQ(true, res);
  //
  //            lock_mgr.UnlockRow(txn1, toid, rid1);
  //            lock_mgr.UnlockRow(txn1, toid, rid0);
  //            lock_mgr.UnlockTable(txn1, toid);
  //
  //            txn_mgr.Commit(txn1);
  //            EXPECT_EQ(TransactionState::COMMITTED, txn1->GetState());
  //        });
  //
  //        std::thread t2([&] {
  //            // Sleep so T0 can take necessary locks
  //            std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //            bool res = lock_mgr.LockTable(txn2, LockManager::LockMode::INTENTION_EXCLUSIVE, toid);
  //            EXPECT_EQ(res, true);
  //
  //            res = lock_mgr.LockRow(txn2, LockManager::LockMode::EXCLUSIVE, toid, rid2);
  //            EXPECT_EQ(TransactionState::GROWING, txn2->GetState());
  //
  //            // This will block
  //            res = lock_mgr.LockRow(txn2, LockManager::LockMode::EXCLUSIVE, toid, rid0);
  //            EXPECT_EQ(res, false);
  //
  //            EXPECT_EQ(TransactionState::ABORTED, txn2->GetState());
  //
  //
  //        });
  //
  //        // Sleep for enough time to break cycle
  //        std::this_thread::sleep_for(cycle_detection_interval * 2);
  //
  //        t0.join();
  //        t1.join();
  //        t2.join();
  // EXPECT_EQ(2,txn_mgr.txn_map.size());
  delete txn0;
  delete txn1;
  delete txn2;
  delete txn3;
  delete txn4;
}
}  // namespace bustub
