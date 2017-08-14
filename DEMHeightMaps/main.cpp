
#include <lodepng.h>

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"

#include "DEMTile.h"
#include "DEMData.h"
#include "BorderRenderer.h"

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

int main(int argc, char * argv[])
{
	//LoadTmp();
	//return 0;

	int w = 8000;
	int h = 4000;


	//DEMData dd("H://DEM_Voidfill//");	
	//dd.ExportTileList("D://tiles.xml");

	//DEMData dd("H://DEM_Voidfill_debug//", "D://tiles_debug.xml");
	//DEMData dd("E://DEM_Voidfill_debug//");
	//DEMData dd("E://DEM23//");
	DEMData dd("E://DEM_Voidfill//");
	dd.SetMinMaxElevation(0, 5000);
	
	uint8_t * data = dd.BuildMap(w, h, { 31.0_deg, -27.0_deg }, /**/{ 58.0_deg, 47.0_deg}, true);
	//unsigned char * data = dd.BuildMap(w, h, {51.15_deg, 12.007_deg} /**/ {48.0_deg, 18.9999_deg}, true);

	Utils::SaveToFile<uint8_t>(data, w * h, "D://rrr.raw");
	//-------------------------------------------------------------
	//-------------------------------------------------------------
	//-------------------------------------------------------------

	//return 0;
	//unsigned char * data = new unsigned char[w * h];
	//memset(data, 0, w * h);
	BorderRenderer br("I://hranice//");
	br.SetData(w, h, data);
	br.DrawBorders({ 31.0_deg, -27.0_deg }, /**/{ 58.0_deg, 47.0_deg }, true);		 //Evropa
	//br.DrawBorders({51.15_deg, 12.007_deg} /**/ {48.0_deg, 18.9999_deg}, true); //CZ
	Utils::SaveToFile(data, w * h, "D://rrr.raw");
	
	
	uint32_t error = lodepng::encode("D://rrr.png", data, w, h, LodePNGColorType::LCT_GREY, 8);


	return 0;
}