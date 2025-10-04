#pragma once

#include <iostream>
#include <queue>
#include <unordered_map>
#include <limits>

template<typename KeyT, typename ValueT>
class IdealCache {
public:
	explicit IdealCache(size_t capacity = 0);
	
	void    LoadAccessPattern(KeyT key, const std::vector<size_t>& access_times);
	ValueT* Get(KeyT key);
	void    Put(const KeyT& key, const ValueT& value);
	bool    Contains(KeyT key) const;
	void    DumpCache() const;
	
	size_t GetCurrentSize() const;
	size_t GetMaxSize() const;

private:
	void Remove();
	void UpdateNextUses();

	struct CacheEntry {
		ValueT data;
		size_t next_use;
	};

	std::unordered_map<KeyT, std::queue<size_t>>    access_sequence_;
	std::unordered_map<KeyT, CacheEntry>            data_;
	size_t capacity_;
	size_t size_;
	size_t current_access_index_;
};



template<typename KeyT, typename ValueT>
IdealCache<KeyT, ValueT>::IdealCache(size_t capacity)
	: capacity_(capacity), size_(0), current_access_index_(0) {}

template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::LoadAccessPattern(KeyT key, const std::vector<size_t>& access_times) {
	std::queue<size_t> time_queue;
	for (auto time : access_times) {
		time_queue.push(time);
	}
	access_sequence_[key] = time_queue;
}



template<typename KeyT, typename ValueT>
ValueT* IdealCache<KeyT, ValueT>::Get(KeyT key) {
	auto it = data_.find(key);
	if (it == data_.end()) {
		return nullptr;
	}

	UpdateNextUses();
	++current_access_index_;
	
	return &(it->second.data);
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::Put(const KeyT& key, const ValueT& value) {
	if (capacity_ == 0) return;

	UpdateNextUses();

	auto it = data_.find(key);
	if (it != data_.end()) {
		it->second.data = value;
		return;
	}

	if (size_ >= capacity_) {
		Remove();
	}

	auto seq_it = access_sequence_.find(key);
	if (seq_it == access_sequence_.end() || seq_it->second.empty()) {
		return;
	}

	CacheEntry new_entry;
	new_entry.data = value;
	new_entry.next_use = seq_it->second.front();
	seq_it->second.pop();

	data_.emplace(key, new_entry);
	++size_;
	++current_access_index_;
}



template<typename KeyT, typename ValueT>
bool IdealCache<KeyT, ValueT>::Contains(KeyT key) const {
	return data_.find(key) != data_.end();
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::DumpCache() const {
	std::cout << "Capacity: " << capacity_ << ", Size: " << size_ << std::endl;
	std::cout << "Data contents:" << std::endl;
	for (const auto& entry : data_) {
		std::cout << "  " << entry.first << " -> " << entry.second.data 
				  << " | Next Use: " << entry.second.next_use << std::endl;
	}
}



template<typename KeyT, typename ValueT>
size_t IdealCache<KeyT, ValueT>::GetCurrentSize() const { 
	return size_; 
}



template<typename KeyT, typename ValueT>
size_t IdealCache<KeyT, ValueT>::GetMaxSize() const { 
	return capacity_; 
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::UpdateNextUses() {
	for (auto& [key, entry] : data_) {
		auto seq_it = access_sequence_.find(key);
		if (seq_it != access_sequence_.end()) {
			while (!seq_it->second.empty() && seq_it->second.front() <= current_access_index_) {
				seq_it->second.pop();
			}
			
			if (!seq_it->second.empty()) {
				entry.next_use = seq_it->second.front();
			} else {
				entry.next_use = std::numeric_limits<size_t>::max();
			}
		} else {
			entry.next_use = std::numeric_limits<size_t>::max();
		}
	}
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::Remove() {
	if (data_.empty()) return;

	auto target = data_.begin();
	size_t furthest_use = target->second.next_use;

	for (auto it = data_.begin(); it != data_.end(); ++it) {
		if (it->second.next_use > furthest_use) {
			furthest_use = it->second.next_use;
			target = it;
		}
	}

	data_.erase(target);
	--size_;
}