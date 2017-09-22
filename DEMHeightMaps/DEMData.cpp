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

DEMData::DEMData(std::initializer_list<std::string> dirs, const std::string & tilesInfoXML, std::shared_ptr<IProjectionInfo> projection)
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
			std::string src = tileNodes->Attribute("source");;

			DEMTileInfo di;
			di.fileName = tileNodes->Attribute("name");
			di.minLat = GeoCoordinate::deg(atof(tileNodes->Attribute("lat")));
			di.minLon = GeoCoordinate::deg(atof(tileNodes->Attribute("lon")));
			di.stepLat = GeoCoordinate::deg(atof(tileNodes->Attribute("step_lat")));
			di.stepLon = GeoCoordinate::deg(atof(tileNodes->Attribute("step_lon")));
			di.pixelStepLat = GeoCoordinate::deg(atof(tileNodes->Attribute("pixel_step_lat")));
			di.pixelStepLon = GeoCoordinate::deg(atof(tileNodes->Attribute("pixel_step_lon")));
			di.width = atoi(tileNodes->Attribute("w"));
			di.height = atoi(tileNodes->Attribute("h"));
			di.bytesPerValue = atoi(tileNodes->Attribute("b"));
			di.source = DEMTileInfo::HGT;
			if (src == "bil")
			{
				di.source = DEMTileInfo::BIL;
			}

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

				t.pixelStepLat = ti.pixelStepLat;
				t.pixelStepLon = ti.pixelStepLon;

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
			tile->SetDoubleAttribute("pixel_step_lat", ti.pixelStepLat.deg());
			tile->SetDoubleAttribute("pixel_step_lon", ti.pixelStepLon.deg());
			tile->SetAttribute("w", ti.width);
			tile->SetAttribute("h", ti.height);
			tile->SetAttribute("b", ti.bytesPerValue);
			if (ti.source == DEMTileInfo::HGT) tile->SetAttribute("source", "hgt");
			else if (ti.source == DEMTileInfo::BIL) tile->SetAttribute("source", "bil");
			root->LinkEndChild(tile);
		}
	}
	
	doc.SaveFile(fileName.c_str());
}

//=======================================================================================
// Data obtaining
//=======================================================================================

std::unordered_map<size_t, std::unordered_map<size_t, TileInfo>> DEMData::BuildTileMap(int tileW, int tileH,
	const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, 
	const IProjectionInfo::Coordinate & tileStep)
{
	//calculate "full" resolution
	//uint32_t lonLength = static_cast<uint32_t>(std::ceil((max.lon.rad() - min.lon.rad()) / pixelStep.lon.rad()));
	//uint32_t latLength = static_cast<uint32_t>(std::ceil((max.lat.rad() - min.lat.rad()) / pixelStep.lat.rad()));

	std::unordered_map<size_t, std::unordered_map<size_t, TileInfo>> res;

	
	size_t y = 0; //file

	for (double lat = min.lat.rad(); lat < max.lat.rad(); lat += tileStep.lat.rad())
	{
		bool breakLat = false;
		if (lat >= max.lat.rad())
		{
			lat = max.lat.rad();
			breakLat = true;
		}
		
		size_t x = 0; //dir
		for (double lon = min.lon.rad(); lon < max.lon.rad(); lon += tileStep.lon.rad())
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
			ti.pixelStepLat = GeoCoordinate::rad(tileStep.lat.rad() / tileH);
			ti.pixelStepLon = GeoCoordinate::rad(tileStep.lon.rad() / tileW);

			res[x][y] = ti;

			if (breakLon)
			{
				break;
			}

			x++;
		}

		if (breakLat)
		{
			break;
		}

		y++;
	}

	return res;
}

uint8_t * DEMData::BuildMap(int w, int h, const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, bool keepAR)
{
	printf("BUILD map Lon: %f %f / Lat: %f %f\n", min.lon.deg(), max.lon.deg(), min.lat.deg(), max.lat.deg());

	uint8_t * heightMap = new uint8_t[w * h];
	memset(heightMap, 0, w * h * sizeof(uint8_t));
		
	this->projection->SetFrame(min, max, w, h, keepAR);
		
	

	coords.clear();
	tilePixels.clear();
	//pixelTiles.clear();
	

	//pixelTiles.resize(w * h, nullptr);
	
	
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

			//pixelTiles[x + y * w] = ti;
			tilePixels[ti].push_back(x + y * w);	

			
		}
	}

	printf("Tiles count: %i \n", tilePixels.size());

	int count = 0;
	int lastProgress = 0;
	for (auto ti : tilePixels)
	{		
		DEMTileData td;
		td.SetTileInfo(ti.first);
		
		if (ti.second.size() > 10)
		{
			td.LoadTileData();
		}
		
		for (size_t i = 0; i < ti.second.size(); i++)
		{
			size_t index = ti.second[i];									

			heightMap[index] = static_cast<uint8_t>(this->GetHeight(td, index));
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

DEMData::Neighbors DEMData::GetCoordinateNeighbors(const IProjectionInfo::Coordinate & c, DEMTileInfo * ti)
{

	double stepLatRad = this->projection->GetStepLat().rad();
	double stepLonRad = this->projection->GetStepLon().rad();


	//calculate how many pixels is step in DEM tiles

	//pixel based step
	int samplingStepLon = 10;
	int samplingStepLat = 10;

	//calculate how any pixels is between two "pixels" and divide it with sampling step
	int stepLon = static_cast<int>((stepLonRad / ti->pixelStepLon.rad()) / samplingStepLon);
	int stepLat = static_cast<int>((stepLatRad / ti->pixelStepLat.rad()) / samplingStepLat);

	//offset start / end by step
	double startLat = c.lat.rad() - 0.5 * stepLat * ti->pixelStepLat.rad();
	double endLat = c.lat.rad() + 0.5 * stepLat * ti->pixelStepLat.rad();

	double startLon = c.lon.rad() - 0.5 * stepLon * ti->pixelStepLon.rad();
	double endLon = c.lon.rad() + 0.5 * stepLon * ti->pixelStepLon.rad();

	double dLat = samplingStepLat * ti->pixelStepLat.rad();
	double dLon = samplingStepLon * ti->pixelStepLon.rad();

	//iterate all coordinates in radius
	Neighbors ns;

	IProjectionInfo::Coordinate nc;
	for (double nLat = startLat; nLat <= endLat; nLat += dLat)
	{
		nc.lat = GeoCoordinate::rad(nLat);
		for (double nLon = startLon; nLon <= endLon; nLon += dLon)
		{
			nc.lon = GeoCoordinate::rad(nLon);

			DEMTileInfo * ti = this->GetTile(nc);
			if (ti == nullptr)
			{
				continue;
			}

			ns.neighborsCache[ti].push_back(ns.neighborCoord.size());
			ns.neighborCoord.push_back(nc);
		}
	}

	return ns;
}

short DEMData::GetHeight(DEMTileData & td, int index)
{
	double value = 0;


	const IProjectionInfo::Coordinate & c = coords[index];
	value = td.GetValue(c);

	/*
	//with neighbors
	int count = 1;

	//get all neighbors for given pixel at [index]
	auto & n = this->GetCoordinateNeighbors(coords[index], td.GetTileInfo());

	//printf("Neighbors for %d: %d\n", index, n.neighborCoord.size());

	//iterate neighbors cache ordered by tile
	for (auto & nt : n.neighborsCache)
	{
		DEMTileData td;
		td.SetTileInfo(nt.first);
		td.LoadTileData();

		//for each tile, go over "positions" within this tile
		for (size_t i = 0; i < nt.second.size(); i++)
		{
			size_t nIndex = nt.second[i];
			value += td.GetValue(n.neighborCoord[nIndex]);
			
			count++;
		}
	}

	value /= count;
	*/

	return static_cast<uint8_t>(Utils::MapRange(this->minHeight, this->maxHeight, 0.0, 255.0, value));
}

DEMTileInfo * DEMData::GetTile(const IProjectionInfo::Coordinate & c)
{
	int lon = static_cast<int>(c.lon.deg());
	int lat = static_cast<int>(c.lat.deg());

	lat += 90;
	lon += 180;

	int startX = (lon <= 0) ? 0 : -1;
	int startY = (lat <= 0) ? 0 : -1;

	int endX = (lon >= 360) ? 0 : 1;
	int endY = (lat >= 180) ? 0 : 1;

	for (int y = startY; y <= endY; y++)
	{
		for (int x = startX; x <= endX; x++)
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
