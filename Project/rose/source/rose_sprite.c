
/*
 *   Source [rose_sprite.c]
 */

#include "rose_internal.h"

#define BATCH_VERTICES (2048 * 6)

static ID3D11Buffer* batch_buffer  = NULL;
static uInt32        batch_cursor  = 0;
static uInt32        batch_count   = 0;
static ROSE_Texture* batch_texture = NULL;
static ROSE_Vertex2D batch_vertices[BATCH_VERTICES];

ROSE_Sprite* ROSE_SpriteCreate(ROSE_Texture* texture)
{
	if ((texture == NULL) || (texture->view == NULL))
		texture = rose__blank_texture;

	ROSE_Sprite* sprite = calloc(1, sizeof(ROSE_Sprite));

	sprite->u0 = 0.0f; sprite->v0 = 0.0f;
	sprite->u1 = 1.0f; sprite->v1 = 1.0f;

	sprite->width   = (float)texture->width;
	sprite->height  = (float)texture->height;
	sprite->texture = texture;

	return sprite;
}

void ROSE_SpriteDestroy(ROSE_Sprite* sprite)
{
	free(sprite);
}

void ROSE_SpriteRegion(ROSE_Sprite* sprite, uInt32 xpos, uInt32 ypos, uInt32 width, uInt32 height)
{
	float tw = (float)sprite->texture->width;
	float th = (float)sprite->texture->height;

	sprite->u0 = (float)xpos / tw;
	sprite->v0 = (float)ypos / th;
	sprite->u1 = (float)(xpos + width)  / tw;
	sprite->v1 = (float)(ypos + height) / th;
	sprite->width  = (float)width;
	sprite->height = (float)height;
}

void ROSE_SpriteDraw(ROSE_Sprite* sprite, ROSE_Vec2 position, float scale, ROSE_Color color)
{
	float x0 = position.x;
	float y0 = position.y;
	float x1 = position.x + sprite->width  * scale;
	float y1 = position.y + sprite->height * scale;

	float u0 = sprite->u0, v0 = sprite->v0;
	float u1 = sprite->u1, v1 = sprite->v1;

	ROSE_Vertex2D* verts = ROSE__BatchAllocate(sprite->texture, 6);
	verts[0] = (ROSE_Vertex2D){ x0, y0, u0, v0, color };
	verts[1] = (ROSE_Vertex2D){ x1, y0, u1, v0, color };
	verts[2] = (ROSE_Vertex2D){ x0, y1, u0, v1, color };
	verts[3] = (ROSE_Vertex2D){ x1, y0, u1, v0, color };
	verts[4] = (ROSE_Vertex2D){ x1, y1, u1, v1, color };
	verts[5] = (ROSE_Vertex2D){ x0, y1, u0, v1, color };
}

ROSE_Vertex2D* ROSE__BatchAllocate(ROSE_Texture* texture, uInt32 count)
{
	if (texture != batch_texture) {
		ROSE__BatchFlush();
		batch_texture = texture;
	}

	if ((batch_count + count) > BATCH_VERTICES)
		ROSE__BatchFlush();

	ROSE_Vertex2D* vertices = batch_vertices + batch_count;
	batch_count += count;
	return vertices;
}

void ROSE__BatchCreate(void)
{
	D3D11_BUFFER_DESC desc = {
		.ByteWidth      = sizeof(ROSE_Vertex2D) * BATCH_VERTICES,
		.Usage          = D3D11_USAGE_DYNAMIC,
		.BindFlags      = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	HRESULT result = ID3D11Device_CreateBuffer(rose__device, &desc, NULL, &batch_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	batch_cursor  = BATCH_VERTICES;
	batch_count   = 0;
	batch_texture = NULL;
}

void ROSE__BatchDestroy(void)
{
	if (batch_buffer != NULL)
		ID3D11Buffer_Release(batch_buffer);

	batch_buffer  = NULL;
	batch_cursor  = 0;
	batch_count   = 0;
	batch_texture = NULL;
}

void ROSE__BatchFlush(void)
{
	if (batch_count == 0)
		return;

	D3D11_MAP mode = D3D11_MAP_WRITE_NO_OVERWRITE;
	bool wrap = (batch_cursor + batch_count) > BATCH_VERTICES;

	if (wrap) {
		mode = D3D11_MAP_WRITE_DISCARD;
		batch_cursor = 0;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = ID3D11DeviceContext_Map(rose__context, (ID3D11Resource*)batch_buffer, 0, mode, 0, &mapped);
	CHECK_HRESULT(result, "ID3D11DeviceContext_Map()");

	ROSE_Vertex2D* dst = (ROSE_Vertex2D*)mapped.pData + batch_cursor;
	memcpy(dst, batch_vertices, sizeof(ROSE_Vertex2D) * batch_count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)batch_buffer, 0);

	ROSE__BindPipeline2D(batch_buffer, batch_texture);
	ID3D11DeviceContext_Draw(rose__context, batch_count, batch_cursor);

	batch_cursor += batch_count;
	batch_count = 0;
}

