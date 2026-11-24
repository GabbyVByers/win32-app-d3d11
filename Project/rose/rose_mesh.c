
/*
 *   Source [rose_mesh.c]
 */

#include "rose_internal.h"

ROSE_Mesh2D* ROSE_Mesh2DCreate(const ROSE_Vertex2D* vertices, uInt32 count)
{
	ROSE_Mesh2D* mesh = calloc(1, sizeof(ROSE_Mesh2D));
	ROSE_Mesh2DUpdate(mesh, vertices, count);
	return mesh;
}

void ROSE_Mesh2DDestroy(ROSE_Mesh2D* mesh)
{
	if (mesh == NULL)
		return;

	if (mesh->vertices != NULL)
		ID3D11Buffer_Release(mesh->vertices);

	free(mesh);
}

void ROSE_Mesh2DDraw(ROSE_Mesh2D* mesh, ROSE_Texture* texture)
{
	rose__batch_flush();

	bool empty =
		(mesh->vertices == NULL) ||
		(mesh->count == 0);

	if (empty)
		return;

	rose__bind_2d(mesh->vertices, texture);
	ID3D11DeviceContext_Draw(rose__context, mesh->count, 0);
}

void ROSE_Mesh2DUpdate(ROSE_Mesh2D* mesh, const ROSE_Vertex2D* vertices, uInt32 count)
{
	bool empty =
		(vertices == NULL) ||
		(count == 0);

	if (empty) {
		mesh->count = 0;
		return;
	}

	if (count > mesh->capacity) {
		uInt32 capacity = (mesh->capacity * 2);

		if (capacity < count)
			capacity = count;

		rose__mesh_reserve(mesh, capacity);
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = ID3D11DeviceContext_Map(rose__context, (ID3D11Resource*)mesh->vertices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	CHECK_HRESULT(result, "ID3D11DeviceContext_Map()");

	memcpy(mapped.pData, vertices, sizeof(ROSE_Vertex2D) * count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)mesh->vertices, 0);

	mesh->count = count;
}

void rose__mesh_reserve(ROSE_Mesh2D* mesh, uInt32 capacity)
{
	if (capacity <= mesh->capacity)
		return;

	if (mesh->vertices != NULL)
		ID3D11Buffer_Release(mesh->vertices);

	D3D11_BUFFER_DESC vertex_buffer_desc = {
		.ByteWidth      = sizeof(ROSE_Vertex2D) * capacity,
		.Usage          = D3D11_USAGE_DYNAMIC,
		.BindFlags      = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	mesh->vertices = NULL;
	HRESULT result = ID3D11Device_CreateBuffer(rose__device, &vertex_buffer_desc, NULL, &mesh->vertices);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	mesh->capacity = capacity;
}

