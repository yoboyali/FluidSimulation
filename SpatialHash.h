//
// Created by Ali Hamdy on 18/02/2026.
//

#ifndef SPATIALHASH_H
#define SPATIALHASH_H

struct vector2
{
    float x ,y;
};

class SpatialHash
{
public:
    SpatialHash(double spacing, int maxNumOfObjects);
    ~SpatialHash(); // Destructor to free memory

    void create(vector2* positions, int numObjects);
    void query(vector2* positions, int index, float maxDist);

    int* queryIds;  // Public so you can access query results
    int querySize;  // Number of results from query

private:
    int hashCoords(int x, int y);
    vector2 intCoord(vector2 position);
    int hashPosition(vector2* positions, int index);

    double Spacing;
    int NumObjects;
    int tableSize;

    int* cellStart;
    int* cellEntries;
};



#endif //SPATIALHASH_H
