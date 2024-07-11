//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.cpp
//
// Identification: src/concurrency/lock_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "concurrency/lock_manager.h"

#include "common/config.h"
#include "concurrency/transaction.h"
#include "concurrency/transaction_manager.h"

namespace bustub {

auto LockManager::LockTable(Transaction *txn, LockMode lock_mode, const table_oid_t &oid) -> bool {
    // std::cout << "lock: " << oid << std::endl;
    // 检查是否可以加锁
    CheckLock(txn, lock_mode, LockObject::TABLE);
    // 锁住map
    table_lock_map_latch_.lock();
    auto lock_request_queue_it = table_lock_map_.find(oid);
    // 及时释放锁提高并发性
    table_lock_map_latch_.unlock();
    // 没有请求队列，则生成一个
    if (lock_request_queue_it == table_lock_map_.end()) {
        table_lock_map_latch_.lock();
        table_lock_map_.emplace(oid, new LockRequestQueue());
        //lock_request_queue_it = table_lock_map_.find(oid);

        LockRequest *lock_request = new LockRequest(txn->GetTransactionId(), lock_mode, oid);
        lock_request->granted_ = true;
        table_lock_map_[oid]->request_queue_.push_back(lock_request);
        table_lock_map_latch_.unlock();
        ModifyLockSet(txn,oid,lock_mode,LockObject::TABLE,ModifyMode::ADD);
        return true;
    }
    auto request_queue = lock_request_queue_it->second;
    request_queue->latch_.lock();
    for (auto lock_request_iter = request_queue->request_queue_.begin(); lock_request_iter != request_queue->request_queue_.end(); ++lock_request_iter) {
        auto lock_request1 = *lock_request_iter;
        if (txn->GetTransactionId() == lock_request1->txn_id_) {
            // 队列中存在和当前事务id相等的记录 可能会发生锁升级
            // 存在相同的申请记录，这个事务肯定被授予了锁，不然会导致事务阻塞在该函数中不会返回

            // 被授予的锁和当前锁类型相同，则直接返回
            if (lock_request1->lock_mode_ == lock_mode) {
                request_queue->latch_.unlock();
                return true;
            }
            // 存在别的锁在升级 抛异常  不允许多个事务同时升级锁 抛升级冲突异常
            if (request_queue->upgrading_ != INVALID_TXN_ID) {
                request_queue->latch_.unlock();
                txn->SetState(TransactionState::ABORTED);
                throw TransactionAbortException(txn->GetTransactionId(), AbortReason::UPGRADE_CONFLICT);
            }

            
            request_queue->upgrading_ = txn->GetTransactionId();
            // request_queue->latch_.unlock();
            // UnlockTable(txn,oid);
            // request_queue->latch_.lock();
            // 升级锁时，先释放之前的锁
            ModifyLockSet(txn,oid,lock_request1->lock_mode_,LockObject::TABLE,ModifyMode::DELETE);
            delete (*lock_request_iter);
            request_queue->request_queue_.erase(lock_request_iter);

            LockRequest *lock_request = new LockRequest(txn->GetTransactionId(), lock_mode, oid);
            request_queue->request_queue_.push_back(lock_request);
            lock_request_iter = std::prev(request_queue->request_queue_.end());
            std::unique_lock<std::mutex> lock(request_queue->latch_, std::adopt_lock);
            while (!CheckGrant(lock_request, request_queue)) {
                request_queue->cv_.wait(lock);
                if (txn->GetState() == TransactionState::ABORTED) {
                    delete lock_request;
                    request_queue->request_queue_.erase(lock_request_iter);
                    request_queue->upgrading_ = INVALID_TXN_ID;
                    request_queue->cv_.notify_all();
                    return false;
                }
            }
            request_queue->upgrading_ = INVALID_TXN_ID;
            lock_request->granted_ = true;
            ModifyLockSet(txn,oid,lock_mode,LockObject::TABLE,ModifyMode::ADD);
            if (lock_mode!=LockMode::EXCLUSIVE) {
                request_queue->cv_.notify_all();
            }
            return true;
        }
    }
    request_queue->latch_.unlock();
    LockRequest *lock_request = new LockRequest(txn->GetTransactionId(), lock_mode, oid);
    request_queue->latch_.lock();
    request_queue->request_queue_.push_back(lock_request);
    auto lock_request_iter = std::prev(request_queue->request_queue_.end());
    std::unique_lock<std::mutex> lock(request_queue->latch_, std::adopt_lock);
    while (!CheckGrant(lock_request, request_queue)) {
        request_queue->cv_.wait(lock);
        if (txn->GetState() == TransactionState::ABORTED) {
            delete lock_request;
            request_queue->request_queue_.erase(lock_request_iter);
            // request_queue->upgrading_ = INVALID_TXN_ID;
            request_queue->cv_.notify_all();
            return false;
        }
    }
    //request_queue->cv_.wait(lock,[&](){return CheckGrant(lock_request, request_queue);});
    ModifyLockSet(txn,oid,lock_mode,LockObject::TABLE,ModifyMode::ADD);
    lock_request->granted_ = true;
    return true;
}

auto LockManager::UnlockTable(Transaction *txn, const table_oid_t &oid) -> bool {
    // std::cout << "unlock: " << oid << std::endl;
    table_lock_map_latch_.lock();
    auto lock_request_queue_it = table_lock_map_.find(oid);
    table_lock_map_latch_.unlock();
    // 释放不存在的锁
    if (lock_request_queue_it == table_lock_map_.end()) {
        txn->SetState(TransactionState::ABORTED);
        throw TransactionAbortException(txn->GetTransactionId(), AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
    }

    // 解锁前保证没有任何行锁
    const auto &row_s_lock_set = txn->GetSharedRowLockSet()->find(oid);
    const auto &row_x_lock_set = txn->GetExclusiveRowLockSet()->find(oid);

    if (!(row_s_lock_set == txn->GetSharedRowLockSet()->end() || row_x_lock_set == txn->GetExclusiveRowLockSet()->end())) {
        txn->SetState(TransactionState::ABORTED);
        throw TransactionAbortException(txn->GetTransactionId(), AbortReason::TABLE_UNLOCKED_BEFORE_UNLOCKING_ROWS);
    }
    auto request_queue = lock_request_queue_it->second;
    request_queue->latch_.lock();
    for (auto it = request_queue->request_queue_.begin(); it != request_queue->request_queue_.end(); ++it) {

        if ((*it)->txn_id_ == txn->GetTransactionId() && (*it)->granted_) {
            if (txn->GetState() == TransactionState::GROWING) {
                switch(txn->GetIsolationLevel()) {
                    case IsolationLevel::REPEATABLE_READ:
                        if ((*it)->lock_mode_ == LockMode::EXCLUSIVE || (*it)->lock_mode_ == LockMode::SHARED) {
                            txn->SetState(TransactionState::SHRINKING);
                        }
                        break;
                    case IsolationLevel::READ_COMMITTED:
                        if ((*it)->lock_mode_ == LockMode::EXCLUSIVE) {
                            txn->SetState(TransactionState::SHRINKING);
                        }
                        break;
                    case IsolationLevel::READ_UNCOMMITTED:
                        if ((*it)->lock_mode_ == LockMode::EXCLUSIVE) {
                            txn->SetState(TransactionState::SHRINKING);
                        }
                        if ((*it)->lock_mode_ == LockMode::SHARED) {
                            txn->SetState(TransactionState::ABORTED);
                            throw Exception("read uncommitted use shared lock");
                        }
                        break;
                }
            }
            // request_queue->latch_.lock();
            ModifyLockSet(txn,oid,(*it)->lock_mode_,LockObject::TABLE,ModifyMode::DELETE);
            delete (*it);

            request_queue->request_queue_.erase(it);

            request_queue->latch_.unlock();
            request_queue->cv_.notify_all();
            return true;
        }
    }
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
}

auto LockManager::LockRow(Transaction *txn, LockMode lock_mode, const table_oid_t &oid, const RID &rid) -> bool {
    CheckLock(txn, lock_mode, LockObject::ROW);
    CheckTableIntentionLock(txn,lock_mode,oid);

    row_lock_map_latch_.lock();
    auto lock_request_queue_it = row_lock_map_.find(rid);
    row_lock_map_latch_.unlock();

    if (lock_request_queue_it == row_lock_map_.end()) {
        row_lock_map_latch_.lock();
        row_lock_map_.emplace(rid,new LockRequestQueue());

        LockRequest *lock_request = new LockRequest(txn->GetTransactionId(), lock_mode, oid, rid);
        lock_request->granted_ = true;
        row_lock_map_[rid]->request_queue_.push_back(lock_request);
        row_lock_map_latch_.unlock();
        ModifyLockSet(txn,oid,lock_mode,LockObject::ROW,ModifyMode::ADD,rid);
        return true;
    }
    auto request_queue = lock_request_queue_it->second;
    request_queue->latch_.lock();
    for (auto lock_request_iter = request_queue->request_queue_.begin(); lock_request_iter != request_queue->request_queue_.end(); ++lock_request_iter) {
        auto lock_request1 = *lock_request_iter;
        if (txn->GetTransactionId() == lock_request1->txn_id_) {
            if (lock_request1->lock_mode_ == lock_mode) {
                request_queue->latch_.unlock();
                return true;
            }

            if (request_queue->upgrading_ != INVALID_TXN_ID) {
                request_queue->latch_.unlock();
                txn->SetState(TransactionState::ABORTED);
                throw TransactionAbortException(txn->GetTransactionId(), AbortReason::UPGRADE_CONFLICT);
            }

            request_queue->upgrading_ = txn->GetTransactionId();
            ModifyLockSet(txn,oid,lock_mode,LockObject::ROW,ModifyMode::DELETE,rid);
            delete (*lock_request_iter);
            request_queue->request_queue_.erase(lock_request_iter);

            LockRequest *lock_request = new LockRequest(txn->GetTransactionId(), lock_mode, oid, rid);
            request_queue->request_queue_.push_back(lock_request);
            lock_request_iter = std::prev(request_queue->request_queue_.end());
            std::unique_lock<std::mutex> lock(request_queue->latch_,std::adopt_lock);
            while (!CheckGrant(lock_request,request_queue)) {
                request_queue->cv_.wait(lock);
                if (txn->GetState() == TransactionState::ABORTED) {
                    delete lock_request;
                    request_queue->request_queue_.erase(lock_request_iter);
                    request_queue->upgrading_ = INVALID_TXN_ID;
                    request_queue->cv_.notify_all();
                    return false;
                }
            }
            request_queue->upgrading_ = INVALID_TXN_ID;
            lock_request->granted_ = true;
            ModifyLockSet(txn,oid,lock_mode,LockObject::ROW,ModifyMode::ADD,rid);
            if (lock_mode!=LockMode::EXCLUSIVE) {
                request_queue->cv_.notify_all();
            }
            return true;
        }
    }
    LockRequest *lock_request = new LockRequest(txn->GetTransactionId(), lock_mode, oid, rid);
    request_queue->request_queue_.push_back(lock_request);
    auto lock_request_iter = std::prev(request_queue->request_queue_.end());
    std::unique_lock<std::mutex> lock(request_queue->latch_,std::adopt_lock);
    while (!CheckGrant(lock_request,request_queue)) {
        request_queue->cv_.wait(lock);
        if (txn->GetState() == TransactionState::ABORTED) {
            delete lock_request;
            request_queue->request_queue_.erase(lock_request_iter);
            // request_queue->upgrading_ = INVALID_TXN_ID;
            request_queue->cv_.notify_all();
            return false;
        }
    }
    ModifyLockSet(txn,oid,lock_mode,LockObject::ROW,ModifyMode::ADD,rid);
    lock_request->granted_ = true;
    return true;
}

auto LockManager::UnlockRow(Transaction *txn, const table_oid_t &oid, const RID &rid) -> bool {
    row_lock_map_latch_.lock();
    auto lock_request_queue_it = row_lock_map_.find(rid);
    row_lock_map_latch_.unlock();
    if (lock_request_queue_it == row_lock_map_.end()) {
        txn->SetState(TransactionState::ABORTED);
        throw TransactionAbortException(txn->GetTransactionId(), AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
    }

    auto request_queue = lock_request_queue_it->second;
    request_queue->latch_.lock();
    for (auto it = request_queue->request_queue_.begin(); it != request_queue->request_queue_.end(); ++it) {

        if ((*it)->txn_id_ == txn->GetTransactionId() && (*it)->granted_) {
            if (txn->GetState() == TransactionState::GROWING) {
                switch(txn->GetIsolationLevel()) {
                    case IsolationLevel::REPEATABLE_READ:
                        if ((*it)->lock_mode_ == LockMode::EXCLUSIVE || (*it)->lock_mode_ == LockMode::SHARED) {
                            txn->SetState(TransactionState::SHRINKING);
                        }
                        break;
                    case IsolationLevel::READ_COMMITTED:
                        if ((*it)->lock_mode_ == LockMode::EXCLUSIVE) {
                            txn->SetState(TransactionState::SHRINKING);
                        }
                        break;
                    case IsolationLevel::READ_UNCOMMITTED:
                        if ((*it)->lock_mode_ == LockMode::EXCLUSIVE) {
                            txn->SetState(TransactionState::SHRINKING);
                        }
                        if ((*it)->lock_mode_ == LockMode::SHARED) {
                            txn->SetState(TransactionState::ABORTED);
                            throw Exception("read uncommitted use shared lock");
                        }
                        break;
                }
            }
            // request_queue->latch_.lock();
            ModifyLockSet(txn,oid,(*it)->lock_mode_,LockObject::ROW,ModifyMode::DELETE,rid);
            delete (*it);

            request_queue->request_queue_.erase(it);

            request_queue->latch_.unlock();
            request_queue->cv_.notify_all();
            return true;
        }
    }
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
}




auto  LockManager::CheckLock(Transaction *txn, LockMode lock_mode, LockObject lock_object) -> bool {
    if (lock_object == LockObject::ROW) {
        // 不允许行级加意向锁
        if (lock_mode == LockMode::INTENTION_SHARED || lock_mode == LockMode::INTENTION_EXCLUSIVE || lock_mode == LockMode::SHARED_INTENTION_EXCLUSIVE) {
            // 设置事务状态为已中止
            txn->SetState(TransactionState::ABORTED);
            // 防御式编程，预测可能的错误，抛出异常
            throw TransactionAbortException(txn->GetTransactionId(),AbortReason::ATTEMPTED_INTENTION_LOCK_ON_ROW);
        }
    }

    // 在读未提交隔离级别下，不允许共享锁、意向共享锁和共享意向排他锁
    if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED) {
        if (lock_mode == LockMode::SHARED || lock_mode == LockMode::INTENTION_SHARED || lock_mode == LockMode::SHARED_INTENTION_EXCLUSIVE) {
            txn->SetState(TransactionState::ABORTED);
            throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_SHARED_ON_READ_UNCOMMITTED);
        }
    }

    // 事务正在收缩中，不允许加锁,不允许加排他锁和意向排他锁
    if (txn->GetState() == TransactionState::SHRINKING) {
        if (lock_mode == LockMode::EXCLUSIVE || lock_mode == LockMode::INTENTION_EXCLUSIVE) {
            txn->SetState(TransactionState::ABORTED);
            throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
        }
    }

    // 可重复读，所有锁允许处于增长状态，收缩状态不允许有锁
    if (txn->GetIsolationLevel() == IsolationLevel::REPEATABLE_READ) {
       if (txn->GetState() == TransactionState::SHRINKING) {
           txn->SetState(TransactionState::ABORTED);
           throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
       }
       return true;
    }

    // 读已提交，增长状态允许所有锁、 收缩状态只允许意向共享锁和共享锁
    if (txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED) {
        if (txn->GetState() == TransactionState::SHRINKING) {
            if (lock_mode != LockMode::INTENTION_SHARED && lock_mode != LockMode::SHARED){
                txn->SetState(TransactionState::ABORTED);
                throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
            }
        }
        return true;
    }

    // 读未提交，在增长状态允许使用排他锁、意向排他锁，不允许使用共享锁和意向共享锁和共享意向排他锁
    if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED) {
            if (txn->GetState() != TransactionState::GROWING || (lock_mode != LockMode::EXCLUSIVE && lock_mode != LockMode::INTENTION_EXCLUSIVE)){
                txn->SetState(TransactionState::ABORTED);
                throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
            }
        return true;
    }
    return true;
}


auto LockManager::CheckGrant(const LockRequest* checked_request, std::shared_ptr<LockRequestQueue> request_queue) -> bool {
    const auto& lock_mode = checked_request->lock_mode_;
    for (const auto &request : request_queue->request_queue_) {
        if (request->granted_) {
            switch (request->lock_mode_) {
                case LockMode::SHARED:
                    if (lock_mode == LockMode::INTENTION_SHARED|| lock_mode == LockMode::SHARED) {
                        break;
                    } else {
                        return false;
                    }
                case LockMode::EXCLUSIVE:
                    return false;
                case LockMode::INTENTION_SHARED:
                    if (lock_mode == LockMode::EXCLUSIVE) {
                        return false;
                    } else {
                        break;
                    }
                case LockMode::INTENTION_EXCLUSIVE:
                    if (lock_mode == LockMode::INTENTION_SHARED || lock_mode == LockMode::INTENTION_EXCLUSIVE) {
                        break;
                    } else {
                        return false;
                    }
                case LockMode::SHARED_INTENTION_EXCLUSIVE:
                    if (lock_mode == LockMode::INTENTION_SHARED) {
                        break;
                    } else {
                        return false;
                    }
            }
        } else {
            // 未授权请求，按先进先出原则， 不是当前请求，返回false，否则返回true
            return request == checked_request;
        }
    }
    throw Exception("unexpected error in grant check");
}
void LockManager::ModifyLockSet(Transaction *txn,oid_t oid, LockMode lock_mode, LockObject lock_object, ModifyMode modify_mode,RID rid) {
    if (modify_mode == ModifyMode::ADD) {
        if (lock_object == LockObject::TABLE) {
            switch (lock_mode) {
                case LockMode::SHARED:
                    txn->SetTableSharedLocked(oid);
                    break;
                case LockMode::EXCLUSIVE:
                    txn->SetTableExclusiveLocked(oid);
                    break;
                case LockMode::INTENTION_SHARED:
                    txn->SetTableIntentionSharedLocked(oid);
                    break;
                case LockMode::INTENTION_EXCLUSIVE:
                    txn->SetTableIntentionExclusiveLocked(oid);
                    break;
                case LockMode::SHARED_INTENTION_EXCLUSIVE:
                    txn->SetTableSharedIntentionExclusiveLocked(oid);
                    break;
            }
        } else if (lock_object == LockObject::ROW) {
            switch (lock_mode) {
                case LockMode::SHARED:
                    txn->SetRowSharedLocked(oid,rid);
                    break;
                case LockMode::EXCLUSIVE:
                    txn->SetRowExclusiveLocked(oid,rid);
                    break;
               default:
                    break;
            }
        }
    } else if (modify_mode == ModifyMode::DELETE) {
        if (lock_object == LockObject::TABLE) {
            switch (lock_mode) {
                case LockMode::SHARED:
                    txn->SetTableSharedUnlocked(oid);
                    break;
                case LockMode::EXCLUSIVE:
                    txn->SetTableExclusiveUnlocked(oid);
                    break;
                case LockMode::INTENTION_SHARED:
                    txn->SetTableIntentionSharedUnlocked(oid);
                    break;
                case LockMode::INTENTION_EXCLUSIVE:
                    txn->SetTableIntentionExclusiveUnlocked(oid);
                    break;
                case LockMode::SHARED_INTENTION_EXCLUSIVE:
                    txn->SetTableSharedIntentionExclusiveUnlocked(oid);
                    break;
            }
        } else if (lock_object == LockObject::ROW) {
            switch (lock_mode) {
                case LockMode::SHARED:
                    txn->SetRowSharedUnLocked(oid,rid);
                    break;
                case LockMode::EXCLUSIVE:
                    txn->SetRowExclusiveUnLocked(oid,rid);
                    break;
                default:
                    break;
            }
        }
    }
}
void LockManager::CheckTableIntentionLock(Transaction *txn, const LockManager::LockMode &lockMode,table_oid_t oid) {
    switch (lockMode) {
        // 如果想获取一个节点的S/IS锁，那么这个事务必须至少持有父节点的IS锁；
        case LockMode::SHARED:
            if (!txn->IsTableIntentionSharedLocked(oid)
                &&!txn->IsTableSharedLocked(oid)
                &&!txn->IsTableExclusiveLocked(oid)
                &&!txn->IsTableIntentionExclusiveLocked(oid)) {
                txn->SetState(TransactionState::ABORTED);
                throw TransactionAbortException(txn->GetTransactionId(), AbortReason::TABLE_LOCK_NOT_PRESENT);
            }
            break;
        // 如果想获取一个节点的X/IX/SIX锁，那么这个事务必须至少持有父节点的IX锁；
        case LockMode::EXCLUSIVE:
            if (!txn->IsTableExclusiveLocked(oid)
                &&!txn->IsTableIntentionExclusiveLocked(oid)
                &&!txn->IsTableSharedIntentionExclusiveLocked(oid)) {
                txn->SetState(TransactionState::ABORTED);
                throw TransactionAbortException(txn->GetTransactionId(), AbortReason::TABLE_LOCK_NOT_PRESENT);
            }
        default:
            break;
    }
}

void LockManager::AddEdge(txn_id_t t1, txn_id_t t2) {
    // std::scoped_lock<std::mutex> lock(waits_for_latch_);
    // waits_for_latch_.lock();
    auto iter = waits_for_.find(t1);
    // 没有 新建加入
    if (iter == waits_for_.end()) {
        waits_for_[t1].push_back(t2);
        // waits_for_latch_.unlock();
        return;
    }
    // 重复不加入
    for (auto &id : iter->second) {
        if (id == t2) {
            // waits_for_latch_.unlock();
            return;
        }
    }
    waits_for_[t1].push_back(t2);
    // waits_for_latch_.unlock();
}


void LockManager::RemoveEdge(txn_id_t t1, txn_id_t t2) {
    // std::scoped_lock<std::mutex> lock(waits_for_latch_);
    // waits_for_latch_.lock();
    auto iter = waits_for_.find(t1);
    if (iter == waits_for_.end()) {
        // waits_for_latch_.unlock();
        return;
    }

    for (auto it = waits_for_[t1].begin(); it != waits_for_[t1].end(); ++it) {
        if (*it == t2) {
            waits_for_[t1].erase(it);
            // vec为空，则删除这个map映射
            if (waits_for_[t1].empty()) {
                waits_for_.erase(iter);
            }
            // waits_for_latch_.unlock();
            return;
        }
    }
   // waits_for_latch_.unlock();
}

auto LockManager::HasCycle(txn_id_t *txn_id) -> bool {
    // std::scoped_lock<std::mutex> lock(waits_for_latch_);
    std::unordered_set<txn_id_t> idset;
    std::function<txn_id_t(txn_id_t)> dfs = [&](txn_id_t id){
        if (waits_for_.find(id) == waits_for_.end()) return -1;
        for (auto &i : waits_for_[id]) {
            if (idset.find(i)!=idset.end()) {
                return std::max(id,i);
            }
            idset.insert(i);
            txn_id_t temp = dfs(i);
            if (temp == -1) {
                idset.erase(i);
                continue;
            }
            return std::max(id,temp);
        }
        return -1;
    };
    *txn_id = -1;
    for (auto &i: waits_for_) {
        idset.insert(i.first);
        *txn_id = dfs(i.first);
        idset.erase(i.first);
        if (*txn_id!=-1) return true;
    }
    return false;
}

auto LockManager::GetEdgeList() -> std::vector<std::pair<txn_id_t, txn_id_t>> {
    std::scoped_lock<std::mutex> lock(waits_for_latch_);
    std::vector<std::pair<txn_id_t, txn_id_t>> edges(0);
    for (auto iter : waits_for_) {
        for (auto it : iter.second) {
            edges.emplace_back(iter.first,it);
        }
    }
    return edges;
}

void LockManager::RunCycleDetection() {
    while (enable_cycle_detection_) {

        std::this_thread::sleep_for(cycle_detection_interval);
        {  // TODO(students): detect deadlock
            std::scoped_lock<std::mutex> lock(waits_for_latch_);
            waits_for_.clear();
            // 构建waits_for图
            // 从table中构建
            table_lock_map_latch_.lock();
            for (const auto &iter : table_lock_map_) {
                for (const auto &i : iter.second->request_queue_) {
                    for (const auto &j :iter.second->request_queue_) {
                        if (i==j) continue;
                        if (i->granted_ == false && j->granted_ == true) {
                            AddEdge(i->txn_id_,j->txn_id_);
                            // std::cout << "add table edge : " << i->txn_id_ << "->" << j->txn_id_ << std::endl;
                        }
                    }
                }
            }
            table_lock_map_latch_.unlock();

            // 从row中构建
            row_lock_map_latch_.lock();
            for (const auto &iter : row_lock_map_) {
                for (const auto &i : iter.second->request_queue_) {
                    for (const auto &j : iter.second->request_queue_) {
                        if (i == j) continue;
                        if (i->granted_ == false && j->granted_ == true) {
                            AddEdge(i->txn_id_,j->txn_id_);
                            // std::cout << "add row edge : " << i->txn_id_ << "->" << j->txn_id_ << std::endl;
                        }
                    }
                }
            }
            row_lock_map_latch_.unlock();


            txn_id_t txn_id = -1;

//            std::cout << "waits_for : ";
//            for (auto &i : waits_for_) {
//                std::cout << i.first << " ";
//            }
//            std::cout << std::endl;

            bool hascycle = HasCycle(&txn_id);
            // std::cout << "has cycle: " << hascycle <<  std::endl;
            while (hascycle) {
                auto txn = TransactionManager::GetTransaction(txn_id);
                // 打破循环等待 设置事务为aborted
                txn->SetState(TransactionState::ABORTED);
                auto it = waits_for_.find(txn_id);
                if (it != waits_for_.end()) {
                    waits_for_.erase(it);
                }
                std::vector<txn_id_t> tvec;
                for (auto &i : waits_for_) {
                    for (auto &j : i.second) {
                        if (j == txn_id) {
                            tvec.push_back(i.first);
                            // RemoveEdge(i.first,j);
                        }
                    }
                }
                for (auto &i : tvec) {
                    RemoveEdge(i,txn_id);
                }

                row_lock_map_latch_.lock();
                for (auto &i : row_lock_map_) {
                    for (auto &j : i.second->request_queue_) {
                        if (j->txn_id_ == txn_id) {
                            i.second->cv_.notify_all();
                            break;
                        }
                    }
                }
                row_lock_map_latch_.unlock();

                table_lock_map_latch_.lock();
                for (auto &i : table_lock_map_) {
                    for (auto &j : i.second->request_queue_) {
                        if (j->txn_id_ == txn_id) {
                            i.second->cv_.notify_all();
                            break;
                        }
                    }
                }
                table_lock_map_latch_.unlock();

                hascycle = HasCycle(&txn_id);

            }
            //enable_cycle_detection_ = false;
            //return;
        }
    }
}
}  // namespace bustub
