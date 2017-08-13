#include "MapProjections.h"

#include <string>
#include <math.h>
#include <algorithm>

#include "./Utils/Utils.h"

#define MPI 3.141592653589793238462

#define cot(x)  (1.0 / tan(x))
#define sec(x)  (1.0 / cos(x))
#define sinc(x) (sin(x) / (x))
#define sgn(x)  ((x < 0) ? -1 : (x > 0))



MapProjections::MapProjections(PROJECTION_TYPE proj,
	int mapWidth, int mapHeight, 
	double minLon, double minLat, double maxLon, double maxLat)
{
	this->mapHeightRatio = 1;
	this->mapWidthRatio = 1;


	this->widthPadding = 0;
	this->heightPadding = 0;

	this->projection = proj;

	//Prepare for Lambert

	double p1 = DEG_TO_RAD(35);
	double p2 = DEG_TO_RAD(65);
	double refLat = DEG_TO_RAD(52);
		
	this->lambertN = log(cos(p1) * sec(p2));
	this->lambertN /= (log(tan(0.25 * MPI + 0.5 * p2) * cot(0.25 * MPI + 0.5 * p1)));

	this->lambertF = cos(p1) * pow(tan(0.25 * MPI + 0.5 * p1), this->lambertN);
	this->lambertF /= this->lambertN;

	this->lambertPhi0 = this->lambertF * pow(cot(0.25 * MPI + 0.5 * refLat), this->lambertN);	
	


	double p1azim = DEG_TO_RAD(30);
	this->lambertSinP1 = sin(p1azim);
	this->lambertCosP1 = cos(p1azim);			

	// Set extrema

	this->useGlobalRatio = true;

	this->SetMinMaxCoordinate(minLon, minLat, maxLon, maxLat);
	this->SetDimension(mapWidth, mapHeight);
	
}


MapProjections::~MapProjections()
{
}

void MapProjections::SetKeepAR(bool keepAr)
{
	this->useGlobalRatio = keepAr;
}

int MapProjections::GetWidth()
{
	return this->mapWidth;
}

int MapProjections::GetHeight()
{
	return this->mapHeight;
}



//http://en.wikipedia.org/wiki/Lambert_azimuthal_equal-area_projection
//http://mathworld.wolfram.com/LambertAzimuthalEqual-AreaProjection.html
//http://en.wikipedia.org/wiki/Winkel_tripel_projection

void MapProjections::SetMinMaxCoordinate(double minLon, double minLat, double maxLon, double maxLat)
{
	this->minX = 100000;
	this->maxX = -100000;
	this->minY = 100000;
	this->maxY = -100000;

	double x = 0;
	double y = 0;

	if (this->projection == MERCATOR) this->ProjectionMercator(minLon, minLat, &x, &y);
	else if (this->projection == GALL_STEREO) this->ProjectionGallStereo(minLon, minLat, &x, &y);
	else if (this->projection == LAMBERT_CONIC) this->ProjectionLambertConic(minLon, minLat, &x, &y);
	else if (this->projection == LAMBERT_AZIMUT) this->ProjectionLambertAzimut(minLon, minLat, &x, &y);
	else if (this->projection == WINKEL_TRIPEL) this->ProjectionWinkelTripel(minLon, minLat, &x, &y);

	this->minX = std::min(this->minX, x);
	this->minY = std::min(this->minY, y);

	double x2 = 0;
	double y2 = 0;

	if (this->projection == MERCATOR) this->ProjectionMercator(maxLon, maxLat, &x2, &y2);
	else if (this->projection == GALL_STEREO) this->ProjectionGallStereo(maxLon, maxLat, &x2, &y2);
	else if (this->projection == LAMBERT_CONIC) this->ProjectionLambertConic(maxLon, maxLat, &x2, &y2);
	else if (this->projection == LAMBERT_AZIMUT) this->ProjectionLambertAzimut(maxLon, maxLat, &x2, &y2);
	else if (this->projection == WINKEL_TRIPEL) this->ProjectionWinkelTripel(maxLon, maxLat, &x2, &y2);

	this->minX = std::min(this->minX, x2);
	this->minY = std::min(this->minY, y2);


	//-----------------------------------------------------------

	x = x - this->minX;
	y = y - this->minY;

	this->maxX = std::max(this->maxX, x);
	this->maxY = std::max(this->maxY, y);

	x2 = x2 - this->minX;
	y2 = y2 - this->minY;

	this->maxX = std::max(this->maxX, x2);
	this->maxY = std::max(this->maxY, y2);	

}

void MapProjections::SetDimension(int width, int height)
{
	this->mapWidth = width;
	this->mapHeight = height;
		
	this->mapWidthRatio = this->mapWidth / this->maxX;
	this->mapHeightRatio = this->mapHeight / this->maxY;

	// using different ratios for width and height will cause the map to be stretched. So, we have to determine
	// the global ratio that will perfectly fit into the given image dimension
	if (this->useGlobalRatio)
	{				
		double globalRatio = std::min(mapWidthRatio, mapHeightRatio);
		this->mapWidthRatio = globalRatio;
		this->mapHeightRatio = globalRatio;
	}
	
	this->widthPadding = (this->mapWidth - (this->mapWidthRatio * this->maxX)) * 0.5;
	this->heightPadding = (this->mapHeight - (this->mapHeightRatio * this->maxY)) * 0.5;

}

//========================================================================================================
//========================================================================================================
//========================================================================================================


void MapProjections::ProjectionMercator(double lon, double lat, double * x, double * y) const
{	
	*x = DEG_TO_RAD(lon);
	*y = log(tan((MPI / 4.0) + 0.5 * DEG_TO_RAD(lat)));
}


void MapProjections::ProjectionGallStereo(double lon, double lat, double * x, double * y) const
{
	double longitude = DEG_TO_RAD(lon);
	double latitude  = DEG_TO_RAD(lat);
		
	double R = 2;
	*x = R * longitude / sqrt(2.0);
	*y = R * (1 + sqrt(2.0) / 2) * tan(0.5 * latitude);
}

//http://en.wikipedia.org/wiki/Lambert_conformal_conic_projection
void MapProjections::ProjectionLambertConic(double lon, double lat, double * x, double * y) const
{
	double longitude = DEG_TO_RAD(lon - 10);
	double latitude  = DEG_TO_RAD(lat);
	
	
	double lambertPhi = this->lambertF * pow(cot(0.25 * MPI + 0.5 * latitude), this->lambertN);

	*x = lambertPhi * sin(this->lambertN * longitude);
	*y = this->lambertPhi0 - lambertPhi * cos(this->lambertN * longitude);
	
}

void MapProjections::ProjectionLambertAzimut(double lon, double lat, double * x, double * y) const
{
	double longitude = DEG_TO_RAD(lon - 20);
	double latitude  = DEG_TO_RAD(lat);

	double cosLon = 0;
	double cosLat = 0;
	double sinLat = 0;
	double sinLon = 0;
	
	cosLat = cos(latitude);
	sinLat = sin(latitude);
	
	cosLon = cos(longitude);
	sinLon = sin(longitude);
	
	double k = 1 + this->lambertSinP1 * sinLat + this->lambertCosP1 * cosLat * cosLon;

	k = sqrt(2.0 / k);

	*x = k * cosLat * sinLon;
	*y = k * (this->lambertCosP1 * sinLat - this->lambertSinP1 * cosLat * cosLon);

}

void MapProjections::ProjectionWinkelTripel(double lon, double lat, double * x, double * y) const
{
	double longitude = DEG_TO_RAD(lon);
	double latitude  = DEG_TO_RAD(lat);

	double cosLat = cos(latitude);
	double cosHalfLon = cos(0.5 * longitude);
	double sinHalfLon = sin(0.5 * longitude);

	double sinLat = sin(latitude);
	

	double alfa = acos(cosLat * cosHalfLon);
	double sinAlfa = 1;
	if (alfa != 0)
	{
		sinAlfa = sinc(alfa);
	}

	//double cosParalel = cos(acos(2.0 / MPI));
	double cosParalel = 0.636619772;

	//----

	
	double tmp = 2 * cosLat * sinHalfLon;
	tmp /= sinAlfa;
	
	*x = 0.5 * (longitude * cosParalel + tmp);
		
	*y = 0.5 * (latitude +  (sinLat / sinAlfa));
	
}

/*-----------------------------------------------------------
Function:	ConvertCoordinates
Parameters:
	[in] lon - longitude (x)
	[in] lat - latitude (y)
	[out] x
	[out] y

Convert GPS coordinates to image pixel coordinates.


http://stackoverflow.com/questions/5983099/converting-longitude-latitude-to-x-y-coordinate
-------------------------------------------------------------*/
void MapProjections::ConvertCoordinates(double lon, double lat, double * x, double * y)
{
	double tmpX = 0;
	double tmpY = 0;

	if (this->projection == MERCATOR) this->ProjectionMercator(lon, lat, &tmpX, &tmpY);
	else if (this->projection == GALL_STEREO) this->ProjectionGallStereo(lon, lat, &tmpX, &tmpY);
	else if (this->projection == LAMBERT_CONIC) this->ProjectionLambertConic(lon, lat, &tmpX, &tmpY);
	else if (this->projection == LAMBERT_AZIMUT) this->ProjectionLambertAzimut(lon, lat, &tmpX, &tmpY);
	else if (this->projection == WINKEL_TRIPEL) this->ProjectionWinkelTripel(lon, lat, &tmpX, &tmpY);
	
	
	*x = (this->widthPadding + ((tmpX - this->minX) * this->mapWidthRatio));
	*y = (this->mapHeight - this->heightPadding - ((tmpY - this->minY) * this->mapHeightRatio));


}

//========================================================================================================
//========================================================================================================
//========================================================================================================

//to do: inverse conversion

void MapProjections::ProjectionMercatorInverse(double x, double y, double * lon, double * lat) const
{
	*lon = RAD_TO_DEG(x);
	*lat = RAD_TO_DEG(2.0 * atan(exp(y)) - (MPI / 2.0));	
}

void MapProjections::ProjectionLambertConicInverse(double x, double y, double * lon, double * lat) const
{
	
	//http://mathworld.wolfram.com/LambertConformalConicProjection.html

	//Lambert conic inverse

	

	//----

	double tmp = sqrt(x * x + (this->lambertPhi0 - y) * (this->lambertPhi0 - y));
	double lambertPhi = sgn(this->lambertN) * tmp;

	tmp = x / (this->lambertPhi0 - y);
	double delta = atan(tmp);

	tmp = pow((this->lambertF / lambertPhi), 1.0 / this->lambertN);
	double latitude = 2 * atan(tmp) - 0.5 * MPI;

	double longitude = DEG_TO_RAD(10) + delta / this->lambertN;

	*lon = RAD_TO_DEG(longitude);
	*lat = RAD_TO_DEG(latitude);
}


void MapProjections::ConvertCoordinatesInverse(double x, double y, double * lon, double * lat)
{

	double tmpX = (x - this->widthPadding) / this->mapWidthRatio;
	tmpX = tmpX + this->minX;

	double tmpY = -1 * (y - this->mapHeight + this->heightPadding) / this->mapHeightRatio;
	tmpY = tmpY + this->minY;

	double tmpLon = 0;
	double tmpLat = 0;

	if (this->projection == MERCATOR) this->ProjectionMercatorInverse(tmpX, tmpY, &tmpLon, &tmpLat);
	else if (this->projection == LAMBERT_CONIC) this->ProjectionLambertConicInverse(tmpX, tmpY, &tmpLon, &tmpLat);
	
	*lon = tmpLon;
	*lat = tmpLat;
}
