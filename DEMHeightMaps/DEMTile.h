#ifndef DEM_TILE_H
#define DEM_TILE_H

#include <unordered_map>

#include "./GPSUtils.h"

//=============================================================================================
//=============================================================================================
//=============================================================================================

typedef struct DEMTileInfo 
{
	double botLeftLat; //sirka (+ => N, - => S)
	double botLeftLon; //delka (+ => E, - => W)

	double stepLat;
	double stepLon;

	int width;
	int height;

	int bytesPerValue;

	std::string fileName;

	/**
	* 2 --- 3
	* |		|
	* 0 --- 1
	*/
	GPSPoint GetCorner(int i) const
	{
		if (i == 0) return GPSPoint(botLeftLon, botLeftLat);
		if (i == 1) return GPSPoint(botLeftLon + stepLon, botLeftLat);
		if (i == 2) return GPSPoint(botLeftLon, botLeftLat + stepLat);
		return GPSPoint(botLeftLon + stepLon, botLeftLat + stepLat);
	};

	bool IsPointInside(double lon, double lat) const	
	{
		if (lon < botLeftLon) return false;
		if (lat < botLeftLat) return false;

		if (lon > botLeftLon + stepLon) return false;
		if (lat > botLeftLat + stepLat) return false;
		
		return true;
	};

} DEMTileInfo;

struct hashFunc 
{
	size_t operator()(const DEMTileInfo &k) const 
	{
		size_t h1 = std::hash<double>()(k.botLeftLon);
		size_t h2 = std::hash<double>()(k.botLeftLat);
		return (h1 ^ (h2 << 1));
	}
};

struct equalsFunc 
{
	bool operator()(const DEMTileInfo& lhs, const DEMTileInfo& rhs) const 
	{
		return (lhs.botLeftLon == rhs.botLeftLon) && (lhs.botLeftLat == rhs.botLeftLat);
	}
};

//=============================================================================================
//=============================================================================================
//=============================================================================================

class DEMTileData 
{
	public:
				
		DEMTileData(const DEMTileInfo & info);
		~DEMTileData();

		
		short GetValue(double lon, double lat);

		friend class DEMData;

	private:
		


		DEMTileInfo info;
		short * data;

		void LoadTileData();
};

#endif
