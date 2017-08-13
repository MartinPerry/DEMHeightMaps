#ifndef MAP_PROJECTIONS_H
#define MAP_PROJECTIONS_H

#include <vector>


class MapProjections
{

	public:

		typedef enum PROJECTION_TYPE 
		{
			MERCATOR, 
			GALL_STEREO, 
			LAMBERT_CONIC, 
			LAMBERT_AZIMUT, 
			WINKEL_TRIPEL

		} PROJECTION_TYPE;
		

		MapProjections(PROJECTION_TYPE proj, int mapWidth, int mapHeight, double minLon, double minLat, double maxLon, double maxLat);
		~MapProjections();

		void SetKeepAR(bool keepAr);
		
		int GetWidth();
		int GetHeight();
		
		void ConvertCoordinates(double lon, double lat, double * x, double * y);
		void ConvertCoordinatesInverse(double x, double y, double * lon, double * lat);
				
	private:
		PROJECTION_TYPE projection;
		int mapWidth;
		int mapHeight;

		
		double mapWidthRatio;
		double mapHeightRatio;

		bool useGlobalRatio;

		double minX;
		double minY;
		double maxX;
		double maxY;

		double widthPadding;
		double heightPadding;

		double lambertF;
		double lambertN;		
		double lambertPhi0;

		double lambertSinP1;
		double lambertCosP1;

		void ProjectionMercator(double lon, double lat, double * x, double * y) const;
		void ProjectionGallStereo(double lon, double lat, double * x, double * y) const;
		void ProjectionLambertConic(double lon, double lat, double * x, double * y) const;
		void ProjectionLambertAzimut(double lon, double lat, double * x, double * y) const;
		void ProjectionWinkelTripel(double lon, double lat, double * x, double * y) const;

		void ProjectionMercatorInverse(double x, double y, double * lon, double * lat) const;
		void ProjectionLambertConicInverse(double x, double y, double * lon, double * lat) const;
		
		
		void SetMinMaxCoordinate(double minLon, double minLat, double maxLon, double maxLan);
		void SetDimension(int width, int height);

};


#endif
