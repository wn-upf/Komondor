#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/*
 * computeDistance(): returns the distance between 2 points
 * Input arguments:
 * - x1,y1,z1: first point position
 * - x2,y2,z2: second point position
 * Output:
 * - distance: distance [m]
 */
double computeDistance(int x1, int y1, int z1, int x2, int y2, int z2){
	double distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
	return distance;
}
