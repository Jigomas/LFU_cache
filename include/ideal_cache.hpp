#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
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

	struct KeyAccess {
		std::vector<size_t> accesses;
		size_t current_index;
	};

	std::unordered_map<KeyT, KeyAccess>    access_sequence_;
	std::unordered_map<KeyT, CacheEntry>   data_;
	size_t capacity_;
	size_t size_;
	size_t current_access_index_;
};



template<typename KeyT, typename ValueT>
IdealCache<KeyT, ValueT>::IdealCache(size_t capacity)
	: capacity_(capacity), size_(0), current_access_index_(0) {
	data_.reserve(capacity_);                    
	access_sequence_.reserve(capacity_ * 3);     
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::LoadAccessPattern(KeyT key, const std::vector<size_t>& access_times) {
	access_sequence_[key] = KeyAccess{access_times, 0};
}



template<typename KeyT, typename ValueT>
ValueT* IdealCache<KeyT, ValueT>::Get(KeyT key) {
	auto it = data_.find(key);
	if (it == data_.end()) {
		return nullptr;
	}

	++current_access_index_;
	UpdateNextUses();
	
	return &(it->second.data);
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::Put(const KeyT& key, const ValueT& value) {
	if (capacity_ == 0) return;

	++current_access_index_;
	UpdateNextUses();

	if (data_.find(key) != data_.end()) {
		data_[key].data = value;
		return;
	}

	auto seq_it = access_sequence_.find(key);
	if (seq_it == access_sequence_.end()) {
		return;
	}

	auto& access = seq_it->second;
	if (access.current_index >= access.accesses.size()) {
		return;
	}

	++access.current_index;
	if (access.current_index >= access.accesses.size()) {
		return;
	}

	size_t new_next_use = access.accesses[access.current_index];

	if (size_ >= capacity_) {
		auto max_it = std::max_element(data_.begin(), data_.end(),
			[](const auto& a, const auto& b) {
				return a.second.next_use < b.second.next_use;
			});

		if (new_next_use >= max_it->second.next_use) {
			return;
		}
		
		data_.erase(max_it);
		--size_;
	}

	data_.emplace(key, CacheEntry{value, new_next_use});
	++size_;
}



template<typename KeyT, typename ValueT>
bool IdealCache<KeyT, ValueT>::Contains(KeyT key) const {
	return data_.find(key) != data_.end();
}



template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::DumpCache() const {
	std::cout << "Index: " << current_access_index_ << std::endl;
	//std::cout << "Capacity: " << capacity_ << ", Size: " << size_ << std::endl;
	//std::cout << "Data contents:" << std::endl;
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
	static constexpr size_t MAX_USE = std::numeric_limits<size_t>::max();
	
	for (auto& [key, entry] : data_) {
		auto seq_it = access_sequence_.find(key);
		if (seq_it == access_sequence_.end()) {
			entry.next_use = MAX_USE;
			continue;
		}

		auto& access = seq_it->second;
		size_t& idx = access.current_index;
		const auto& accesses = access.accesses;
		
		while (idx < accesses.size() && accesses[idx] <= current_access_index_) {
			++idx;
		}

		entry.next_use = (idx < accesses.size()) ? accesses[idx] : MAX_USE;
	}
}




template<typename KeyT, typename ValueT>
void IdealCache<KeyT, ValueT>::Remove() {
	if (data_.empty()) return;

	auto target = std::max_element(data_.begin(), data_.end(), 
		[](const auto& a, const auto& b) {
			return a.second.next_use < b.second.next_use;
		});
	
	data_.erase(target);
	--size_;
}