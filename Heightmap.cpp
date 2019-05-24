#include "Heightmap.h"
#include <fstream>
#include <vector>

Heightmap::Heightmap()
{
}

Heightmap::Heightmap(int m, int n)
{
	Recreate(m, n);
}

Heightmap::Heightmap(int m, int n, const std::string & filename, float heightScale, float heightOffset)
{
	LoadRAW(m, n, filename, heightScale, heightOffset);
}

void Heightmap::Recreate(int m, int n)
{
	mHeightScale = 1.0f;
	mHeightOffset = 0.0f;
	mHeightMap.resize(m, n, 0.0f);
}

void Heightmap::LoadRAW(int m, int n, const std::string & filename, float heightScale, float heightOffset)
{
	mHeightMapFilename = filename;
	mHeightScale = heightScale;
	mHeightOffset = heightOffset;

	// A height for each vertex
	std::vector<unsigned char> in(m * n);

	// Open the file.
	std::ifstream inFile;
	inFile.open(filename.c_str(), std::ios_base::binary);
	if (!inFile) HR(E_FAIL);

	// Read the RAW bytes.
	inFile.read((char*)&in[0], (std::streamsize)in.size());

	// Done with file.
	inFile.close();

	// Copy the array data into a float table format and scale
	// the heights.
	mHeightMap.resize(m, n, 0);
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			int k = i * n + j;
			mHeightMap(i, j) = (float)in[k] * heightScale + heightOffset;
		}
	}

	// Filter the table to smooth it out.  We do this because 256 height
	// steps is rather course.  And now that we copied the data into a
	// float-table, we have more precision.  So we can smooth things out
	// a bit by filtering the heights.
	Filter3x3();
}

int Heightmap::NumRows() const
{
	return mHeightMap.numRows();
}

int Heightmap::NumCols() const
{
	return mHeightMap.numCols();
}

float & Heightmap::operator()(int i, int j)
{
	return mHeightMap(i, j);
}

const float & Heightmap::operator()(int i, int j) const
{
	return mHeightMap(i, j);
}

bool Heightmap::InBounds(int i, int j)
{
	return
		i >= 0 &&
		i < (int)mHeightMap.numRows() &&
		j >= 0 &&
		j < (int)mHeightMap.numCols();
}

float Heightmap::SampleHeight3x3(int i, int j)
{
	float avg = 0.0f;
	float num = 0.0f;

	for (int m = i - 1; m <= i + 1; ++m)
	{
		for (int n = j - 1; n <= j + 1; ++n)
		{
			if (InBounds(m, n))
			{
				avg += mHeightMap(m, n);
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Heightmap::Filter3x3()
{
	Table<float> temp(mHeightMap.numRows(), mHeightMap.numCols());

	for (int i = 0; i < mHeightMap.numRows(); ++i)
		for (int j = 0; j < mHeightMap.numCols(); ++j)
			temp(i, j) = SampleHeight3x3(i, j);

	mHeightMap = temp;
}