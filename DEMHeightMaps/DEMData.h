#ifndef DEM_DATA_H
#define DEM_DATA_H

#include <unordered_map>
#include <memory>
#include <thread>
#include <vector>

#include <MapProjection.h>
#include <GeoCoordinate.h>


#include "./DEMTile.h"
#include "./Cache/MemoryCache.h"
#include "./Strings/MyString.h"

typedef std::unordered_map<DEMTileInfo, DEMTileData, hashFunc, equalsFunc> DemTileMap;

typedef struct Neighbors
{
	std::vector<Projections::Coordinate> neighborCoord;
	std::unordered_map<DEMTileInfo *, std::vector<size_t>> neighborsCache;
} Neighbors;

template <typename HeightType, typename ProjType>
class DEMData 
{
	public:

		DEMData(std::initializer_list<MyStringAnsi> dirs);
		DEMData(std::initializer_list<MyStringAnsi> dirs, const MyStringAnsi & tilesInfoXML);
		~DEMData();
		
		std::shared_ptr<ProjType> GetProjection() const;

		void SetVerboseEnabled(bool val);
		void SetElevationMappingEnabled(bool val);
		void SetMinMaxElevation(double minElev, double maxElev);

		void ExportTileList(const MyStringAnsi & fileName);
		
		std::unordered_map<size_t, std::unordered_map<size_t, TileInfo>> BuildTileMap(
			int totalW, int totalH, int tilesCountX, int tilesCountY,
			const Projections::Coordinate & min, const Projections::Coordinate & max);


		void ProcessTileMap(int totalW, int totalH,
			int tilesCountX, int tilesCountY,
			const Projections::Coordinate & min, const Projections::Coordinate & max,
			std::function<void(TileInfo & ti, size_t, size_t y)> tileCallback);

		void ProcessTileMap(int tileW, int tileH,
			const Projections::Coordinate & min, const Projections::Coordinate & max,
			const Projections::Coordinate & tileStep,
			std::function<void(TileInfo & ti, size_t x, size_t y)> tileCallback);

		HeightType * BuildMap(int w, int h, const Projections::Coordinate & min, const Projections::Coordinate & max, bool keepAR);
		

	
	private:
		
		

		const int TILE_SIZE_3 = 1201;
		const int TILE_SIZE_1 = 3601;

		bool verbose;

		bool elevMapping;
		double minHeight;
		double maxHeight;

		//loaded tiles and projection info
		std::vector<std::vector<DEMTileInfo>> tiles2Dmap; //[geo position][all tiles]
		std::shared_ptr<ProjType> projection;

		//main image tiles
		std::vector<Projections::Coordinate> coords; //[pixel] = coords		
		std::unordered_map<DEMTileInfo *, std::vector<size_t>> tilePixels; //[tile] = list of pixels

	
		MemoryCache<MyStringAnsi, TileRawData, LRUControl<MyStringAnsi>> * tilesCache;
		
		

		void LoadTiles();
		void ImportTileList(const MyStringAnsi & fileName);

		Neighbors GetCoordinateNeighbors(const Projections::Coordinate & c, DEMTileInfo * ti);

		DEMTileInfo * GetTile(const Projections::Coordinate & c);
		void AddTile(const DEMTileInfo & ti);

		short GetHeight(DEMTileData & td, size_t index);
		
};


/*
//http://ideone.com/Z7zldb
template<typename Index, typename Callable>
static void ParallelFor(Index start, Index end, Callable func) {
// Estimate number of threads in the pool
const static unsigned nb_threads_hint = std::thread::hardware_concurrency();
const static unsigned nb_threads = (nb_threads_hint == 0u ? 8u : nb_threads_hint);

// Size of a slice for the range functions
Index n = end - start + 1;
Index slice = (Index)std::round(n / static_cast<double> (nb_threads));
slice = std::max(slice, Index(1));

// [Helper] Inner loop
auto launchRange = [&func](int k1, int k2) {
for (Index k = k1; k < k2; k++) {
func(k);
}
};

// Create pool and launch jobs
std::vector<std::thread> pool;
pool.reserve(nb_threads);
Index i1 = start;
Index i2 = std::min(start + slice, end);
for (unsigned i = 0; i + 1 < nb_threads && i1 < end; ++i) {
pool.emplace_back(launchRange, i1, i2);
i1 = i2;
i2 = std::min(i2 + slice, end);
}
if (i1 < end) {
pool.emplace_back(launchRange, i1, end);
}

// Wait for jobs to finish
for (std::thread &t : pool) {
if (t.joinable()) {
t.join();
}
}
}

// Serial version for easy comparison
template<typename Index, typename Callable>
static void SequentialFor(Index start, Index end, Callable func) {
for (Index i = start; i < end; i++) {
func(i);
}
}

*/


#endif
