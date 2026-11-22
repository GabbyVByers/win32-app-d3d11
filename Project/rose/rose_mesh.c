
/*
 *   Source [rose_mesh.c]
 */

#define ROSE_EXTERN
#include "rose_internal.h"

ROSE_Mesh* ROSE_MeshCreate(const ROSE_Vertex2D* vertices, uInt32 count)
{
	ROSE_Mesh* mesh = calloc(1, sizeof(ROSE_Mesh));
	mesh->vertices = NULL;
	mesh->count = 0;

	const bool empty =
		!vertices ||
		count == 0;

	if (empty)
		return mesh;

	D3D11_BUFFER_DESC vertex_buffer_desc = {
		.ByteWidth = sizeof(ROSE_Vertex2D) * count,
		.Usage     = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};

	ID3D11Buffer* buffer = NULL;
	D3D11_SUBRESOURCE_DATA vertex_data = { .pSysMem = vertices };
	HRESULT result = ID3D11Device_CreateBuffer(rose__device, &vertex_buffer_desc, &vertex_data, &buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	mesh->vertices = buffer;
	mesh->count = count;
	
	return mesh;
}

void ROSE_MeshDestroy(ROSE_Mesh* mesh)
{
	if (!mesh)
		return;

	if (mesh->vertices)
		ID3D11Buffer_Release(mesh->vertices);

	free(mesh);
}

void ROSE_MeshDraw(ROSE_Mesh* mesh)
{
	const bool empty =
		!mesh->vertices ||
		mesh->count == 0;

	if (empty)
		return;

	UINT stride = sizeof(ROSE_Vertex2D);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(rose__context, rose__input_layout);
	ID3D11DeviceContext_IASetVertexBuffers(rose__context, 0, 1, &mesh->vertices, &stride, &offset);
	ID3D11DeviceContext_IASetPrimitiveTopology(rose__context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11DeviceContext_VSSetShader(rose__context, rose__vertex_shader, NULL, 0);
	ID3D11DeviceContext_PSSetShader(rose__context, rose__pixel_shader, NULL, 0);

	constants_t constants = { .screen_size = { rose__viewport.Width, rose__viewport.Height } };
	ID3D11DeviceContext_UpdateSubresource(rose__context, (ID3D11Resource*)rose__constant_buffer, 0, NULL, &constants, 0, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(rose__context, 0, 1, &rose__constant_buffer);

	ID3D11DeviceContext_Draw(rose__context, mesh->count, 0);
}

void ROSE_MeshUpdate(ROSE_Mesh* mesh, const ROSE_Vertex2D* vertices, uInt32 count)
{
	if (mesh->vertices)
		ID3D11Buffer_Release(mesh->vertices);

	D3D11_BUFFER_DESC vertex_buffer_desc = {
		.ByteWidth = sizeof(ROSE_Vertex2D) * count,
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};

	mesh->vertices = NULL;
	mesh->count = count;

	D3D11_SUBRESOURCE_DATA vertex_data = { .pSysMem = vertices };
	HRESULT result = ID3D11Device_CreateBuffer(rose__device, &vertex_buffer_desc, &vertex_data, &mesh->vertices);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");
}

