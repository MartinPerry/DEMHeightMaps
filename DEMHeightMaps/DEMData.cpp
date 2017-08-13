#include "./DEMData.h"

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"
#include "./TinyXML/tinyxml.h"

#include "./MapProjections.h"

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

	for (size_t i = 0; i < tileFiles.size(); i++)
	{
		const std::string & fileName = tileFiles[i]->name;
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

		if (tileFiles[i]->fileSize == TILE_SIZE_1 * TILE_SIZE_1)
		{
			tileSize = TILE_SIZE_1;
			bytesPerValue = 1;
		}
		else if (tileFiles[i]->fileSize == 2 * TILE_SIZE_1 * TILE_SIZE_1)
		{
			tileSize = TILE_SIZE_1;
			bytesPerValue = 2;
		}
		else if (tileFiles[i]->fileSize == TILE_SIZE_3 * TILE_SIZE_3)
		{
			tileSize = TILE_SIZE_3;
			bytesPerValue = 1;
		}
		else if (tileFiles[i]->fileSize == 2 * TILE_SIZE_3 * TILE_SIZE_3)
		{
			tileSize = TILE_SIZE_3;
			bytesPerValue = 2;
		}

		DEMTileInfo tileInfo;
		tileInfo.botLeftLat = lat;
		tileInfo.botLeftLon = lon;
		tileInfo.stepLat = 1;
		tileInfo.stepLon = 1;

		tileInfo.width = tileSize;
		tileInfo.height = tileSize;
		tileInfo.bytesPerValue = bytesPerValue;
		tileInfo.fileName = tileFiles[i]->filePath;

		
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
	if (tileNodes != NULL)
	{		
		while (tileNodes)
		{
			DEMTileInfo di;
			di.fileName = tileNodes->Attribute("name");
			di.botLeftLat = atof(tileNodes->Attribute("lat"));
			di.botLeftLon = atof(tileNodes->Attribute("lon"));
			di.stepLat = atof(tileNodes->Attribute("step_lat"));
			di.stepLon = atof(tileNodes->Attribute("step_lon"));
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
		tile->SetDoubleAttribute("lat", ti.botLeftLat);
		tile->SetDoubleAttribute("lon", ti.botLeftLon);
		tile->SetDoubleAttribute("step_lat", ti.stepLat);
		tile->SetDoubleAttribute("step_lon", ti.stepLon);
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

uint8_t * DEMData::BuildMap(int w, int h, double minLon, double minLat, double maxLon, double maxLat, bool keepAR)
{
	uint8_t * heightMap = new uint8_t[w * h];
	memset(heightMap, 0, w * h * sizeof(uint8_t));

	MapProjections mp = MapProjections(MapProjections::MERCATOR, w, h, minLon, minLat, maxLon, maxLat);
	mp.SetKeepAR(keepAR);

	DEMData::DEMArea area = this->GetTilesInArea(minLon, minLat, maxLon, maxLat);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			double lon = 0;
			double lat = 0;
			mp.ConvertCoordinatesInverse(x, y, &lon, &lat);

			short value = this->GetValue(area, lon, lat);
			uint8_t v = static_cast<uint8_t>(Utils::MapRange(this->minHeight, this->maxHeight, 0.0, 255.0, value));

			heightMap[x + y * w] = v;

		}
	}

	return heightMap;
}

DEMData::DEMArea DEMData::GetTilesInArea(double minLon, double minLat, double maxLon, double maxLat)
{
	DEMArea da(GPSPoint(minLon, minLat), GPSPoint(maxLon, maxLat));
	
	for (size_t i = 0; i < this->tiles.size(); i++)
	{
		int isInside = true;

		const DEMTileInfo * tile = &this->tiles[i];

		for (int j = 0; j < 4; j++)
		{
			if (this->IsInside(tile->GetCorner(j), da))
			{
				isInside = false;
				break;
			}
		}

		if (isInside == false)
		{
			continue;
		}

		if (this->cachedTiledData.find(*tile) != this->cachedTiledData.end())
		{
			da.tileData.push_back(this->cachedTiledData[*tile]);
		}
		else
		{
			DEMTileData * td = new DEMTileData(*tile);

			da.tileData.push_back(td);
			this->cachedTiledData[*tile] = td;
		}
	}

	return da;

}

bool DEMData::IsInside(const GPSPoint & p, DEMArea & area)
{

	if (p.lat < area.min.lat) return false;
	if (p.lat > area.max.lat) return false;

	if (p.lon < area.min.lon) return false;
	if (p.lon > area.max.lon) return false;

	return true;
}


short DEMData::GetValue(DEMArea & da, double lon, double lat)
{
	for (size_t i = 0; i < da.tileData.size(); i++)
	{
		const DEMTileInfo & ti = da.tileData[i]->info;

		if (ti.IsPointInside(lon, lat))
		{
			short value = da.tileData[i]->GetValue(lon, lat);
			return value;
		}

	}

	return 0;
}

