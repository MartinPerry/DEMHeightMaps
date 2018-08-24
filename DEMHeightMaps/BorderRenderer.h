#ifndef BORDER_RENDERER_H
#define BORDER_RENDERER_H

#include <memory>
#include <unordered_map>

#include <MapProjection.h>

#include "./Strings/MyString.h"

template <typename ProjType>
class BorderRenderer
{
	public:
		BorderRenderer(const MyStringAnsi & borderDir, std::shared_ptr<ProjType> projection);
		~BorderRenderer();

		void SetData(int w, int h, uint8_t * data);
		
		void DrawBorders(const Projections::Coordinate & min, const Projections::Coordinate & max, bool keepAR);
				

	private:
		std::shared_ptr<ProjType> projection;
		std::unordered_map<MyStringAnsi, std::vector<Projections::Coordinate> > borders;

		int w;
		int h;
		uint8_t * realHeightMap;


		void LoadBorderDirectory(const MyStringAnsi & path);
		void ProcessBorderCSV(const MyStringAnsi & borderFileName);
		
};


#endif
