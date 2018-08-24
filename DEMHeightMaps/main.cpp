
#include <memory>
#include <lodepng.h>

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"

#include "DEMTile.h"
#include "DEMData.h"
#include "BorderRenderer.h"

#include <Projections.h>
#include <MapProjection.h>
#include <GeoCoordinate.h>

#include "./Strings/MyString.h"

#include "./DB/Database/PostgreSQLWrapper.h"
#include "./DB/Database/SQLSelect.h"
#include "./DB/Database/SQLInsert.h"
#include "./DB/Database/SQLPreparedStatement.h"
#include "./DB/Database/PostGis.h"
#include "./DB/Database/PostGisRaster.h"

/*
void LoadTmp()
{
	MyStringAnsi dd = MyStringAnsi::LoadFromFile("D://data_dem.htm");

	std::vector<int> ddx = dd.FindAll("href");

	std::vector<MyStringAnsi> cc;
	for (int i = 0; i < ddx.size(); i++)
	{
		int offset = ddx[i];

		MyStringAnsi ss = dd.SubString(offset + 6);
		int of = ss.Find("zip");

		ss = ss.SubString(0, of);
		ss += "zip";

		cc.push_back(ss);

	}

	VFS::Initialize("H://DEM_Voidfill", DEBUG_MODE);
	//std::vector<MyStringAnsi> ii = VFS::GetInstance()->GetAllFiles();
	std::vector<VFS_FILE *> ii = VFS::GetInstance()->GetMainFiles();


	std::unordered_set<MyStringAnsi> oo;
	for (int j = 0; j < ii.size(); j++)
	{
		MyStringAnsi rr = ii[j]->filePath;

		rr.Replace("H:\\DEM_Voidfill\\", "http://viewfinderpanoramas.org/dem3/");

		oo.insert(rr);
	}


	MyStringAnsi output = "";
	for (int j = 0; j < cc.size(); j++)
	{
		if (oo.find(cc[j]) != oo.end())
		{
			continue;
		}

		oo.insert(cc[j]);

		output += cc[j];
		output += "\n";
	}
	output.SaveToFile("D://dem_dl.txt");
	printf("x");
}
*/

//blbe jsou asi ty detailnejsi dlazdice - nejak otocene ? wtf
//diry v mape ? obcas nejaky bug - sediva cara
//prumerovani z okoli - ?funguje? pomale
//nahrat dlazdice do RAM disku misto na ssd ?


#define MERCATOR_MAX 85.051 //15
#define MERCATOR_MIN -85.051 //15


void CreateBackgroundMaps()
{	
	//DEMData dd("H://DEM_Voidfill_debug//", "D://tiles_debug.xml");
	//DEMData dd({ "E://DEM_Voidfill_debug//" });
	//DEMData dd({ "E://debug_uk//" });
	//DEMData dd("E://DEM23//");
	//DEMData<uint8_t, Projections::Mercator> dd({ "F://DEM_Voidfill_opened//", "F://DEM_srtm_opened//" });
	DEMData<uint8_t, Projections::Mercator> dd({ "E://DEM_Voidfill//", "E://DEM_srtm//" });
	//DEMData dd({ "F://DEM_Voidfill_opened//", "F://DEM_srtm_opened//" });
	//DEMData dd({ "E://DEM_srtm//" });

	dd.SetVerboseEnabled(true);

	printf("Data inited\n");

	dd.SetMinMaxElevation(0, 5000);
	dd.SetElevationMappingEnabled(true);

	int zoomLevel = 1;

	double stepLat = (MERCATOR_MAX - MERCATOR_MIN) / (std::pow(2.0, zoomLevel));
	double stepLon = (180.0 - -180.0) / (std::pow(2.0, zoomLevel));

	auto tiles = dd.BuildTileMap(512, 512,
	{ GeoCoordinate::deg(MERCATOR_MIN), GeoCoordinate::deg(-180.0) },
	{ GeoCoordinate::deg(MERCATOR_MAX), GeoCoordinate::deg(180.0) },
	{ GeoCoordinate::deg(stepLat), GeoCoordinate::deg(stepLon) });
	
	//BorderRenderer<Projections::Mercator> br("I://hranice//", dd.GetProjection());

	for (auto & tDir : tiles)
	{
		for (auto & tFiles : tDir.second)
		{
			auto & t = tFiles.second;
			uint8_t * data = dd.BuildMap(t.width, t.height, t.GetCorner(0), t.GetCorner(3), false);

			//br.SetData(t.width, t.height, data);
			//br.DrawBorders(t.GetCorner(0), t.GetCorner(3), false);

			MyStringAnsi ss = "D://T//DEM//";
			/*
			ss += "tile_";
			ss += int(t.minLat.deg() * 10) / 10.0;
			ss += "_";
			ss += int(t.minLon.deg() * 10) / 10.0;
			ss += "_";
			ss += int(t.GetCorner(3).lat.deg() * 10) / 10.0;
			ss += "_";
			ss += int(t.GetCorner(3).lon.deg() * 10) / 10.0;
			*/
			ss += "tile_";
			ss += tDir.first;
			ss += "_";
			ss += tFiles.first;
			ss += ".png";
			uint32_t error = lodepng::encode(ss.c_str(), data, t.width, t.height, LodePNGColorType::LCT_GREY, 8);

			SAFE_DELETE_ARRAY(data);
		}
	}
}



static std::string * uint16_tToString = new std::string[10000];
static std::string * uint16_tToStringWithComa = new std::string[10000];
static std::string * uint16_tToStringWithOpen = new std::string[10000];
static std::string * uint16_tToStringWithCloseComa = new std::string[10000];


std::string BuildDataAsStr(uint16_t * data, int w, int h)
{
	std::string newData = "";
	newData.reserve(w * h * 5);

	for (int y = 0; y < (h - 1); y++)
	{
		int yw = y * w;

		newData += uint16_tToStringWithOpen[data[0 + yw]];
		for (int x = 1; x < (w - 1); x++)
		{			
			newData += uint16_tToStringWithComa[data[x + yw]];
		}		
		newData += uint16_tToStringWithCloseComa[data[(w - 1) + yw]];		
	}

	newData += uint16_tToStringWithOpen[data[0 + (h - 1) * w]];
	for (int x = 1; x < (w - 1); x++)
	{		
		newData += uint16_tToStringWithComa[data[x + (h - 1) * w]];
	}	
	newData += uint16_tToString[data[(w - 1) + (h - 1) * w]];
	newData += "]";
	
	return newData;
}

int main(int argc, char * argv[])
{
	CreateBackgroundMaps();

	return 0;

	for (uint16_t v = 0; v < 10000; v++)
	{
		uint16_tToString[v] = std::to_string(v);

		uint16_tToStringWithComa[v] = std::to_string(v);
		uint16_tToStringWithComa[v] += ", ";

		uint16_tToStringWithCloseComa[v] = std::to_string(v);;
		uint16_tToStringWithCloseComa[v] += "], ";

		uint16_tToStringWithOpen[v] = "[";
		uint16_tToStringWithOpen[v] += std::to_string(v);;
		uint16_tToStringWithOpen[v] += ", ";
	}

	PostgreSQLWrapper psql = PostgreSQLWrapper();
	if (psql.Connect("localhost", "postgres", "admin", "postgres", "5432") == false)
	{
		printf("failed to connect");
		return 0;
	}

	SQLInsert insert("height_maps", { "raw_data"});




	//LoadTmp();
	//return 0;

	
	//DEMData dd("H://DEM_Voidfill//");	
	//dd.ExportTileList("D://tiles.xml");

	//DEMData dd("H://DEM_Voidfill_debug//", "D://tiles_debug.xml");
	//DEMData dd({ "E://DEM_Voidfill_debug//" }, proj);
	//DEMData dd({ "E://debug_uk//" }, proj);
	//DEMData dd("E://DEM23//");
	DEMData<uint16_t, Projections::Equirectangular> dd({ "D://Heightmaps//DEM_Voidfill//", "D://Heightmaps//DEM_srtm//" });
	//DEMData<uint16_t> dd({ }, proj);
	//DEMData dd({ "F://DEM_Voidfill_opened//", "F://DEM_srtm_opened//" }, proj);
	//DEMData dd({ "E://DEM_srtm//" }, proj);

	//dd.SetMinMaxElevation(0, 9000);

	//VFS::GetInstance()->ExportStructure("d://vfs.txt");

	//dd.ExportTileList("D://tile_list.xml");

	//double stepLat = 80.0;
	//double stepLon = 180.0;

	/*
	//test na oblast vody
	auto tiles = dd.BuildTileMap(512, 512,
	{ GeoCoordinate::deg(52), GeoCoordinate::deg(-40) },
	{ GeoCoordinate::deg(56), GeoCoordinate::deg(-30) },
	{ GeoCoordinate::deg(0.5), GeoCoordinate::deg(0.5) });

	for (auto & tDir : tiles)
	{
		for (auto & tFiles : tDir.second)
		{
			auto & t = tFiles.second;
			auto * data = dd.BuildMap(t.width, t.height, t.GetCorner(0), t.GetCorner(3), false);

			if (data != nullptr)
			{
				printf("neni voda");
			}

			delete[] data;
			//br.SetData(t.width, t.height, data);

		}
	}
	*/




	double stepLat = 0.0025 * 64;// (90.0 - -90.0) / (std::pow(2.0, zoomLevel));
	double stepLon = 0.0025 * 64;// (180.0 - -180.0) / (std::pow(2.0, zoomLevel));

	
	dd.ProcessTileMap(64, 64,
	{ GeoCoordinate::deg(-90.0),  GeoCoordinate::deg(-180.0) },
	{ GeoCoordinate::deg(90.0), GeoCoordinate::deg(180.0) },
	{ GeoCoordinate::deg(stepLat), GeoCoordinate::deg(stepLon) },
		[&](TileInfo & t, size_t x, size_t y) {

		double latDeg = t.GetCorner(0).lat.deg();
		double lonDeg = t.GetCorner(0).lon.deg();

		if (lonDeg == static_cast<double>(static_cast<int>(lonDeg)))
		{
			printf("Lon: %f Lat: %f\n", lonDeg, latDeg);
		}
		else if (latDeg == static_cast<double>(static_cast<int>(latDeg)))
		{
			printf("Lon: %f Lat: %f\n", lonDeg, latDeg);
		}

		uint16_t * data = dd.BuildMap(t.width, t.height, t.GetCorner(0), t.GetCorner(3), false);
		//uint16_t * data = new uint16_t[t.width * t.height];
		//memset(data, 0, t.width * t.height * sizeof(uint16_t));

		if (data == nullptr)
		{
			//tile is empty
			//probably "water only" tile
			return;
		}

		auto tmp = t.GetCorner(2);
		double upperLeftX = tmp.lon.deg();
		double upperLeftY = tmp.lat.deg();
			
		std::string strData = BuildDataAsStr(data, t.width, t.height);

		std::string q = "INSERT INTO height_maps (raw_data) "
			"VALUES ("
			"ST_SetValues("
				"ST_AddBand("
					"ST_MakeEmptyRaster(";
						q += std::to_string(t.width);
						q += ", ";
						q += std::to_string(t.height);
						q += ", ";
						q += std::to_string(upperLeftX);
						q += ", ";
						q += std::to_string(upperLeftY);
						q += ", ";
						q += std::to_string(t.pixelStepLon.deg());
						q += ", ";
						q += std::to_string(-t.pixelStepLat.deg());
						q += ", 0, 0, 4326"
					"), "
					"1, '16BUI', 0, 0"
				"), "
				"1, 1, 1, ARRAY[";
					q += strData;
				q += "]::double  precision[][]"
			") "
			")";

			psql.RunQuery(q);

			SAFE_DELETE_ARRAY(data);
		}
	);

	delete[] uint16_tToString;
	delete[] uint16_tToStringWithComa;
	delete[] uint16_tToStringWithCloseComa;
	delete[] uint16_tToStringWithOpen;


	/*
	
	
	WITH ref As
	(
		SELECT (ST_SetSRID(ST_Point(lon, lat), 4326)) AS pt
	) SELECT 
    lon, lat, 
    ST_Value(raw_data, 1, ref.pt) AS val
  FROM
    height_maps 	
  CROSS JOIN ref
  WHERE ST_Intersects(height_maps.raw_data, ref.pt)

	
	*/

	/*
	SELECT (md).* FROM 
(
SELECT ST_MetaData(raw_data) As md FROM height_maps
) as foo
	*/
	
	return 0;
	
	int w = 360 * 10;
	int h = 180 * 10;

	Projections::Coordinate minc = { -90.0_deg, -180.0_deg };
	Projections::Coordinate maxc = { 90.0_deg, 180.0_deg };

	//IProjectionInfo::Coordinate minc = { 35.0_deg, 42.0_deg };
	//IProjectionInfo::Coordinate maxc = { 48.0_deg, 57.0_deg };

	//uint8_t * data = dd.BuildMap(w, h, { 50.0_deg, -2.0_deg }, { 52.0_deg, 2.0_deg }, true);
	//uint8_t * data = dd.BuildMap(w, h, { 65.0_deg,  13.5_deg }, { 65.1_deg, 25.0_deg }, true);
	
	//uint8_t * data = dd.BuildMap(w, h, { 31.0_deg, -27.0_deg }, { 58.0_deg, 47.0_deg}, true);
	//unsigned char * data = dd.BuildMap(w, h, {51.15_deg, 12.007_deg} {48.0_deg, 18.9999_deg}, true);

	/*
	uint8_t * data = dd.BuildMap(w, h, minc, maxc, true);
	Utils::SaveToFile<uint8_t>(data, w * h, "D://rrr.raw");
	lodepng::encode("D://rrr.png", data, w, h, LodePNGColorType::LCT_GREY, 8);

	//-------------------------------------------------------------
	//-------------------------------------------------------------
	//-------------------------------------------------------------
	//return 0;
	//return 0;
	//unsigned char * data = new unsigned char[w * h];
	//memset(data, 0, w * h);
	BorderRenderer br2("I://hranice//", proj);
	br2.SetData(w, h, data);
	br2.DrawBorders(minc, maxc, true);


	//br2.DrawBorders({ 50.0_deg, -2.0_deg }, { 52.0_deg, 2.0_deg }, true);
	//br2.DrawBorders({ 65.0_deg,  13.5_deg }, { 65.1_deg, 25.0_deg }, true);		 //Evropa
	
	//br2.DrawBorders( { 31.0_deg, -27.0_deg }, { 58.0_deg, 47.0_deg}, true);		 //Evropa
	//br.DrawBorders({51.15_deg, 12.007_deg} {48.0_deg, 18.9999_deg}, true); //CZ
	//Utils::SaveToFile(data, w * h, "D://rrr.raw");
	
	
	uint32_t error = lodepng::encode("D://rrr_border.png", data, w, h, LodePNGColorType::LCT_GREY, 8);
	*/

	return 0;
}