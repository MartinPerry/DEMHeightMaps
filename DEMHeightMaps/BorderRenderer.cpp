#include "BorderRenderer.h"

#include <MapProjection.h>
#include <GeoCoordinate.h>
#include <Projections.h>
#include <ProjectionRenderer.h>

#include "./VFS/win_dirent.h"
#include "./VFS/VFSUtils.h"
#include "./Utils/Utils.h"


BorderRenderer::BorderRenderer(const std::string & borderDir)
{
	this->LoadBorderDirectory(borderDir);
	this->realHeightMap = NULL;
}

BorderRenderer::~BorderRenderer()
{
}

void BorderRenderer::SetData(int w, int h, uint8_t * data)
{
	this->w = w;
	this->h = h;

	this->realHeightMap = data;
}

/*-----------------------------------------------------------
Function:	LoadBorderDirectory
Parameters:
	[in] path - full path to directory

Load border data from given directory.
-------------------------------------------------------------*/
void BorderRenderer::LoadBorderDirectory(const std::string & path)
{
	printf("---- Loading border directory ----\n");

	DIR *dir;
	struct dirent * ent;

	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		return;
	}


	std::string newDirName;
	std::string fullPath;
	std::string key;

	/* print all the files and directories within directory */
	while ((ent = readdir(dir)) != NULL)
	{
		if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
		{
			continue;
		}

		switch (ent->d_type)
		{
		case DT_REG:

			//file
			fullPath = dir->patt;
			fullPath = fullPath.substr(0, fullPath.length() - 1);
			fullPath += ent->d_name;

			if (fullPath.find(".csv") != std::string::npos)
			{
				this->ProcessBorderCSV(fullPath);
			}
			
			break;

		case DT_DIR:

			break;

		default:
			//printf ("%s:\n", ent->d_name);
			break;
		}
	}


	closedir(dir);
}

void BorderRenderer::ProcessBorderCSV(const std::string & borderFileName)
{
	std::string content = loadFromFile(borderFileName);

	//content = content.SubString(0, 2000);

	std::vector<std::string> lines = split(content, '\n');

	std::string keyName;
	std::vector<IProjectionInfo::Coordinate> * border = NULL;

	int tmp = 0;
	int USE_EVERY_NTH_POINT = 1;

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> line = split(lines[i], ';');
		if (line.size() <= 2)
		{
			continue;
		}
		if (line.size() > 5)
		{
			keyName = line[7];
			tmp = 0;
		}
		else
		{
			if (keyName.find("Czech") != std::string::npos)
			{
				USE_EVERY_NTH_POINT = 1;
			}
			else
			{
				USE_EVERY_NTH_POINT = 1;
			}

			if (tmp % USE_EVERY_NTH_POINT == 0)
			{
				

				std::string key = keyName;
				key += "_";
				key += line[3];

				border = &this->borders[key];
				
				IProjectionInfo::Coordinate point;
				point.lon = GeoCoordinate::deg(atof(line[0].c_str()));
				point.lat = GeoCoordinate::deg(atof(line[1].c_str()));

				border->push_back(point);
			}

			tmp++;
		}

	}


}



void BorderRenderer::DrawBorders(const IProjectionInfo::Coordinate & min, const IProjectionInfo::Coordinate & max, bool keepAR)
{	



	IProjectionInfo * mercator = new Mercator();
	mercator->SetFrame(min, max, w, h, keepAR);


	ProjectionRenderer render(mercator);
	render.SetRawDataTarget(realHeightMap);
	
	for (auto it : this->borders)
	{

		const auto & b = it.second;

		for (size_t i = 0; i < b.size(); i++)
		{
			auto start = b[i % b.size()];
			auto end = b[(i + 1) % b.size()];

			if (start.lat.rad() > 4)
			{
				continue;
			}			
			if (start.lat.rad() < -4)
			{
				continue;
			}

			render.DrawLine(start, end);						
		}

	}	
}

void BorderRenderer::DrawLine(int startX, int startY, int endX, int endY, int width, int height)
{
	//TO DO.. kdyz jsou start body mimo okno
	//oriznout na okno
	//napr. pres Cohen-Sutherlanda

	if ((startX >= static_cast<int>(width))
		|| (startY >= static_cast<int>(height)))
	{
		return;
	}

	if ((endX >= static_cast<int>(width))
		|| (endY >= static_cast<int>(height)))
	{
		return;
	}

	if ((startX < 0)
		|| (startY < 0))
	{
		return;
	}

	if ((endX < 0)
		|| (endY < 0))
	{
		return;
	}

	int dx = abs(static_cast<long>(endX - startX));
	int dy = abs(static_cast<long>(endY - startY));
	int sx, sy, e2;


	(startX < endX) ? sx = 1 : sx = -1;
	(startY < endY) ? sy = 1 : sy = -1;
	int err = dx - dy;

	while (1)
	{
		realHeightMap[startX + startY * width] = 255;
		if ((startX == endX) && (startY == endY)) break;
		e2 = 2 * err;
		if (e2 > -dy)
		{
			err = err - dy;
			startX = startX + sx;
		}
		if (e2 < dx)
		{
			err = err + dx;
			startY = startY + sy;
		}
	}

}

