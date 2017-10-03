#ifndef _LRU_CACHE_CONTROL_H_
#define _LRU_CACHE_CONTROL_H_


#include <unordered_map>
#include <list>

#include "./CacheControl.h"

/// <summary>
/// Least recently used
/// </summary>
template <typename Key>
class LRUControl : public CacheType<Key, LRUControl<Key>>
{
	friend class CacheType<Key, LRUControl<Key>>;

protected:
	void InsertImpl(const Key & key);
	void InsertKeyWithUsageImpl(const Key & key, size_t usage);

	bool EraseImpl();
	bool EraseImpl(const Key & key);
	void ClearImpl();
	const Key & GetLeastKeyImpl();
	void UpdateImpl(const Key & key);
	size_t GetKeyUsageImpl(const Key & key);
	size_t GetItemsCountImpl() { return keys.size(); }

	using Iterator = typename std::list<Key>::const_iterator;

	std::list<Key> lruQueue;
	std::unordered_map<Key, Iterator> keys;
};



//======================================================
//============== Implementation ========================
//======================================================

/// <summary>
/// Insert key with default usage
/// </summary>
/// <param name="key">key to insert</param>
template <typename Key>
void LRUControl<Key>::InsertImpl(const Key & key)
{
	this->InsertKeyWithUsageImpl(key, 0);
}

/// <summary>
/// Insert key with specified usage
/// </summary>
/// <param name="key">key to insert</param>
/// <param name="usage">starting usage value</param>
template <typename Key>
void LRUControl<Key>::InsertKeyWithUsageImpl(const Key & key, size_t usage)
{
	lruQueue.emplace_front(key);
	keys[key] = lruQueue.cbegin();
}

/// <summary>
/// Update key usage
/// </summary>
/// <param name="key">key to update</param>
template <typename Key>
void LRUControl<Key>::UpdateImpl(const Key & key)
{
	auto it = keys.find(key);
	if (it == keys.cend())
	{
		return;
	}

	lruQueue.splice(lruQueue.cbegin(), lruQueue, keys[key]);
}

/// <summary>
/// Erase key with lowest frequency
/// </summary>
/// <returns>true / false if key was deleted or not</returns>
template <typename Key>
bool LRUControl<Key>::EraseImpl()
{
	if (lruQueue.size() == 0)
	{
		return false;
	}
	keys.erase(lruQueue.back());
	lruQueue.pop_back();

	return true;
}

/// <summary>
/// Erase given key
/// </summary>
/// <param name="key">key to erase</param>
/// <returns>true / false if key was deleted or not</returns>
template <typename Key>
bool LRUControl<Key>::EraseImpl(const Key & key)
{
	auto it = keys.find(key);
	if (it == keys.cend())
	{
		return false;
	}


	typename std::list<Key>::iterator tmp;
	for (tmp = lruQueue.begin(); tmp != lruQueue.end(); tmp++)
	{
		if (*tmp == key)
		{
			break;
		}
	}
	lruQueue.erase(tmp);
	keys.erase(it);

	return true;
}

template <typename Key>
void LRUControl<Key>::ClearImpl()
{
	lruQueue.clear();
	keys.clear();
}

/// <summary>
/// Usage of the key
/// </summary>
/// <returns>usage number associated with key</returns>
template <typename Key>
size_t LRUControl<Key>::GetKeyUsageImpl(const Key & key)
{
	//no usage for LRU
	return 0;
}

/// <summary>
/// Get key with lowest frequency
/// </summary>
/// <returns>key with lowest frequency</returns>
template <typename Key>
const Key & LRUControl<Key>::GetLeastKeyImpl()
{
	return lruQueue.back();
}


#endif
