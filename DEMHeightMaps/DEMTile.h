#ifndef DEM_TILE_H
#define DEM_TILE_H


#include <atomic>
#include <unordered_map>
#include <mutex>

#include <GeoCoordinate.h>
#include <MapProjection.h>

#include "./Cache/DataCache.h"

#include "./Strings/MyString.h"

//=============================================================================================
//=============================================================================================
//=============================================================================================

//https://librenepal.com/article/reading-srtm-data-with-python/
typedef struct TileInfo 
{
	enum SOURCE { HGT = 1, BIL = 2 };

	
	GeoCoordinate minLat; //sirka (+ => N, - => S)
	GeoCoordinate minLon; //delka (+ => E, - => W)

	GeoCoordinate stepLat;
	GeoCoordinate stepLon;

	GeoCoordinate pixelStepLat;
	GeoCoordinate pixelStepLon;

	int width;
	int height;

	SOURCE source;

	/**
	* 2 --- 3
	* |		|
	* 0 --- 1
	*/
	Projections::Coordinate GetCorner(int i) const
	{
		if (i == 0) return { minLon, minLat };
		if (i == 1) return { GeoCoordinate::rad(minLon.rad() + stepLon.rad()), minLat };
		if (i == 2) return { minLon, GeoCoordinate::rad(minLat.rad() + stepLat.rad()) };
		return { GeoCoordinate::rad(minLon.rad() + stepLon.rad()), GeoCoordinate::rad(minLat.rad() + stepLat.rad()) };
	};

	bool IsPointInside(const Projections::Coordinate & p) const
	{
		if (p.lon.rad() < minLon.rad()) return false;
		if (p.lat.rad() < minLat.rad()) return false;

		if (p.lon.rad() > minLon.rad() + stepLon.rad()) return false;
		if (p.lat.rad() > minLat.rad() + stepLat.rad()) return false;

		return true;
	};

} TileInfo;

struct hashFunc 
{
	size_t operator()(const TileInfo &k) const 
	{
		size_t h1 = std::hash<double>()(k.minLon.rad());
		size_t h2 = std::hash<double>()(k.minLat.rad());
		return (h1 ^ (h2 << 1));
	}
};

struct equalsFunc 
{
	bool operator()(const TileInfo& lhs, const TileInfo& rhs) const 
	{
		return (lhs.minLon.rad() == rhs.minLon.rad()) && (lhs.minLat.rad() == rhs.minLat.rad());
	}
};

//=============================================================================================
//=============================================================================================
//=============================================================================================

typedef struct DEMTileInfo : TileInfo
{
	int bytesPerValue;

	MyStringAnsi fileName;
	MyStringAnsi filePath;
	bool isArchived;

} DEMTileInfo;

typedef struct TileRawData 
{
	size_t dataSize;
	short * data;

	~TileRawData()
	{
		//delete[] data;
		//data = nullptr;
		//dataSize = 0;
	}

} TileRawData;

class DEMTileData 
{
	public:
					
		DEMTileData(MemoryCache<MyStringAnsi, TileRawData, LRUControl<MyStringAnsi>> * cache);
		DEMTileData(DEMTileData const&) = default;

		DEMTileData& operator=(DEMTileData const&) = delete;		
		~DEMTileData();

		DEMTileInfo * GetTileInfo();

		//void ReleaseData();
		void LoadTileData();

		void SetTileInfo(DEMTileInfo * info);
		short GetValue(const Projections::Coordinate & c);

		
	private:
		
		
		MemoryCache<MyStringAnsi, TileRawData, LRUControl<MyStringAnsi>> * cache;
		DEMTileInfo * info;

		TileRawData data;
		

		short GetValue(int index);

		
};

#endif
