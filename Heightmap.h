#pragma once

#include "d3dUtil.h"
#include "Table.h"
#include <string>

class Heightmap
{
public:
	Heightmap();
	Heightmap(int m, int n);
	Heightmap(int m, int n, const std::string& filename, float heightScale, float heightOffset);

	void Recreate(int m, int n);
	void LoadRAW(int m, int n, const std::string& filename, float heightScale, float heightOffset);

	int NumRows() const;
	int NumCols() const;

	// For non-const objects
	float& operator() (int i, int j);

	// For const objects
	const float& operator() (int i, int j) const;

private:
	bool  InBounds(int i, int j);
	float SampleHeight3x3(int i, int j);
	void  Filter3x3();
private:
	std::string  mHeightMapFilename;
	Table<float> mHeightMap;
	float        mHeightScale;
	float        mHeightOffset;
};

