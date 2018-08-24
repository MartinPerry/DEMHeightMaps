#include "BorderRenderer.h"

#include <MapProjection.h>
#include <GeoCoordinate.h>
#include <Projections.h>
#include <ProjectionRenderer.h>

#include "./VFS/win_dirent.h"
//#include "./VFS/VFSUtils.h"
#include "./Utils/Utils.h"


template <typename ProjType>
BorderRenderer<ProjType>::BorderRenderer(const MyStringAnsi & borderDir, std::shared_ptr<ProjType> projection)
{
	this->projection = projection;
	this->LoadBorderDirectory(borderDir);
	this->realHeightMap = NULL;
}

template <typename ProjType>
BorderRenderer<ProjType>::~BorderRenderer()
{
}

template <typename ProjType>
void BorderRenderer<ProjType>::SetData(int w, int h, uint8_t * data)
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
template <typename ProjType>
void BorderRenderer<ProjType>::LoadBorderDirectory(const MyStringAnsi & path)
{
	printf("---- Loading border directory ----\n");

	DIR *dir;
	struct dirent * ent;

	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		return;
	}


	MyStringAnsi newDirName;
	MyStringAnsi fullPath;
	MyStringAnsi key;

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
			fullPath = fullPath.SubString(0, fullPath.length() - 1);
			fullPath += ent->d_name;

			if (fullPath.Find(".csv") != MyStringAnsi::npos)
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

template <typename ProjType>
void BorderRenderer<ProjType>::ProcessBorderCSV(const MyStringAnsi & borderFileName)
{
	MyStringAnsi content = MyStringAnsi::LoadFromFile(borderFileName.c_str());

	//content = content.SubString(0, 2000);

	std::vector<MyStringAnsi> lines = content.Split('\n');

	MyStringAnsi keyName;
	std::vector<Projections::Coordinate> * border = NULL;

	int tmp = 0;
	int USE_EVERY_NTH_POINT = 1;

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<MyStringAnsi> line = lines[i].Split(';');
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
			if (keyName.Find("Czech") != MyStringAnsi::npos)
			{
				USE_EVERY_NTH_POINT = 1;
			}
			else
			{
				USE_EVERY_NTH_POINT = 1;
			}

			if (tmp % USE_EVERY_NTH_POINT == 0)
			{
				

				MyStringAnsi key = keyName;
				key += "_";
				key += line[3];

				border = &this->borders[key];
				
				Projections::Coordinate point;
				point.lon = GeoCoordinate::deg(atof(line[0].c_str()));
				point.lat = GeoCoordinate::deg(atof(line[1].c_str()));

				border->push_back(point);
			}

			tmp++;
		}

	}


}


template <typename ProjType>
void BorderRenderer<ProjType>::DrawBorders(const Projections::Coordinate & min, const Projections::Coordinate & max, bool keepAR)
{	

	
	projection->SetFrame(min, max, w, h, keepAR);


	Projections::ProjectionRenderer render(projection.get());
	render.SetRawDataTarget(realHeightMap, Projections::ProjectionRenderer::GREY);
	
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

template class BorderRenderer<Projections::Mercator>;
template class BorderRenderer<Projections::Equirectangular>;
