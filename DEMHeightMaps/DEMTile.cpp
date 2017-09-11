#include "./DEMTile.h"

#include <vector>
#include <limits>

#include "./VFS/VFS.h"
#include "./Utils/Utils.h"


DEMTileData::~DEMTileData()
{
	this->ReleaseData();
}

void DEMTileData::ReleaseData()
{

	delete[](this->data);
	this->data = nullptr;
	this->dataSize = 0;
}

void DEMTileData::SetTileInfo(const DEMTileInfo & info)
{
	this->info = info;
	this->data = nullptr;
}


//Data are overlaping from neighboring tiles at the borders
//https://www.orekit.org/forge/projects/rugged/wiki/DirectLocationWithDEM
short DEMTileData::GetValue(const IProjectionInfo::Coordinate & c)
{		
	double difLon = c.lon.rad() - this->info.minLon.rad();
	double difLat = c.lat.rad() - this->info.minLat.rad();

	if ((difLon < 0) || (difLon > this->info.stepLon.rad()))
	{
		//outside
		return 0;// std::numeric_limits<T>::max();
	}
	if ((difLat < 0) || (difLat > this->info.stepLat.rad()))
	{
		//outside
		return 0;// std::numeric_limits<T>::max();
	}

	//lazy loading data - load data, when we really need them, not before
	this->LoadTileData();

	double x = difLon / this->info.pixelStepLon.rad(); 
	double y = difLat / this->info.pixelStepLat.rad();

	//tiles are "horizontally" flipped... [0,0] is at top left, not bottom left, where is minimal lon/lat
	int index = static_cast<int>(x) + (this->info.height - 1 - static_cast<int>(y)) * this->info.width;

	
	short value = this->data[index];	
	
	if (this->info.source == TileInfo::HGT)
	{
		short b = (value >> 8) & 0xff;  // next byte, bits 8-15
		short a = value & 0xff;  // low-order byte: bits 0-7

		value = 256 * a + b;
	}
	else 
	{		
		if (value < 0)
		{
			value = 0;
		}				
	}

	return value;
}

void DEMTileData::LoadTileData()
{

	if (this->data != nullptr)
	{
		return;
	}
	
	char * tileData = VFS::GetInstance()->GetFileContent(this->info.fileName, &dataSize);
	this->data = reinterpret_cast<short *>(tileData);	
}

