#ifndef _CACHE_CONTROL_H_
#define _CACHE_CONTROL_H_

template <typename Key, typename T>
class CacheType
{
public:
	void Insert(const Key & key) { static_cast<T*>(this)->InsertImpl(key); }
	bool Erase() { return static_cast<T*>(this)->EraseImpl(); }
	const Key & GetLeastKey() { return static_cast<T*>(this)->GetLeastKeyImpl(); }
	void Update(const Key & key) { static_cast<T*>(this)->UpdateImpl(key); }
	size_t GetItemsCount() { return static_cast<T*>(this)->GetItemsCountImpl(); }

	//specialized methods for direct control
	//should be used only in special cases (saving / restoring cache etc.)
	bool Erase(const Key & key) { return static_cast<T*>(this)->EraseImpl(key); }
	size_t GetKeyUsage(const Key & key) { return static_cast<T*>(this)->GetKeyUsageImpl(key); }
	void InsertKeyWithUsage(const Key & key, size_t usage) { return static_cast<T*>(this)->InsertKeyWithUsageImpl(key, usage); }
	void Clear() { return static_cast<T *>(this)->ClearImpl(); }
};

#endif
