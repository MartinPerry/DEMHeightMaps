#include "./DEMTile.h"

#include <vector>
#include <limits>

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"


DEMTileData::DEMTileData(const DEMTileInfo & info)
{
	this->info = info;
	this->data = NULL;
}

DEMTileData::~DEMTileData()
{
	delete[] (this->data);
}

//Data are overlaping from neighboring tiles at the borders
//https://www.orekit.org/forge/projects/rugged/wiki/DirectLocationWithDEM
short DEMTileData::GetValue(double lon, double lat)
{		
	double difLon = lon - this->info.botLeftLon;
	double difLat = lat - this->info.botLeftLat;

	if ((difLon < 0) || (difLon > this->info.stepLon))
	{
		//outside
		return 0;// std::numeric_limits<T>::max();
	}
	if ((difLat < 0) || (difLat > this->info.stepLat))
	{
		//outside
		return 0;// std::numeric_limits<T>::max();
	}

	//lazy loading data - load data, when we really need them, not before
	this->LoadTileData();

	double x = difLon / (this->info.stepLon / this->info.width);
	double y = difLat / (this->info.stepLat / this->info.height);

	//tiles are "horizontally" flipped... [0,0] is at top left, not bottom left, where is minimal lon/lat
	int index = static_cast<int>(x) + (this->info.height - 1 - static_cast<int>(y)) * this->info.width;

	short value = this->data[index];	

	short b = (value >> 8) & 0xff;  // next byte, bits 8-15
	short a = value & 0xff;  // low-order byte: bits 0-7

	value = 256 * a + b;

	return value;
}

void DEMTileData::LoadTileData()
{
	if (this->data != nullptr)
	{
		return;
	}

	int fs = 0;
	char * tileData = VFS::GetInstance()->GetFileContent(this->info.fileName, &fs);
	this->data = reinterpret_cast<short *>(tileData);	
}

