#ifndef DEM_DATA_H
#define DEM_DATA_H

#include <unordered_map>
#include "./GPSUtils.h"

#include "./DEMTile.h"



typedef std::unordered_map<DEMTileInfo, DEMTileData *, hashFunc, equalsFunc> DemTileMap;


class DEMData 
{
	public:

		typedef struct DEMArea
		{
			DEMArea(const GPSPoint & min, const GPSPoint & max) 
				: min(min), max(max)
			{
			}

			GPSPoint min;
			GPSPoint max;
					
			std::vector<DEMTileData *> tileData;

		} DEMArea;

		DEMData(const std::string & dir);
		DEMData(const std::string & dir, const std::string & tilesInfoXML);
		~DEMData();
		
		void SetMinMaxElevation(double minElev, double maxElev);

		void ExportTileList(const std::string & fileName);
		DEMData::DEMArea GetTilesInArea(double topLeftLat, double topLeftLon, double botRightLat, double botRightLon);

		uint8_t * BuildMap(int w, int h, double minLon, double minLat, double maxLon, double maxLat, bool keepAR);

		short GetValue(DEMArea & area, double lon, double lat);

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

		bool IsInside(const GPSPoint & p, DEMArea & area);
		
};



#endif
