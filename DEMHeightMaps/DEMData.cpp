#include "./DEMData.h"

#include <algorithm>

#include <MapProjection.h>
#include <GeoCoordinate.h>
#include <Projections.h>

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"
#include "./TinyXML/tinyxml.h"


//=======================================================================================
// ctors & dtor
//=======================================================================================

DEMData::DEMData(std::initializer_list<std::string> dirs, std::shared_ptr<IProjectionInfo> projection)
{	
	this->tiles2Dmap.resize(360 * 180); //resolution 1 degree

	this->projection = projection;

	this->minHeight = 0;
	this->maxHeight = 9000;

	VFS::Initialize(DEBUG_MODE);
	for (auto d : dirs)
	{
		VFS::GetInstance()->AddDirectory(d);		
	}

	this->LoadTiles();
}

DEMData::DEMData(const std::string & dir, const std::string & tilesInfoXML, std::shared_ptr<IProjectionInfo> projection)
{
	this->tiles2Dmap.resize(360 * 180); //resolution 1 degree

	this->projection = projection;

	this->minHeight = 0;
	this->maxHeight = 9000;

	
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

void DEMData::LoadTiles()
{	
	std::vector<VFS_FILE *> tileFiles = VFS::GetInstance()->GetAllFiles();

	
	//map file names are: lat | lon - down left corner (south-west)
	//eg: N35W001.hgt

	for (const auto & file : tileFiles)
	{
		TileInfo::SOURCE src = TileInfo::HGT;
		const std::string & fileName = file->name;

		if (fileName.length() < 7)
		{
			printf("Incorrect tile file: %s\n", file->filePath);
			continue;
		}
		

		int lat = 10 * (fileName[1] - '0') + (fileName[2] - '0');
		if ((fileName[0] == 'S') || (fileName[0] == 's'))
		{
			lat *= -1;
		}


		int lon = 0;
		if (fileName[3] == '_') 
		{
			src = TileInfo::BIL;
			lon = 100 * (fileName[5] - '0') + 10 * (fileName[6] - '0') + (fileName[7] - '0');
			if ((fileName[4] == 'W') || (fileName[4] == 'w'))
			{
				lon *= -1;
			}
		}
		else 
		{
			lon = 100 * (fileName[4] - '0') + 10 * (fileName[5] - '0') + (fileName[6] - '0');
			if ((fileName[3] == 'W') || (fileName[3] == 'w'))
			{
				lon *= -1;
			}
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
		else 
		{
			continue;
		}

		DEMTileInfo tileInfo;
		tileInfo.minLat = GeoCoordinate::deg(lat);
		tileInfo.minLon = GeoCoordinate::deg(lon);
		tileInfo.stepLat = GeoCoordinate::deg(1);
		tileInfo.stepLon = GeoCoordinate::deg(1);
		tileInfo.pixelStepLat = GeoCoordinate::deg(1.0 / (tileSize - 1)); //-1 -> we are counting "between" pixels, not pixels
		tileInfo.pixelStepLon = GeoCoordinate::deg(1.0 / (tileSize - 1));

		tileInfo.width = tileSize;
		tileInfo.height = tileSize;
		tileInfo.bytesPerValue = bytesPerValue;
		tileInfo.fileName = file->filePath;				
		tileInfo.source = src;
		
		this->AddTile(tileInfo);
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
			di.minLat = GeoCoordinate::deg(atof(tileNodes->Attribute("lat")));
			di.minLon = GeoCoordinate::deg(atof(tileNodes->Attribute("lon")));
			di.stepLat = GeoCoordinate::deg(atof(tileNodes->Attribute("step_lat")));
			di.stepLon = GeoCoordinate::deg(atof(tileNodes->Attribute("step_lon")));
			di.width = atoi(tileNodes->Attribute("w"));
			di.height = atoi(tileNodes->Attribute("h"));
			di.bytesPerValue = atoi(tileNodes->Attribute("b"));

			this->AddTile(di);

			tileNodes = tileNodes->NextSiblingElement("tile");
		}


	}

}

void DEMData::AddTile(const DEMTileInfo & ti)
{
	int lon = static_cast<int>(ti.minLon.deg());
	int lat = static_cast<int>(ti.minLat.deg());
	
	//move it to + intervals
	lat += 90;
	lon += 180;

	size_t index = lon + lat * 180;

	for (auto & t : this->tiles2Dmap[index])
	{
		if ((t.minLat.rad() == ti.minLat.rad()) && (t.minLon.rad() == ti.minLon.rad()))
		{
			if (t.width * t.height < ti.width * ti.height)
			{
				//replace with "larger tile"
				t.bytesPerValue = ti.bytesPerValue;
				t.width = ti.width;
				t.height = ti.height;
				t.fileName = ti.fileName;
				t.stepLon = ti.stepLon;
				t.stepLat = ti.stepLat;
				t.source = ti.source;
				return;
			}
		}
	}

	this->tiles2Dmap[index].push_back(ti);
}

void DEMData::ExportTileList(const std::string & fileName)
{
	TiXmlDocument doc;
	
	TiXmlElement * root = new TiXmlElement("dem");
	doc.LinkEndChild(root);
	
	TiXmlElement * tile;
	for (size_t i = 0; i < this->tiles2Dmap.size(); i++)
	{
		for (const DEMTileInfo & ti : this->tiles2Dmap[i])
		{
			tile = new TiXmlElement("tile");
			tile->SetAttribute("name", ti.fileName.c_str());
			tile->SetDoubleAttribute("lat", ti.minLat.deg());
			tile->SetDoubleAttribute("lon", ti.minLon.deg());
			tile->SetDoubleAttribute("step_lat", ti.stepLat.deg());
			tile->SetDoubleAttribute("step_lon", ti.stepLon.deg());
			tile->SetAttribute("w", ti.width);
			tile->SetAttribute("h", ti.height);
			tile->SetAttribute("b", ti.bytesPerValue);
			root->LinkEndChild(tile);
		}
	}
	
	doc.SaveFile(fileName.c_str());
}

//=======================================================================================
// Data obtaining
//=======================================================================================

std::vector<TileInfo> DEMData::BuildTileMap(int tileW, int tileH, 
	const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, 
	const IProjectionInfo::Coordinate & tileStep)
{
	//calculate "full" resolution
	//uint32_t lonLength = static_cast<uint32_t>(std::ceil((max.lon.rad() - min.lon.rad()) / pixelStep.lon.rad()));
	//uint32_t latLength = static_cast<uint32_t>(std::ceil((max.lat.rad() - min.lat.rad()) / pixelStep.lat.rad()));

	std::vector<TileInfo> res;


	for (double lat = min.lat.rad(); lat <= max.lat.rad(); lat += tileStep.lat.rad())
	{
		bool breakLat = false;
		if (lat >= max.lat.rad())
		{
			lat = max.lat.rad();
			breakLat = true;
		}

		for (double lon = min.lon.rad(); lon <= max.lon.rad(); lon += tileStep.lon.rad())
		{		
			bool breakLon = false;
			if (lon >= max.lon.rad())
			{
				lon = max.lon.rad();
				breakLon = true;
			}

			TileInfo ti;

			ti.width = tileW;
			ti.height = tileH;
			ti.minLon = GeoCoordinate::rad(lon);
			ti.minLat = GeoCoordinate::rad(lat);
			ti.stepLon = tileStep.lon;
			ti.stepLat = tileStep.lat;

			res.push_back(ti);

			if (breakLon)
			{
				break;
			}
		}

		if (breakLat)
		{
			break;
		}
	}

	return res;
}

uint8_t * DEMData::BuildMap(int w, int h, const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, bool keepAR)
{
	printf("BUILD map Lon: %f %f / Lat: %f %f\n", min.lon.deg(), max.lon.deg(), min.lat.deg(), max.lat.deg());

	uint8_t * heightMap = new uint8_t[w * h];
	memset(heightMap, 0, w * h * sizeof(uint8_t));
		
	this->projection->SetFrame(min, max, w, h, keepAR);
		
	std::vector<DEMTileInfo *> pixelTiles;
	std::unordered_map<DEMTileInfo *, std::vector<size_t>> tilePixels;

	pixelTiles.resize(w * h, nullptr);
	
	std::vector<IProjectionInfo::Coordinate> coords;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{				
			IProjectionInfo::Coordinate c = this->projection->ProjectInverse({ x, y });
			coords.push_back(c);

			DEMTileInfo * ti = this->GetTile(c);
			if (ti == nullptr)
			{
				continue;
			}

			pixelTiles[x + y * w] = ti;
			tilePixels[ti].push_back(x + y * w);			
		}
	}

	printf("Tiles count: %i \n", tilePixels.size());

	int count = 0;
	int lastProgress = 0;
	for (auto ti : tilePixels)
	{
		DEMTileData td;
		td.SetTileInfo(*ti.first);
		td.LoadTileData();

		for (size_t i = 0; i < ti.second.size(); i++)
		{
			size_t index = ti.second[i];
			
			
			short value = td.GetValue(coords[index]);
			uint8_t v = static_cast<uint8_t>(Utils::MapRange(this->minHeight, this->maxHeight, 0.0, 255.0, value));

			heightMap[index] = v;			
		}

		td.ReleaseData();	

		double progress = ((static_cast<double>(count) / tilePixels.size()) * 100.0);
		if (static_cast<int>(progress) != lastProgress)
		{
			printf("\rProgress: %i %%", static_cast<int>(progress));
			fflush(stdout);
			
			lastProgress = static_cast<int>(progress);
		}
		count++;
	}
	

	/*
	for (int y = 0; y < h; y++)
	{		
		for (int x = 0; x < w; x++)
		{
			//ParallelFor(0, w, [&](int x) {

				size_t index = x + y * w;
				
				short value = pixelTiles[index]->GetValue(coords[index]);
				uint8_t v = static_cast<uint8_t>(Utils::MapRange(this->minHeight, this->maxHeight, 0.0, 255.0, value));

				heightMap[index] = v;
			//});
		}
	}
	*/
	printf("\nMap builded\n");

	return heightMap;
}

DEMTileInfo * DEMData::GetTile(const IProjectionInfo::Coordinate & c)
{
	int lon = static_cast<int>(c.lon.deg());
	int lat = static_cast<int>(c.lat.deg());

	lat += 90;
	lon += 180;

	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			for (auto & t : this->tiles2Dmap[(lon + x) + (lat + y) * 180])
			{
				if (t.IsPointInside(c))
				{
					return &t;
				}
			}
		}
	}
	

	return nullptr;
}
