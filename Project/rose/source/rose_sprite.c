
/*
 *   Source [rose_sprite.c]
 */

#include "rose_internal.h"

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

void ROSE_SpriteDraw(ROSE_Sprite* sprite, ROSE_DrawInfo draw)
{
	float x0 = draw.position.x;
	float y0 = draw.position.y;
	float x1 = draw.position.x + sprite->width  * draw.scale;
	float y1 = draw.position.y + sprite->height * draw.scale;

	float u0 = sprite->u0, v0 = sprite->v0;
	float u1 = sprite->u1, v1 = sprite->v1;

	ROSE_VertexShape* verts = ROSE__BatchAllocate(sprite->texture, 6);
	verts[0] = (ROSE_VertexShape){ x0, y0, u0, v0, draw.color };
	verts[1] = (ROSE_VertexShape){ x1, y0, u1, v0, draw.color };
	verts[2] = (ROSE_VertexShape){ x0, y1, u0, v1, draw.color };
	verts[3] = (ROSE_VertexShape){ x1, y0, u1, v0, draw.color };
	verts[4] = (ROSE_VertexShape){ x1, y1, u1, v1, draw.color };
	verts[5] = (ROSE_VertexShape){ x0, y1, u0, v1, draw.color };
}

