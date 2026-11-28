
/*
 *   Source [rose_shape.c]
 */

#include "rose_internal.h"

ROSE_Shape* ROSE_ShapeCreate(const ROSE_VertexShape* vertices, uInt32 count)
{
	ROSE_Shape* shape = calloc(1, sizeof(ROSE_Shape));
	ROSE_ShapeUpdate(shape, vertices, count);
	return shape;
}

void ROSE_ShapeDestroy(ROSE_Shape* shape)
{
	if (shape == NULL)
		return;

	if (shape->vertices != NULL)
		ID3D11Buffer_Release(shape->vertices);

	free(shape);
}

void ROSE_ShapeDraw(ROSE_Shape* shape, ROSE_ShapeDrawInfo draw)
{
	ROSE__BatchFlush();

	bool empty =
		(shape->vertices == NULL) ||
		(shape->count == 0);

	if (empty)
		return;

	ROSE__BindPipelineShape(
		shape->vertices,
		draw.texture,
		draw.position,
		draw.scale,
		draw.color,
		draw.primitive,
		draw.point_diameter
	);

	ID3D11DeviceContext_Draw(rose__context, shape->count, 0);
}

void ROSE_ShapeUpdate(ROSE_Shape* shape, const ROSE_VertexShape* vertices, uInt32 count)
{
	bool empty =
		(vertices == NULL) ||
		(count == 0);

	if (empty) {
		shape->count = 0;
		return;
	}

	if (count > shape->capacity) {
		uInt32 capacity = (shape->capacity * 2);

		if (capacity < count)
			capacity = count;

		if (shape->vertices != NULL)
		ID3D11Buffer_Release(shape->vertices);

		D3D11_BUFFER_DESC vertex_buffer_desc = {
			.ByteWidth      = sizeof(ROSE_VertexShape) * capacity,
			.Usage          = D3D11_USAGE_DYNAMIC,
			.BindFlags      = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		};

		shape->vertices = NULL;
		HRESULT result = ID3D11Device_CreateBuffer(rose__device, &vertex_buffer_desc, NULL, &shape->vertices);
		CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

		shape->capacity = capacity;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = ID3D11DeviceContext_Map(rose__context, (ID3D11Resource*)shape->vertices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	CHECK_HRESULT(result, "ID3D11DeviceContext_Map()");

	memcpy(mapped.pData, vertices, sizeof(ROSE_VertexShape) * count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)shape->vertices, 0);

	shape->count = count;
}

