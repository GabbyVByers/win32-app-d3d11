
/*
 *   Header [provinces.h]
 */

#ifndef PROVINCES_HEADER_GUARD
#define PROVINCES_HEADER_GUARD

#include "rose.h"

typedef struct Province {
	uInt32      index;
	ROSE_Color  color;
	uInt32*     neighbours;
	uInt32      neighbour_count;
	ROSE_Shape* border;
	ROSE_Shape* triangles;
} Province;

typedef struct Provinces {
	Province* array;
	uint32_t  count;
	uint32_t  capacity;
} Provinces;

Provinces GenerateWorldProvinces(ROSE_Image* image);
void      DestroyWorldProvinces(Provinces* provinces);

#endif /* PROVINCES_HEADER_GUARD */

