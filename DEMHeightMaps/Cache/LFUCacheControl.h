#ifndef _LFU_CACHE_CONTROL_H_
#define _LFU_CACHE_CONTROL_H_

#include <unordered_map>
#include <map>

#include "./CacheControl.h"


/// <summary>
/// Least frequently use
/// </summary>
template <typename Key>
class LFUControl : public CacheType<Key, LFUControl<Key>>
{

	friend class CacheType<Key, LFUControl<Key>>;

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

	using Iterator = typename std::multimap<size_t, Key>::iterator;


	//multimap - can have multiple same keys
	std::multimap<size_t, Key> frequencies;
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
void LFUControl<Key>::InsertImpl(const Key & key)
{
	constexpr size_t INIT_VAL = 1;

	this->InsertKeyWithUsageImpl(key, INIT_VAL);
}

/// <summary>
/// Insert key with specified usage
/// </summary>
/// <param name="key">key to insert</param>
/// <param name="usage">starting usage value</param>
template <typename Key>
void LFUControl<Key>::InsertKeyWithUsageImpl(const Key & key, size_t usage)
{
	auto new_elem = std::make_pair(usage, key);
	keys[key] = frequencies.emplace_hint(frequencies.cbegin(), std::move(new_elem));
}

/// <summary>
/// Update key usage
/// </summary>
/// <param name="key">key to update</param>
template <typename Key>
void LFUControl<Key>::UpdateImpl(const Key & key)
{
	// get the previous frequency value of a key
	Iterator elem_for_update = keys[key];
	auto updated_elem = std::make_pair(elem_for_update->first + 1, elem_for_update->second);

	// update the previous value
	frequencies.erase(elem_for_update);
	keys[key] = frequencies.emplace_hint(frequencies.cend(), std::move(updated_elem));

}

/// <summary>
/// Erase key with lowest frequency
/// </summary>
/// <returns>true / false if key was deleted or not</returns>
template <typename Key>
bool LFUControl<Key>::EraseImpl()
{
	//erase least used value
	auto it = frequencies.cbegin();

	if (it == frequencies.cend())
	{
		return false;
	}

	Key key = it->second;

	frequencies.erase(keys[key]);	//delete element by its iterator stored in keys[key]
	keys.erase(key);

	return true;
}

/// <summary>
/// Erase given key
/// </summary>
/// <param name="key">key to erase</param>
/// <returns>true / false if key was deleted or not</returns>
template <typename Key>
bool LFUControl<Key>::EraseImpl(const Key & key)
{
	if (keys.find(key) == keys.cend())
	{
		return false;
	}

	frequencies.erase(keys[key]);	//delete element by its iterator stored in keys[key]
	keys.erase(key);

	return true;
}

template <typename Key>
void LFUControl<Key>::ClearImpl()
{
	frequencies.clear();
	keys.clear();
}

/// <summary>
/// Usage of the key
/// </summary>
/// <returns>usage number associated with key</returns>
template <typename Key>
size_t LFUControl<Key>::GetKeyUsageImpl(const Key & key)
{
	Iterator elem = keys[key];
	return elem->first;
}

/// <summary>
/// Get key with lowest frequency
/// </summary>
/// <returns>key with lowest frequency</returns>
template <typename Key>
const Key & LFUControl<Key>::GetLeastKeyImpl()
{
	return frequencies.cbegin()->second;
}


#endif
