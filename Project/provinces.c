
/*
 *   Source [provinces.c]
 */

#include "provinces.h"

#include <stdlib.h>
#include <string.h>

#define INIT_CAPACITY 256

#define RGB_MASK  0x00FFFFFFu
#define OCEAN_KEY 0x00FF0000u

typedef struct Vec2 {
	int32_t x;
	int32_t y;
} Vec2;

typedef struct ProvinceInternal {
	uint32_t          color;
	uint32_t*         neighbours;
	uint32_t          neighbour_count;
	uint32_t          neighbour_capacity;
	Vec2*             pixels;
	uint32_t          pixel_count;
	uint32_t          pixel_capacity;
	ROSE_VertexShape* vert_points;
	uint32_t          vert_point_count;
	uint32_t          vert_point_capacity;
	ROSE_VertexShape* vert_lines;
	uint32_t          vert_line_count;
	uint32_t          vert_line_capacity;
	ROSE_VertexShape* vert_triangles;
	uint32_t          vert_triangle_count;
	uint32_t          vert_triangle_capacity;
} ProvinceInternal;

static ROSE_VertexShape vertex(float x, float y)
{
	return (ROSE_VertexShape) { .position = { x, y }, .texcoord = { 0.0f, 0.0f }, .color = ROSE_COLOR_WHITE };
}

static ROSE_Color keytocolor(uint32_t key)
{
	return ROSE_COLOR_RGBA(key & 0xFFu, (key >> 8) & 0xFFu, (key >> 16) & 0xFFu, 255);
}

static uint32_t getpixelsafe(const uint32_t* pixels, uint32_t width, uint32_t height, int32_t x, int32_t y)
{
	bool bad_bounds =
		(x < 0) ||
		(y < 0) ||
		((uint32_t)x >= width) ||
		((uint32_t)y >= height);

	if (bad_bounds)
		return OCEAN_KEY;

	return pixels[((size_t)y * width) + (uint32_t)x];
}

static bool ischamfered(const uint32_t* pixels, uint32_t width, uint32_t height, int32_t vx, int32_t vy, uint32_t color)
{
	const uint32_t around[4] = {
		getpixelsafe(pixels, width, height, vx - 1, vy - 1),
		getpixelsafe(pixels, width, height, vx,     vy - 1),
		getpixelsafe(pixels, width, height, vx - 1, vy),
		getpixelsafe(pixels, width, height, vx,     vy),
	};

	uint32_t same_count = 0;
	bool     have_other = false;
	uint32_t other = 0;
	for (uint32_t k = 0; k < 4; k++) {
		if (around[k] == color) {
			same_count++;
		}
		else if (!have_other) {
			other = around[k];
			have_other = true;
		}
		else if (around[k] != other) {
			return false;
		}
	}

	return (same_count == 1) || (same_count == 3);
}

#define NO_PROVINCE UINT32_MAX
#define EMPTY_KEY   UINT32_MAX

typedef struct ColorTable {
	uint32_t* keys;
	uint32_t* values;
	uint32_t  count;
	uint32_t  capacity;
} ColorTable;

static uint32_t hashcolor(uint32_t key)
{
	key ^= key >> 16;
	key *= 0x7FEB352Du;
	key ^= key >> 15;
	return key;
}

static ColorTable colortablecreate(uint32_t capacity)
{
	ColorTable table = { .count = 0, .capacity = capacity };
	table.keys = malloc(capacity * sizeof(uint32_t));
	table.values = malloc(capacity * sizeof(uint32_t));
	memset(table.keys, 0xFF, capacity * sizeof(uint32_t));
	return table;
}

static void colortabledestroy(ColorTable* table)
{
	free(table->keys);
	free(table->values);
	*table = (ColorTable){ 0 };
}

static uint32_t colortablefind(ColorTable* table, uint32_t key)
{
	const uint32_t mask = table->capacity - 1;
	for (uint32_t slot = hashcolor(key) & mask;; slot = (slot + 1) & mask) {
		if (table->keys[slot] == key)
			return table->values[slot];
		if (table->keys[slot] == EMPTY_KEY)
			return NO_PROVINCE;
	}
}

static void colortableinsert(ColorTable* table, uint32_t key, uint32_t value)
{
	if (((table->count + 1) * 2) > table->capacity) {
		ColorTable grown = colortablecreate(table->capacity * 2);
		for (uint32_t i = 0; i < table->capacity; i++) {
			if (table->keys[i] != EMPTY_KEY)
				colortableinsert(&grown, table->keys[i], table->values[i]);
		}
		colortabledestroy(table);
		*table = grown;
	}

	const uint32_t mask = table->capacity - 1;
	uint32_t slot = hashcolor(key) & mask;
	while (table->keys[slot] != EMPTY_KEY)
		slot = (slot + 1) & mask;
	table->keys[slot] = key;
	table->values[slot] = value;
	table->count++;
}

Provinces GenerateWorldProvinces(ROSE_Image* image)
{
	uint32_t width  = 0;
	uint32_t height = 0;
	ROSE_ImageSize(image, &width, &height);

	uint32_t* pixels = ROSE_ImageCopyPixels(image);
	for (size_t i = 0; i < (size_t)width * height; i++)
		pixels[i] &= RGB_MASK;

	uint32_t  province_count = 0;
	uint32_t  province_capacity = INIT_CAPACITY;
	ProvinceInternal* provinces = calloc(INIT_CAPACITY, sizeof(ProvinceInternal));

	uint32_t* owner = malloc((size_t)width * height * sizeof(uint32_t));
	ColorTable color_table = colortablecreate(1024);

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			const size_t pixel_index = ((size_t)y * width) + x;
			const uint32_t curr_color = pixels[pixel_index];
			const bool ocean = (curr_color == OCEAN_KEY);
			if (ocean) {
				owner[pixel_index] = NO_PROVINCE;
				continue;
			}

			const uint32_t key = curr_color;
			uint32_t province_index = colortablefind(&color_table, key);
			if (province_index == NO_PROVINCE) {
				province_index = province_count;
				colortableinsert(&color_table, key, province_index);

				ProvinceInternal province = (ProvinceInternal){
					.color = curr_color,
					.pixels = NULL,
					.pixel_count = 0,
					.pixel_capacity = INIT_CAPACITY,
					.neighbours = NULL,
					.neighbour_count = 0,
					.neighbour_capacity = INIT_CAPACITY,
					.vert_points = NULL,
					.vert_point_count = 0,
					.vert_point_capacity = INIT_CAPACITY,
					.vert_triangles = NULL,
					.vert_triangle_count = 0,
					.vert_triangle_capacity = INIT_CAPACITY,
					.vert_lines = NULL,
					.vert_line_count = 0,
					.vert_line_capacity = INIT_CAPACITY,
				};

				province.pixels = calloc(INIT_CAPACITY, sizeof(Vec2));
				province.neighbours = calloc(INIT_CAPACITY, sizeof(uint32_t));
				province.vert_points = calloc(INIT_CAPACITY, sizeof(ROSE_VertexShape));
				province.vert_triangles = calloc(INIT_CAPACITY, sizeof(ROSE_VertexShape));
				province.vert_lines = calloc(INIT_CAPACITY, sizeof(ROSE_VertexShape));

				if (province_count >= province_capacity) {
					province_capacity *= 2;
					provinces = realloc(provinces, province_capacity * sizeof(ProvinceInternal));
				} provinces[province_count++] = province;
			}

			owner[pixel_index] = province_index;

			ProvinceInternal* province = &provinces[province_index];
			const Vec2 pixel = (Vec2){ (int32_t)x, (int32_t)y };
			if (province->pixel_count >= province->pixel_capacity) {
				province->pixel_capacity *= 2;
				province->pixels = realloc(province->pixels, province->pixel_capacity * sizeof(Vec2));
			} province->pixels[province->pixel_count++] = pixel;
		}
	}

	colortabledestroy(&color_table);

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			const size_t   pixel_index = ((size_t)y * width) + x;
			const uint32_t province_index = owner[pixel_index];
			if (province_index == NO_PROVINCE)
				continue;

			const uint32_t adjacent_indices[2] = {
				((x + 1) < width) ? owner[pixel_index + 1] : NO_PROVINCE,
				((y + 1) < height) ? owner[pixel_index + width] : NO_PROVINCE,
			};

			ProvinceInternal* province = &provinces[province_index];
			for (uint32_t k = 0; k < 2; k++) {
				const uint32_t adjacent_index = adjacent_indices[k];
				if ((adjacent_index == NO_PROVINCE) || (adjacent_index == province_index))
					continue;

				bool known = false;
				for (uint32_t n = 0; n < province->neighbour_count; n++) {
					if (province->neighbours[n] == adjacent_index) {
						known = true;
						break;
					}
				}

				if (known)
					continue;

				const uint32_t pair[2][2] = {
					{ province_index, adjacent_index },
					{ adjacent_index, province_index },
				};

				for (uint32_t p = 0; p < 2; p++) {
					ProvinceInternal* holder = &provinces[pair[p][0]];
					const uint32_t    neighbour = pair[p][1];
					if (holder->neighbour_count >= holder->neighbour_capacity) {
						holder->neighbour_capacity *= 2;
						holder->neighbours = realloc(holder->neighbours, holder->neighbour_capacity * sizeof(uint32_t));
					} holder->neighbours[holder->neighbour_count++] = neighbour;
				}
			}
		}
	}

	{
		const int32_t direction_steps[4][2] = { { 1,  0 }, { 0, 1 }, { -1,  0 }, {  0, -1 } };
		const int32_t side_neighbours[4][2] = { { 0, -1 }, { 1, 0 }, {  0,  1 }, { -1,  0 } };
		const int32_t side_starts[4][2] = { { 0,  0 }, { 1, 0 }, {  1,  1 }, {  0,  1 } };

		const uint32_t vertex_columns = width + 1;
		const uint32_t vertex_rows = height + 1;
		uint8_t* edge_masks = calloc((size_t)vertex_columns * vertex_rows, sizeof(uint8_t));

		for (uint32_t i = 0; i < province_count; i++) {
			ProvinceInternal* province = &provinces[i];

			for (uint32_t j = 0; j < province->pixel_count; j++) {
				const Vec2 pixel = province->pixels[j];
				for (uint32_t s = 0; s < 4; s++) {
					const int32_t nx = pixel.x + side_neighbours[s][0];
					const int32_t ny = pixel.y + side_neighbours[s][1];
					const bool in_bounds = (nx >= 0) && (ny >= 0) && ((uint32_t)nx < width) && ((uint32_t)ny < height);
					if (in_bounds && (owner[((size_t)ny * width) + (uint32_t)nx] == i))
						continue;
					const uint32_t v = ((uint32_t)(pixel.y + side_starts[s][1]) * vertex_columns) + (uint32_t)(pixel.x + side_starts[s][0]);
					edge_masks[v] |= (uint8_t)(1u << s);
				}
			}

			const int32_t start_x = province->pixels[0].x;
			const int32_t start_y = province->pixels[0].y;
			int32_t  vx = start_x;
			int32_t  vy = start_y;
			uint32_t dir = 0;

			for (;;) {
				edge_masks[((uint32_t)vy * vertex_columns) + (uint32_t)vx] &= (uint8_t)~(1u << dir);

				if ((province->vert_point_count + 2) > province->vert_point_capacity) {
					province->vert_point_capacity *= 2;
					province->vert_points = realloc(province->vert_points, province->vert_point_capacity * sizeof(ROSE_VertexShape));
				}

				const ROSE_VertexShape midpoint = vertex(
					(float)vx + (0.5f * (float)direction_steps[dir][0]),
					(float)vy + (0.5f * (float)direction_steps[dir][1])
				);
				province->vert_points[province->vert_point_count++] = midpoint;

				vx += direction_steps[dir][0];
				vy += direction_steps[dir][1];

				const bool closed = (vx == start_x) && (vy == start_y);
				uint32_t next = 0;
				if (!closed) {
					const uint8_t mask = edge_masks[((uint32_t)vy * vertex_columns) + (uint32_t)vx];
					while (!(mask & (1u << next))) next++;
				}

				if ((next != dir) && !ischamfered(pixels, width, height, vx, vy, province->color)) {
					const ROSE_VertexShape corner = vertex((float)vx, (float)vy);
					province->vert_points[province->vert_point_count++] = corner;
				}

				dir = next;
				if (closed)
					break;
			}
		}

		free(edge_masks);
	}

	free(owner);
	free(pixels);

	for (uint32_t i = 0; i < province_count; i++) {
		ProvinceInternal* province = &provinces[i];
		const uint32_t point_count = province->vert_point_count;
		if (point_count == 0)
			continue;

		ROSE_VertexShape* original = malloc(point_count * sizeof(ROSE_VertexShape));
		memcpy(original, province->vert_points, point_count * sizeof(ROSE_VertexShape));

		uint32_t kept_count = 0;
		for (uint32_t j = 0; j < point_count; j++) {
			const ROSE_VertexShape prev = original[(j + point_count - 1) % point_count];
			const ROSE_VertexShape curr = original[j];
			const ROSE_VertexShape next = original[(j + 1) % point_count];

			const float in_x = curr.position.x - prev.position.x;
			const float in_y = curr.position.y - prev.position.y;
			const float out_x = next.position.x - curr.position.x;
			const float out_y = next.position.y - curr.position.y;

			const float cross = (in_x * out_y) - (in_y * out_x);
			const float dot = (in_x * out_x) + (in_y * out_y);

			const bool inline_point = (cross == 0.0f) && (dot > 0.0f);
			if (!inline_point) {
				province->vert_points[kept_count++] = curr;
			}
		}

		province->vert_point_count = kept_count;
		free(original);
	}

	for (uint32_t i = 0; i < province_count; i++) {
		ProvinceInternal* province = &provinces[i];
		const uint32_t point_count = province->vert_point_count;
		for (uint32_t j = 0; j < point_count; j++) {
			ROSE_VertexShape pointA = province->vert_points[j];
			ROSE_VertexShape pointB = province->vert_points[(j + 1) % point_count];

			if ((province->vert_line_count + 2) > province->vert_line_capacity) {
				province->vert_line_capacity *= 2;
				province->vert_lines = realloc(province->vert_lines, province->vert_line_capacity * sizeof(ROSE_VertexShape));
			}

			province->vert_lines[province->vert_line_count++] = pointA;
			province->vert_lines[province->vert_line_count++] = pointB;
		}
	}

	for (uint32_t i = 0; i < province_count; i++) {
		ProvinceInternal* province = &provinces[i];
		const ROSE_VertexShape* points = province->vert_points;
		uint32_t remaining = province->vert_point_count;
		if (remaining < 3)
			continue;

		uint32_t* indices = malloc(remaining * sizeof(uint32_t));
		for (uint32_t j = 0; j < remaining; j++)
			indices[j] = j;

		uint32_t j = 0;
		uint32_t attempts = 0;
		while (remaining > 3) {
			const uint32_t ia = indices[(j + remaining - 1) % remaining];
			const uint32_t ib = indices[j];
			const uint32_t ic = indices[(j + 1) % remaining];
			const float ax = points[ia].position.x, ay = points[ia].position.y;
			const float bx = points[ib].position.x, by = points[ib].position.y;
			const float cx = points[ic].position.x, cy = points[ic].position.y;
			const float cross = ((bx - ax) * (cy - by)) - ((by - ay) * (cx - bx));

			bool ear = (cross > 0.0f);
			for (uint32_t k = 0; ear && (k < remaining); k++) {
				const uint32_t ip = indices[k];
				if ((ip == ia) || (ip == ib) || (ip == ic))
					continue;
				const float px = points[ip].position.x;
				const float py = points[ip].position.y;
				const float d0 = ((bx - ax) * (py - ay)) - ((by - ay) * (px - ax));
				const float d1 = ((cx - bx) * (py - by)) - ((cy - by) * (px - bx));
				const float d2 = ((ax - cx) * (py - cy)) - ((ay - cy) * (px - cx));
				if ((d0 >= 0.0f) && (d1 >= 0.0f) && (d2 >= 0.0f))
					ear = false;
			}

			const bool flat = (cross == 0.0f) && (attempts >= remaining);

			if (ear || flat) {
				if (ear) {
					if ((province->vert_triangle_count + 3) > province->vert_triangle_capacity) {
						province->vert_triangle_capacity *= 2;
						province->vert_triangles = realloc(province->vert_triangles, province->vert_triangle_capacity * sizeof(ROSE_VertexShape));
					}
					province->vert_triangles[province->vert_triangle_count++] = points[ia];
					province->vert_triangles[province->vert_triangle_count++] = points[ib];
					province->vert_triangles[province->vert_triangle_count++] = points[ic];
				}

				memmove(&indices[j], &indices[j + 1], (remaining - j - 1) * sizeof(uint32_t));
				remaining--;
				if (j >= remaining)
					j = 0;
				attempts = 0;
				continue;
			}

			j = (j + 1) % remaining;
			attempts++;
		}

		if ((province->vert_triangle_count + 3) > province->vert_triangle_capacity) {
			province->vert_triangle_capacity *= 2;
			province->vert_triangles = realloc(province->vert_triangles, province->vert_triangle_capacity * sizeof(ROSE_VertexShape));
		}
		province->vert_triangles[province->vert_triangle_count++] = points[indices[0]];
		province->vert_triangles[province->vert_triangle_count++] = points[indices[1]];
		province->vert_triangles[province->vert_triangle_count++] = points[indices[2]];

		free(indices);
	}

	Province* result = calloc(province_count, sizeof(Province));
	for (uint32_t i = 0; i < province_count; i++) {
		ProvinceInternal* internal = &provinces[i];
		Province* province = &result[i];

		province->index = i;
		province->color = keytocolor(internal->color);

		province->neighbours = internal->neighbours;
		province->neighbour_count = internal->neighbour_count;

		province->border = ROSE_ShapeCreate(internal->vert_lines, internal->vert_line_count);
		province->triangles = ROSE_ShapeCreate(internal->vert_triangles, internal->vert_triangle_count);

		free(internal->pixels);
		free(internal->vert_points);
		free(internal->vert_lines);
		free(internal->vert_triangles);
	}

	free(provinces);

	return (Provinces) {
		.array = result,
		.count = province_count,
		.capacity = province_count,
	};
}

void DestroyWorldProvinces(Provinces* provinces)
{
	for (uint32_t i = 0; i < provinces->count; i++) {
		Province* province = &provinces->array[i];
		free(province->neighbours);
		ROSE_ShapeDestroy(province->border);
		ROSE_ShapeDestroy(province->triangles);
	}

	free(provinces->array);
	*provinces = (Provinces){ 0 };
}

