#ifndef _DATA_CACHE_H_
#define _DATA_CACHE_H_

//
// Based on implementation from:
// https://github.com/vpetrigo/caches
//

//======================================================
//================= Included ===========================
//======================================================

#define CACHE_SIZE_GB(x) (1024 * 1024 * 1024 * size_t(x))
#define CACHE_SIZE_MB(x) (1024 * 1024 * size_t(x))
#define CACHE_SIZE_KB(x) (1024 * size_t(x))


//======================================================
//======= MemoryCache Cache management =================
//======================================================

#include "./MemoryCache.h"


//======================================================
//========== CRTP interface ============================
//======================================================

#include "./CacheControl.h"

//======================================================
//============ Cache control ===========================
//======================================================

#include "./LFUCacheControl.h"
#include "./LRUCacheControl.h"




#endif
