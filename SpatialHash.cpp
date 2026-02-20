//
// Created by Ali Hamdy on 18/02/2026.
//

#include "SpatialHash.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>


vector2 operator/(vector2 a, float b)
{
    return (vector2){a.x / b, a.y / b};
}

SpatialHash::SpatialHash(double spacing, int maxNumOfObjects)
{
    Spacing = spacing;
    NumObjects = maxNumOfObjects;
    tableSize = 2 * maxNumOfObjects;
    cellStart = new int[tableSize + 1];
    cellEntries = new int[NumObjects];
    queryIds = new int[NumObjects];
    querySize = 0;
}

SpatialHash::~SpatialHash()
{

        delete[] cellStart;
        delete[] cellEntries;
        delete[] queryIds;

}

int SpatialHash::hashCoords(int x, int y)
{
    int hash = (x * 92837111) + (y * 689287499);
    return std::abs(hash) % tableSize;
}

vector2 SpatialHash::intCoord(vector2 position)
{
    return (vector2){std::floor(position.x / Spacing), std::floor(position.y / Spacing)};
}

int SpatialHash::hashPosition(vector2* positions, int index)
{
    vector2 coord = intCoord(positions[index]);
    return hashCoords((int)coord.x, (int)coord.y);
}

void SpatialHash::create(vector2* positions, int numObjects)
{
    numObjects = std::min(numObjects, NumObjects);

    // Reset cell start and entries
    for (int i = 0; i <= tableSize; i++) {
        cellStart[i] = 0;
    }
    for (int i = 0; i < NumObjects; i++) {
        cellEntries[i] = 0;
    }

    // Count objects per cell
    for (int i = 0; i < numObjects; i++) {
        int h = hashPosition(positions, i);
        cellStart[h]++;
    }

    // Determine cell starts (prefix sum)
    int start = 0;
    for (int i = 0; i < tableSize; i++) {
        start += cellStart[i];
        cellStart[i] = start;
    }
    cellStart[tableSize] = start; // guard

    // Fill in object ids
    for (int i = 0; i < numObjects; i++) {
        int h = hashPosition(positions, i);
        cellStart[h]--;
        cellEntries[cellStart[h]] = i;
    }
}

void SpatialHash::query(vector2* positions, int index, float maxDist)
{
    int x0 = (int)std::floor((positions[index].x - maxDist) / Spacing);
    int y0 = (int)std::floor((positions[index].y - maxDist) / Spacing);

    int x1 = (int)std::floor((positions[index].x + maxDist) / Spacing);
    int y1 = (int)std::floor((positions[index].y + maxDist) / Spacing);

    querySize = 0;

    for (int xi = x0; xi <= x1; xi++) {
        for (int yi = y0; yi <= y1; yi++) {
            int h = hashCoords(xi, yi);
            int start = cellStart[h];
            int end = cellStart[h + 1];

            for (int i = start; i < end; i++) {
                queryIds[querySize] = cellEntries[i];
                querySize++;
            }
        }
    }
}