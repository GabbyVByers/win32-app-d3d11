
/*
 *   Source [rose_mesh.c]
 */

#include "rose_internal.h"

ROSE_Mesh* ROSE_MeshCreate(const ROSE_VertexMesh* vertices, uInt32 count)
{
	ROSE_Mesh* mesh = calloc(1, sizeof(ROSE_Mesh));
	ROSE_MeshUpdate(mesh, vertices, count);
	return mesh;
}

void ROSE_MeshDestroy(ROSE_Mesh* mesh)
{
	if (mesh == NULL)
		return;

	if (mesh->vertices != NULL)
		ID3D11Buffer_Release(mesh->vertices);

	free(mesh);
}

void ROSE_MeshUpdate(ROSE_Mesh* mesh, const ROSE_VertexMesh* vertices, uInt32 count)
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
			.ByteWidth = sizeof(ROSE_VertexMesh) * capacity,
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

	memcpy(mapped.pData, vertices, sizeof(ROSE_VertexMesh) * count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)mesh->vertices, 0);

	mesh->count = count;
}

void ROSE_MeshDraw(ROSE_Mesh* mesh, ROSE_MeshDrawInfo draw, ROSE_Camera camera)
{
	ROSE__BatchFlush();

	bool empty =
		(mesh->vertices == NULL) ||
		(mesh->count == 0);

	if (empty)
		return;

	if ((draw.texture == NULL) || (draw.texture->view == NULL))
		draw.texture = rose__blank_texture;

	float aspect = rose__viewport.Width / rose__viewport.Height;

	ROSE_Matrix model   = ROSE_MatrixModel(draw.position, draw.orientation, draw.scale);
	ROSE_Matrix view    = ROSE_MatrixView(camera.position, camera.orientation);
	ROSE_Matrix project = ROSE_MatrixProject(camera.fov_degrees, aspect, camera.near_plane, camera.far_plane);

	rose__constants_mesh_t constants = {
		.mvp  = ROSE_MatrixMVP(model, view, project),
		.tint = draw.color,
	};

	ID3D11DeviceContext_UpdateSubresource(rose__context, (ID3D11Resource*)rose__constant_buffer_mesh, 0, NULL, &constants, 0, 0);

	UINT stride = sizeof(ROSE_VertexMesh);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(rose__context, rose__input_layout_mesh);
	ID3D11DeviceContext_IASetVertexBuffers(rose__context, 0, 1, &mesh->vertices, &stride, &offset);
	ROSE__BindPrimitive(draw.primitive, draw.point_diameter);

	ID3D11DeviceContext_VSSetShader(rose__context, rose__vertex_shader_mesh, NULL, 0);
	ID3D11DeviceContext_PSSetShader(rose__context, rose__pixel_shader_mesh, NULL, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(rose__context, 0, 1, &rose__constant_buffer_mesh);
	ID3D11DeviceContext_PSSetShaderResources(rose__context, 0, 1, &draw.texture->view);
	ID3D11DeviceContext_PSSetSamplers(rose__context, 0, 1, &rose__sampler);
	ID3D11DeviceContext_RSSetState(rose__context, NULL);

	ID3D11DeviceContext_OMSetBlendState(rose__context, rose__blend_state, NULL, 0xFFFFFFFF);
	ID3D11DeviceContext_OMSetDepthStencilState(rose__context, rose__depth_on, 0);

	ID3D11DeviceContext_Draw(rose__context, mesh->count, 0);
}

