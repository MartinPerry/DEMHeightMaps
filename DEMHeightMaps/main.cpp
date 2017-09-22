
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

#include "./DB/Strings/MyString.h"
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

int main(int argc, char * argv[])
{
	//LoadTmp();
	//return 0;

	//std::shared_ptr<Equirectangular> proj = std::make_shared<Equirectangular>();
	std::shared_ptr<Mercator> proj = std::make_shared<Mercator>();
	
	//DEMData dd("H://DEM_Voidfill//");	
	//dd.ExportTileList("D://tiles.xml");

	//DEMData dd("H://DEM_Voidfill_debug//", "D://tiles_debug.xml");
	//DEMData dd({ "E://DEM_Voidfill_debug//" }, proj);
	//DEMData dd({ "E://debug_uk//" }, proj);
	//DEMData dd("E://DEM23//");
	DEMData dd({ "D://Heightmaps//DEM_Voidfill//", "D://Heightmaps//DEM_srtm//" }, proj);	
	//DEMData dd({ "F://DEM_Voidfill_opened//", "F://DEM_srtm_opened//" }, proj);
	//DEMData dd({ "E://DEM_srtm//" }, proj);
	dd.SetMinMaxElevation(0, 5000);

	//VFS::GetInstance()->ExportStructure("d://vfs.txt");

	//dd.ExportTileList("D://tile_list.xml");

	int zoomLevel = 1;

	double stepLat = (MERCATOR_MAX - MERCATOR_MIN) / (std::pow(2.0, zoomLevel));
	double stepLon = (180.0 - -180.0) / (std::pow(2.0, zoomLevel));

	auto tiles = dd.BuildTileMap(512, 512, 
	{ GeoCoordinate::deg(MERCATOR_MIN), GeoCoordinate::deg(-180.0) },
	{ GeoCoordinate::deg(MERCATOR_MAX), GeoCoordinate::deg(180.0) },
	{ GeoCoordinate::deg(stepLat), GeoCoordinate::deg(stepLon) });


	//BorderRenderer br("I://hranice//", proj);
	
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

	return 0;
	
	int w = 360 * 10;
	int h = 180 * 10;

	IProjectionInfo::Coordinate minc = { -90.0_deg, -180.0_deg };
	IProjectionInfo::Coordinate maxc = { 90.0_deg, 180.0_deg };

	//IProjectionInfo::Coordinate minc = { 35.0_deg, 42.0_deg };
	//IProjectionInfo::Coordinate maxc = { 48.0_deg, 57.0_deg };

	//uint8_t * data = dd.BuildMap(w, h, { 50.0_deg, -2.0_deg }, { 52.0_deg, 2.0_deg }, true);
	//uint8_t * data = dd.BuildMap(w, h, { 65.0_deg,  13.5_deg }, { 65.1_deg, 25.0_deg }, true);
	
	//uint8_t * data = dd.BuildMap(w, h, { 31.0_deg, -27.0_deg }, { 58.0_deg, 47.0_deg}, true);
	//unsigned char * data = dd.BuildMap(w, h, {51.15_deg, 12.007_deg} {48.0_deg, 18.9999_deg}, true);

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


	return 0;
}