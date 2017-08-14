#include "./DEMData.h"

#include <MapProjection.h>
#include <GeoCoordinate.h>
#include <Projections.h>

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"
#include "./TinyXML/tinyxml.h"


//=======================================================================================
// ctors & dtor
//=======================================================================================

DEMData::DEMData(const std::string & dir)
{
	this->minHeight = 0;
	this->maxHeight = 9000;

	this->tileDir = dir;
	VFS::Initialize(dir, DEBUG_MODE);

	this->LoadTileDir(dir);
}

DEMData::DEMData(const std::string & dir, const std::string & tilesInfoXML)
{
	this->minHeight = 0;
	this->maxHeight = 9000;

	this->tileDir = dir;
	VFS::Initialize(dir, DEBUG_MODE);

	this->ImportTileList(tilesInfoXML);
}

DEMData::~DEMData()
{
}

void DEMData::SetMinMaxElevation(double minElev, double maxElev)
{
	this->minHeight = minElev;
	this->maxHeight = maxElev;
}

//=======================================================================================
// Loading
//=======================================================================================

void DEMData::LoadTileDir(const std::string & dir)
{	
	std::vector<VFS_FILE *> tileFiles = VFS::GetInstance()->GetAllFiles();

	
	//map file names are: lat | lon - down left corner (south-west)
	//eg: N35W001.hgt

	for (const auto & file : tileFiles)
	{
		const std::string & fileName = file->name;

		if (fileName.length() != 7)
		{
			printf("Incorrect tile file: %s\n", file->filePath);
			continue;
		}
		
		int lat = 10 * (fileName[1] - '0') + (fileName[2] - '0');
		int lon = 100 * (fileName[4] - '0') + 10 * (fileName[5] - '0') + (fileName[6] - '0');

		if (fileName[0] == 'S')
		{
			lat *= -1;
		}
		if (fileName[3] == 'W')
		{
			lon *= -1;
		}

		int tileSize = 0;
		int bytesPerValue = 0;

		if (file->fileSize == TILE_SIZE_1 * TILE_SIZE_1)
		{
			tileSize = TILE_SIZE_1;
			bytesPerValue = 1;
		}
		else if (file->fileSize == 2 * TILE_SIZE_1 * TILE_SIZE_1)
		{
			tileSize = TILE_SIZE_1;
			bytesPerValue = 2;
		}
		else if (file->fileSize == TILE_SIZE_3 * TILE_SIZE_3)
		{
			tileSize = TILE_SIZE_3;
			bytesPerValue = 1;
		}
		else if (file->fileSize == 2 * TILE_SIZE_3 * TILE_SIZE_3)
		{
			tileSize = TILE_SIZE_3;
			bytesPerValue = 2;
		}

		DEMTileInfo tileInfo;
		tileInfo.botLeftLat = GeoCoordinate::deg(lat);
		tileInfo.botLeftLon = GeoCoordinate::deg(lon);
		tileInfo.stepLat = GeoCoordinate::deg(1);
		tileInfo.stepLon = GeoCoordinate::deg(1);

		tileInfo.width = tileSize;
		tileInfo.height = tileSize;
		tileInfo.bytesPerValue = bytesPerValue;
		tileInfo.fileName = file->filePath;

		
		this->tiles.push_back(tileInfo);
	}	
}

void DEMData::ImportTileList(const std::string & fileName)
{
	TiXmlDocument doc(fileName.c_str());
	bool loadOkay = doc.LoadFile();
	if (loadOkay == false)
	{		
		printf("Failed to load file \"%s\"\n", fileName.c_str());
		return;
	}


	TiXmlElement * tileNodes = doc.FirstChildElement("dem")->FirstChildElement("tile");
	if (tileNodes != nullptr)
	{		
		while (tileNodes)
		{
			DEMTileInfo di;
			di.fileName = tileNodes->Attribute("name");
			di.botLeftLat = GeoCoordinate::deg(atof(tileNodes->Attribute("lat")));
			di.botLeftLon = GeoCoordinate::deg(atof(tileNodes->Attribute("lon")));
			di.stepLat = GeoCoordinate::deg(atof(tileNodes->Attribute("step_lat")));
			di.stepLon = GeoCoordinate::deg(atof(tileNodes->Attribute("step_lon")));
			di.width = atoi(tileNodes->Attribute("w"));
			di.height = atoi(tileNodes->Attribute("h"));
			di.bytesPerValue = atoi(tileNodes->Attribute("b"));

			this->tiles.push_back(di);

			tileNodes = tileNodes->NextSiblingElement("tile");
		}


	}

}

void DEMData::ExportTileList(const std::string & fileName)
{
	TiXmlDocument doc;
	
	TiXmlElement * root = new TiXmlElement("dem");
	doc.LinkEndChild(root);
	
	TiXmlElement * tile;
	for (size_t i = 0; i < this->tiles.size(); i++)
	{
		const DEMTileInfo & ti = this->tiles[i];

		tile = new TiXmlElement("tile");
		tile->SetAttribute("name", ti.fileName.c_str());
		tile->SetDoubleAttribute("lat", ti.botLeftLat.deg());
		tile->SetDoubleAttribute("lon", ti.botLeftLon.deg());
		tile->SetDoubleAttribute("step_lat", ti.stepLat.deg());
		tile->SetDoubleAttribute("step_lon", ti.stepLon.deg());
		tile->SetAttribute("w", ti.width);
		tile->SetAttribute("h", ti.height);
		tile->SetAttribute("b", ti.bytesPerValue);
		root->LinkEndChild(tile);
	}
	
	doc.SaveFile(fileName.c_str());
}

//=======================================================================================
// Data obtaining
//=======================================================================================

uint8_t * DEMData::BuildMap(int w, int h, const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, bool keepAR)
{
	printf("BUILD map\n");

	uint8_t * heightMap = new uint8_t[w * h];
	memset(heightMap, 0, w * h * sizeof(uint8_t));

	
	IProjectionInfo * mercator = new Mercator();
	mercator->SetFrame(min, max, w, h, keepAR);
	
	DEMData::DEMArea area = this->GetTilesInArea(min.lon, min.lat, max.lon, max.lat);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			IProjectionInfo::Coordinate c = mercator->ProjectInverse({ x, y });
			
			short value = this->GetValue(area, c);
			uint8_t v = static_cast<uint8_t>(Utils::MapRange(this->minHeight, this->maxHeight, 0.0, 255.0, value));

			heightMap[x + y * w] = v;
		}
	}

	return heightMap;
}

DEMData::DEMArea DEMData::GetTilesInArea(GeoCoordinate minLon, GeoCoordinate minLat, GeoCoordinate maxLon, GeoCoordinate maxLat)
{
	DEMArea da({ minLon, minLat }, { maxLon, maxLat });
	
	for (const DEMTileInfo & tile : this->tiles)
	{
		int isInside = false;		
		for (int j = 0; j < 4; j++)
		{
			if (this->IsInside(tile.GetCorner(j), da))
			{
				isInside = true;
				break;
			}
		}

		if (isInside == false)
		{
			continue;
		}

		if (this->cachedTiledData.find(tile) == this->cachedTiledData.end())
		{											
			this->cachedTiledData[tile] = DEMTileData(tile);
		}

		da.tileData.push_back(&this->cachedTiledData[tile]);
	}

	return da;

}

bool DEMData::IsInside(const IProjectionInfo::Coordinate & p, DEMArea & area)
{

	if (p.lat.rad() < area.min.lat.rad()) return false;
	if (p.lat.rad() > area.max.lat.rad()) return false;

	if (p.lon.rad() < area.min.lon.rad()) return false;
	if (p.lon.rad() > area.max.lon.rad()) return false;

	return true;
}


short DEMData::GetValue(const DEMArea & da, const IProjectionInfo::Coordinate & c)
{		
	for (auto ti : da.tileData)
	{				
		if (ti->info.IsPointInside(c.lon, c.lat))
		{			
			short value = ti->GetValue(c);
			return value;
		}

	}

	return 0;
}

