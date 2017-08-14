#ifndef DEM_DATA_H
#define DEM_DATA_H

#include <unordered_map>
#include <MapProjection.h>
#include <GeoCoordinate.h>


#include "./DEMTile.h"



typedef std::unordered_map<DEMTileInfo, DEMTileData, hashFunc, equalsFunc> DemTileMap;


class DEMData 
{
	public:

		typedef struct DEMArea
		{
			DEMArea(const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max)
				: min(min), max(max)
			{
			}

			IProjectionInfo::Coordinate min;
			IProjectionInfo::Coordinate max;
					
			std::list<DEMTileData *> tileData;

		} DEMArea;

		DEMData(const std::string & dir);
		DEMData(const std::string & dir, const std::string & tilesInfoXML);
		~DEMData();
		
		void SetMinMaxElevation(double minElev, double maxElev);

		void ExportTileList(const std::string & fileName);
		DEMData::DEMArea GetTilesInArea(GeoCoordinate topLeftLat, GeoCoordinate topLeftLon, GeoCoordinate botRightLat, GeoCoordinate botRightLon);

		uint8_t * BuildMap(int w, int h, const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, bool keepAR);

		short GetValue(const DEMArea & area, const IProjectionInfo::Coordinate & c);

	private:
		const int TILE_SIZE_3 = 1201;
		const int TILE_SIZE_1 = 3601;

		double minHeight;
		double maxHeight;

		DemTileMap cachedTiledData;

		std::string tileDir;
		std::vector<DEMTileInfo> tiles;

		void LoadTileDir(const std::string & dir);
		void ImportTileList(const std::string & fileName);

		bool IsInside(const IProjectionInfo::Coordinate & p, DEMArea & area);
		
};



#endif
