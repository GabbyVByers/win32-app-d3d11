
/*
 *   Source [rose_mesh.c]
 */

#include "rose_internal.h"

// MESH 2D

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

void ROSE_Mesh2DDraw(ROSE_Mesh2D* mesh, ROSE_Texture* texture, ROSE_Vec2 position, float scale, ROSE_Color color, ROSE_PRIMITIVE primitive, float diameter)
{
	ROSE__BatchFlush();

	bool empty =
		(mesh->vertices == NULL) ||
		(mesh->count == 0);

	if (empty)
		return;

	ROSE__BindPipeline2D(mesh->vertices, texture, position, scale, color, primitive, diameter);
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

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = ID3D11DeviceContext_Map(rose__context, (ID3D11Resource*)mesh->vertices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	CHECK_HRESULT(result, "ID3D11DeviceContext_Map()");

	memcpy(mapped.pData, vertices, sizeof(ROSE_Vertex2D) * count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)mesh->vertices, 0);

	mesh->count = count;
}

// MESH 3D

ROSE_Mesh3D* ROSE_Mesh3DCreate(const ROSE_Vertex3D* vertices, uInt32 count)
{
	ROSE_Mesh3D* mesh = calloc(1, sizeof(ROSE_Mesh3D));
	ROSE_Mesh3DUpdate(mesh, vertices, count);
	return mesh;
}

void ROSE_Mesh3DDestroy(ROSE_Mesh3D* mesh)
{
	if (mesh == NULL)
		return;

	if (mesh->vertices != NULL)
		ID3D11Buffer_Release(mesh->vertices);

	free(mesh);
}

void ROSE_Mesh3DUpdate(ROSE_Mesh3D* mesh, const ROSE_Vertex3D* vertices, uInt32 count)
{
	bool empty =
		(vertices == NULL) ||
		(count == 0);

	if (empty) {
		mesh->count = 0;
		return;
	}

	if (count > mesh->capacity) {
		uInt32 capacity = mesh->capacity * 2;

		if (capacity < count)
			capacity = count;

		if (mesh->vertices != NULL)
			ID3D11Buffer_Release(mesh->vertices);

		D3D11_BUFFER_DESC vertex_buffer_desc = {
			.ByteWidth = sizeof(ROSE_Vertex3D) * capacity,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		};

		mesh->vertices = NULL;
		HRESULT result = ID3D11Device_CreateBuffer(rose__device, &vertex_buffer_desc, NULL, &mesh->vertices);
		CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

		mesh->capacity = capacity;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = ID3D11DeviceContext_Map(rose__context, (ID3D11Resource*)mesh->vertices, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	CHECK_HRESULT(result, "ID3D11DeviceContext_Map()");

	memcpy(mapped.pData, vertices, sizeof(ROSE_Vertex3D) * count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)mesh->vertices, 0);

	mesh->count = count;
}

void ROSE_Mesh3DDraw(ROSE_Mesh3D* mesh, ROSE_Texture* texture, ROSE_Camera camera, ROSE_Quat orientation, ROSE_Vec3 position, float scale, ROSE_Color color, ROSE_PRIMITIVE primitive, float diameter)
{
	ROSE__BatchFlush();

	bool empty =
		(mesh->vertices == NULL) ||
		(mesh->count == 0);

	if (empty)
		return;

	if ((texture == NULL) || (texture->view == NULL))
		texture = rose__blank_texture;

	float aspect = rose__viewport.Width / rose__viewport.Height;

	ROSE_Matrix model   = ROSE_MatrixModel(position, orientation, scale);
	ROSE_Matrix view    = ROSE_MatrixView(camera.position, camera.orientation);
	ROSE_Matrix project = ROSE_MatrixProject(camera.fov, aspect, camera.near_plane, camera.far_plane);

	rose__constants3d_t constants = {
		.mvp  = ROSE_MatrixMVP(model, view, project),
		.tint = color,
	};

	ID3D11DeviceContext_UpdateSubresource(rose__context, (ID3D11Resource*)rose__constant_buffer_3d, 0, NULL, &constants, 0, 0);

	UINT stride = sizeof(ROSE_Vertex3D);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(rose__context, rose__input_layout_3d);
	ID3D11DeviceContext_IASetVertexBuffers(rose__context, 0, 1, &mesh->vertices, &stride, &offset);
	ROSE__BindPrimitive(primitive, diameter);

	ID3D11DeviceContext_VSSetShader(rose__context, rose__vertex_shader_3d, NULL, 0);
	ID3D11DeviceContext_PSSetShader(rose__context, rose__pixel_shader_3d, NULL, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(rose__context, 0, 1, &rose__constant_buffer_3d);
	ID3D11DeviceContext_PSSetShaderResources(rose__context, 0, 1, &texture->view);
	ID3D11DeviceContext_PSSetSamplers(rose__context, 0, 1, &rose__sampler);
	ID3D11DeviceContext_RSSetState(rose__context, NULL);

	ID3D11DeviceContext_OMSetBlendState(rose__context, rose__blend_state, NULL, 0xFFFFFFFF);
	ID3D11DeviceContext_OMSetDepthStencilState(rose__context, rose__depth_on, 0);

	ID3D11DeviceContext_Draw(rose__context, mesh->count, 0);
}

