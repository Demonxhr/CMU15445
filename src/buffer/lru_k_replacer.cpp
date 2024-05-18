//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
    std::scoped_lock<std::mutex> lock(latch_);
    bool victim_found = false;
    if(!hist_list_.empty())
    {
        // rbegin():倒数第一个元素
        for(auto rit = hist_list_.rbegin();rit!=hist_list_.rend();++rit)
        {
            // 判断是否可以移除
            if(entries_[*rit].evictable_){
                *frame_id = *rit;
                // std::next会让反向迭代器指向下一个迭代器如【1，2，3，4，5】 当当前指向5 调用next指向4
                // base会让反向迭代器转为正向迭代器，并指向正向的下一个  即5.
                hist_list_.erase(std::next(rit).base());
                victim_found = true;
                break;
            }

        }
    }
    if(!victim_found && !cache_list_.empty())
    {
        for(auto rit = cache_list_.rbegin(); rit!=cache_list_.rend();++rit)
        {
            if(entries_[*rit].evictable_)
            {
                *frame_id = *rit;
                cache_list_.erase(std::next(rit).base());
                victim_found = true;
                break;
            }
        }
    }
    if(victim_found)
    {
        entries_.erase(*frame_id);
        --curr_size_;
        return true;
    }
    return false; 
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
    std::scoped_lock<std::mutex> lock(latch_);
    if(frame_id > static_cast<int>(replacer_size_))
    {
        throw std::invalid_argument(std::string("Invalid frame_id") + std::to_string(frame_id));
    }

    size_t new_count = ++entries_[frame_id].hit_count_;
    // 第一次加入
    if(new_count==1){
        ++curr_size_;
        hist_list_.emplace_front(frame_id);
        // list 不会因为list改变而使iterator发生改变
        entries_[frame_id].pos_ = hist_list_.begin();
    }
    else{
        // 到达k次 加入到cache
        if(new_count == k_)
        {
            hist_list_.erase(entries_[frame_id].pos_);
            cache_list_.emplace_front(frame_id);
            entries_[frame_id].pos_ = cache_list_.begin();
        }
        // k次以上，放到队头
        else if(new_count > k_)
        {
            cache_list_.erase(entries_[frame_id].pos_);
            cache_list_.emplace_front(frame_id);
            entries_[frame_id].pos_ = cache_list_.begin();
        }
    }
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
    std::scoped_lock<std::mutex> lock(latch_);
    if(frame_id > static_cast<int>(replacer_size_))
    {
        throw std::invalid_argument(std::string("Invalid frame_id") + std::to_string(frame_id));
    }
    if(entries_.find(frame_id) == entries_.end())
    {
        return;
    }
    if(set_evictable && !entries_[frame_id].evictable_){
        ++curr_size_;
    }
    else if(entries_[frame_id].evictable_ && !set_evictable){
        --curr_size_;
    }
    entries_[frame_id].evictable_ = set_evictable;
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
    std::scoped_lock<std::mutex> lock(latch_);
    if(entries_.find(frame_id)==entries_.end())
    {
        return;
    }
    if(!entries_[frame_id].evictable_){
        throw std::logic_error(std::string("Can't remove an inevictable frame ")+std::to_string(frame_id));
    }
    if(entries_[frame_id].hit_count_ < k_){
        hist_list_.erase(entries_[frame_id].pos_);
    }
    else{
        cache_list_.erase(entries_[frame_id].pos_);
    }
    --curr_size_;
    entries_.erase(frame_id);
}

auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace bustub
