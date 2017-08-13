#ifndef BORDER_RENDERER_H
#define BORDER_RENDERER_H


#include <unordered_map>

#include "GPSUtils.h"


class BorderRenderer
{
	public:
		BorderRenderer(const std::string & borderDir);
		~BorderRenderer();

		void SetData(int w, int h, uint8_t * data);
		
		void DrawBorders(double minLon, double minLat, double maxLon, double maxLat, bool keepAR);
				

	private:
		std::unordered_map<std::string, std::vector<GPSPoint> > borders;

		int w;
		int h;
		uint8_t * realHeightMap;


		void LoadBorderDirectory(const std::string & path);
		void ProcessBorderCSV(const std::string & borderFileName);
		void DrawLine(int startX, int startY, int endX, int endY, int width, int height);

};


#endif
