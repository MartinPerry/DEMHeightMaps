#ifndef GPS_UTILS_H
#define GPS_UTILS_H


#include <vector>


typedef struct GPSPoint
{
	union
	{
		struct 
		{
			double x; //lat
			double y; //lon
		};
		struct 
		{
			double lat;
			double lon;
		};
	};
	
	GPSPoint() 
		: lat(0), lon(0) 
	{
	}

	GPSPoint(double lat, double lon) 
		: lat(lat), lon(lon) 
	{
	}

	bool operator ==(const GPSPoint & other)
	{
		return ((x == other.x) && (y == other.y));
	}

	static double PseudoDistanceSquared(const GPSPoint & a, const GPSPoint & b)
	{
		double x = a.x - b.x;
		double y = a.y - b.y;
		return (x * x) + (y * y);
	};

		static double Dot(const GPSPoint &left, const GPSPoint &right)
		{	
			return (left.x * right.x + left.y * right.y);
		};


} GPSPoint;

typedef struct GPSPointDMS
{
	double deg;
	double min;
	double sec;
} GPSPointDMS;

class GPSUtils 
{
	public:

		static void DouglasPeuckerSimplification(std::vector<GPSPoint> & polyline, double tresshold, std::vector<GPSPoint> & result);
		static void DouglasPeuckerSimplification(std::vector<GPSPoint> & polyline, int startIndex, int endIndex, double tresshold, std::vector<GPSPoint> & result);
		
		static double HaversineDistanceKm(GPSPoint p1, GPSPoint p2);
		static GPSPointDMS CoordinatesDECtoDMS(double dec);

		static void RemoveSamePoints(std::vector<GPSPoint> & polylineA, std::vector<GPSPoint> & polylineB);

	protected:
		static double LinePointDistanceSquared2D(GPSPoint & A, GPSPoint & B, GPSPoint & p);

};


#endif
