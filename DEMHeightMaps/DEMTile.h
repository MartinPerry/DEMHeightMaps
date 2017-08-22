#ifndef DEM_TILE_H
#define DEM_TILE_H


#include <atomic>
#include <unordered_map>
#include <mutex>

#include <GeoCoordinate.h>
#include <MapProjection.h>



//=============================================================================================
//=============================================================================================
//=============================================================================================

typedef struct TileInfo 
{
	GeoCoordinate minLat; //sirka (+ => N, - => S)
	GeoCoordinate minLon; //delka (+ => E, - => W)

	GeoCoordinate stepLat;
	GeoCoordinate stepLon;

	int width;
	int height;

	/**
	* 2 --- 3
	* |		|
	* 0 --- 1
	*/
	IProjectionInfo::Coordinate GetCorner(int i) const
	{
		if (i == 0) return { minLat, minLon };
		if (i == 1) return { minLat,  GeoCoordinate::rad(minLon.rad() + stepLon.rad()) };
		if (i == 2) return { GeoCoordinate::rad(minLat.rad() + stepLat.rad()), minLon };
		return { GeoCoordinate::rad(minLat.rad() + stepLat.rad()), GeoCoordinate::rad(minLon.rad() + stepLon.rad()) };
	};

	bool IsPointInside(const IProjectionInfo::Coordinate & p) const
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

	std::string fileName;

} DEMTileInfo;


class DEMTileData 
{
	public:
					
		DEMTileData() = default;		

		DEMTileData(DEMTileData const&) = delete;
		DEMTileData& operator=(DEMTileData const&) = delete;		
		~DEMTileData();


		void ReleaseData();
		void LoadTileData();

		void SetTileInfo(const DEMTileInfo & info);
		short GetValue(const IProjectionInfo::Coordinate & c);

		friend class DEMData;

	private:
		
		

		DEMTileInfo info;
		short * data;
		int dataSize;

		
};

#endif
