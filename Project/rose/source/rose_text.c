
/*
 *   Source [rose_text.c]
 */

#include "rose_internal.h"

#define GLYPH_WIDTH  7
#define GLYPH_HEIGHT 11
#define GLYPH_COUNT  95
#define GLYPH_SPECIAL (GLYPH_COUNT - 1)

static void reserve(ROSE_Text*, uInt32);
static void characters(ROSE_Text*);

ROSE_Text* ROSE_TextCreate(const char* format, ...)
{
	ROSE_Text* text = calloc(1, sizeof(ROSE_Text));

	va_list args;
	va_start(args, format);

	va_list measure;
	va_copy(measure, args);
	int length = vsnprintf(NULL, 0, format, measure);
	va_end(measure);

	if (length < 0)
		length = 0;

	uInt32 count = (uInt32)length;

	reserve(text, count);
	vsnprintf(text->string, (size_t)count + 1, format, args);

	text->count = count;
	characters(text);
	va_end(args);

	return text;
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text == NULL)
		return;

	free(text->string);
	free(text->vertices);
	free(text);
}

void ROSE_TextUpdate(ROSE_Text* text, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	va_list measure;
	va_copy(measure, args);
	int length = vsnprintf(NULL, 0, format, measure);
	va_end(measure);

	if (length < 0)
		length = 0;

	uInt32 count = (uInt32)length;

	reserve(text, count);
	vsnprintf(text->string, (size_t)count + 1, format, args);

	text->count = count;
	characters(text);
	va_end(args);
}

void ROSE_TextDraw(ROSE_Text* text, ROSE_DrawInfo draw)
{
	for (uInt32 i = 0; i < text->vertex_count; i += 6) {
		ROSE_VertexShape* dst = ROSE__BatchAllocate(rose__ascii_texture, 6);

		for (uInt32 k = 0; k < 6; k++) {
			ROSE_VertexShape src = text->vertices[i + k];
			dst[k] = (ROSE_VertexShape) {
				.position.x = draw.position.x + src.position.x * draw.scale,
				.position.y = draw.position.y + src.position.y * draw.scale,
				.texcoord.x = src.texcoord.x,
				.texcoord.y = src.texcoord.y,
				.color = draw.color,
			};
		}
	}
}

// INTERNAL

static void reserve(ROSE_Text* text, uInt32 count)
{
	bool fits =
		(text->string != NULL) &&
		(count <= text->capacity);

	if (fits)
		return;

	uInt32 capacity = text->capacity * 2;

	if (capacity < count)
		capacity = count;

	if (capacity < 32)
		capacity = 32;

	text->string = realloc(text->string, (size_t)capacity + 1);
	text->vertices = realloc(text->vertices, sizeof(ROSE_VertexShape) * 6 * capacity);
	text->capacity = capacity;
}

static void characters(ROSE_Text* text)
{
	float pen_x = 0.0f;
	float pen_y = 0.0f;
	uInt32 n = 0;

	for (uInt32 i = 0; i < text->count; i++) {
		unsigned char c = (unsigned char)text->string[i];

		if (c == '\n') {
			pen_x = 0.0f;
			pen_y += (float)GLYPH_HEIGHT;
			continue;
		}

		uInt32 glyph = GLYPH_SPECIAL;
		if ((c >= ' ') && (c <= '}'))
			glyph = (uInt32)(c - ' ');

		float x0 = pen_x;
		float y0 = pen_y;
		float x1 = pen_x + (float)GLYPH_WIDTH;
		float y1 = pen_y + (float)GLYPH_HEIGHT;

		float u0 = (float)(glyph + 0) / (float)GLYPH_COUNT;
		float u1 = (float)(glyph + 1) / (float)GLYPH_COUNT;
		float v0 = 0.0f;
		float v1 = 1.0f;

		ROSE_VertexShape* verts = text->vertices + n;
		verts[0] = (ROSE_VertexShape){ x0, y0, u0, v0, ROSE_COLOR_WHITE };
		verts[1] = (ROSE_VertexShape){ x1, y0, u1, v0, ROSE_COLOR_WHITE };
		verts[2] = (ROSE_VertexShape){ x0, y1, u0, v1, ROSE_COLOR_WHITE };
		verts[3] = (ROSE_VertexShape){ x1, y0, u1, v0, ROSE_COLOR_WHITE };
		verts[4] = (ROSE_VertexShape){ x1, y1, u1, v1, ROSE_COLOR_WHITE };
		verts[5] = (ROSE_VertexShape){ x0, y1, u0, v1, ROSE_COLOR_WHITE };

		n += 6;
		pen_x += (float)GLYPH_WIDTH;
	}

	text->vertex_count = n;
}

