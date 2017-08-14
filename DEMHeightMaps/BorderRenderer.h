#ifndef BORDER_RENDERER_H
#define BORDER_RENDERER_H


#include <unordered_map>

#include <MapProjection.h>


class BorderRenderer
{
	public:
		BorderRenderer(const std::string & borderDir);
		~BorderRenderer();

		void SetData(int w, int h, uint8_t * data);
		
		void DrawBorders(const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, bool keepAR);
				

	private:
		std::unordered_map<std::string, std::vector<IProjectionInfo::Coordinate> > borders;

		int w;
		int h;
		uint8_t * realHeightMap;


		void LoadBorderDirectory(const std::string & path);
		void ProcessBorderCSV(const std::string & borderFileName);
		void DrawLine(int startX, int startY, int endX, int endY, int width, int height);

};


#endif
