#ifndef DEM_TILE_H
#define DEM_TILE_H

#include <unordered_map>
#include <GeoCoordinate.h>
#include <MapProjection.h>



//=============================================================================================
//=============================================================================================
//=============================================================================================

typedef struct DEMTileInfo 
{
	GeoCoordinate botLeftLat; //sirka (+ => N, - => S)
	GeoCoordinate botLeftLon; //delka (+ => E, - => W)

	GeoCoordinate stepLat;
	GeoCoordinate stepLon;

	int width;
	int height;

	int bytesPerValue;

	std::string fileName;

	/**
	* 2 --- 3
	* |		|
	* 0 --- 1
	*/
	IProjectionInfo::Coordinate GetCorner(int i) const
	{
		if (i == 0) return { botLeftLon, botLeftLat};
		if (i == 1) return { GeoCoordinate::rad(botLeftLon.rad() + stepLon.rad()), botLeftLat };
		if (i == 2) return { botLeftLon, GeoCoordinate::rad(botLeftLat.rad() + stepLat.rad()) };
		return { GeoCoordinate::rad(botLeftLon.rad() + stepLon.rad()), GeoCoordinate::rad(botLeftLat.rad() + stepLat.rad()) };
	};

	bool IsPointInside(const GeoCoordinate & lon, const GeoCoordinate & lat) const
	{
		if (lon.rad() < botLeftLon.rad()) return false;
		if (lat.rad() < botLeftLat.rad()) return false;

		if (lon.rad() > botLeftLon.rad() + stepLon.rad()) return false;
		if (lat.rad() > botLeftLat.rad() + stepLat.rad()) return false;
		
		return true;
	};

} DEMTileInfo;

struct hashFunc 
{
	size_t operator()(const DEMTileInfo &k) const 
	{
		size_t h1 = std::hash<double>()(k.botLeftLon.rad());
		size_t h2 = std::hash<double>()(k.botLeftLat.rad());
		return (h1 ^ (h2 << 1));
	}
};

struct equalsFunc 
{
	bool operator()(const DEMTileInfo& lhs, const DEMTileInfo& rhs) const 
	{
		return (lhs.botLeftLon.rad() == rhs.botLeftLon.rad()) && (lhs.botLeftLat.rad() == rhs.botLeftLat.rad());
	}
};

//=============================================================================================
//=============================================================================================
//=============================================================================================

class DEMTileData 
{
	public:
			
		DEMTileData() = default;
		DEMTileData(const DEMTileInfo & info);
		~DEMTileData();

		
		short GetValue(const IProjectionInfo::Coordinate & c);

		friend class DEMData;

	private:
		


		DEMTileInfo info;
		short * data;

		void LoadTileData();
};

#endif
