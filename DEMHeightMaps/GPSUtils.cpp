#include "./GPSUtils.h"

#include "./Utils/Utils.h"

//https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
void GPSUtils::DouglasPeuckerSimplification(std::vector<GPSPoint> & polyline, double tresshold, std::vector<GPSPoint> & result)
{
	return GPSUtils::DouglasPeuckerSimplification(polyline, 0, polyline.size() - 1, tresshold, result);
}

void GPSUtils::DouglasPeuckerSimplification(std::vector<GPSPoint> & polyline, int startIndex, int endIndex, double tresshold, std::vector<GPSPoint> & result)
{
	if (endIndex - startIndex <= 2)
	{
		//no points between - copy both points		
		return;
	}

	//find max distance from straight line
	double max = 0;
	int maxIndex = 0;

	for (int i = startIndex + 1; i <= endIndex - 1; i++)
	{
		double d = GPSUtils::LinePointDistanceSquared2D(polyline[startIndex], polyline[endIndex], polyline[i]);
		if (d > max) 
		{
			maxIndex = i;
			max = d;
		}
	}

	
	if (max > tresshold)
	{
		// Recursive call
		DouglasPeuckerSimplification(polyline, startIndex, maxIndex, tresshold, result);
		DouglasPeuckerSimplification(polyline, maxIndex, endIndex, tresshold, result);
		
	}
	else 
	{		
		if ((result.size() > 0) && (result.back() == polyline[startIndex]))
		{
			//if last index value in result == value at start index in polyline
			//insert only "end" point
			result.push_back(polyline[endIndex]);
		}
		else
		{
			result.push_back(polyline[startIndex]);
			result.push_back(polyline[endIndex]);
		}
	}

}


double GPSUtils::LinePointDistanceSquared2D(GPSPoint & A, GPSPoint & B, GPSPoint & p)
{
	// Return minimum distance between line segment vw and point p
	const double l2 = GPSPoint::PseudoDistanceSquared(A, B);  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0)
	{
		return GPSPoint::PseudoDistanceSquared(p, A);   // v == w case
	}

	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line. 
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2
	const double t = ((p.x - A.x) * (B.x - A.x) + (p.y - A.y) * (B.y - A.y)) / l2;  //GPSPoint::Dot(p - A, B - A) / l2;



	if (t < 0.0)
	{
		return GPSPoint::PseudoDistanceSquared(p, A);       // Beyond the 'v' end of the segment
	}
	else if (t > 1.0)
	{
		return GPSPoint::PseudoDistanceSquared(p, B);  // Beyond the 'w' end of the segment
	}

	GPSPoint projection;
	projection.x = A.x + t * (B.x - A.x);  // Projection falls on the segment
	projection.y = A.y + t * (B.y - A.y);  // Projection falls on the segment
	return GPSPoint::PseudoDistanceSquared(p, projection);
}




//haversine distance in km
//http://stackoverflow.com/questions/365826/calculate-distance-between-2-gps-coordinates
double GPSUtils::HaversineDistanceKm(GPSPoint p1, GPSPoint p2)
{
	
	double dlong = DEG_TO_RAD(p2.lon - p1.lon);
	double dlat = DEG_TO_RAD(p2.lat - p1.lat);
	double a = pow(sin(dlat / 2.0), 2) + cos(DEG_TO_RAD(p1.lat)) * cos(DEG_TO_RAD(p2.lat)) * pow(sin(dlong / 2.0), 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	double d = 6367 * c;

	return d;
}


GPSPointDMS GPSUtils::CoordinatesDECtoDMS(double dec)
{
	
	double deg = (int)dec;
	double tempma = deg - dec;

	tempma = tempma * 3600;
	double min = floor(tempma / 60);
	double sec = tempma - (min * 60);

	sec = round(sec);

	GPSPointDMS p;
	p.deg = deg;
	p.min = min;
	p.sec = sec;

	return p;
}


void GPSUtils::RemoveSamePoints(std::vector<GPSPoint> & polylineA, std::vector<GPSPoint> & polylineB)
{

}