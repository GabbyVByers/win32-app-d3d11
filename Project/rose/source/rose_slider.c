
/*
 *   Source [rose_slider.c]
 */

#include "rose_internal.h"

static ROSE_Shape* rect(float width, float height);
static ROSE_Color  tint(ROSE_Color base, ROSE_Color tint);

ROSE_Slider* ROSE_SliderCreate(ROSE_Vec2 dimensions, float min, float max, float* value)
{
	if (dimensions.y < 1.0f)
		dimensions.y = 1.0f;

	if (dimensions.x < dimensions.y)
		dimensions.x = dimensions.y;

	if (max < min) {
		float swap = min;
		min = max;
		max = swap;
	}

	ROSE_Slider* slider = calloc(1, sizeof(ROSE_Slider));

	slider->value      = value;
	slider->dimensions = dimensions;
	slider->min        = min;
	slider->max        = max;
	slider->track      = rect(dimensions.x, dimensions.y);
	slider->knob       = rect(dimensions.y, dimensions.y);

	return slider;
}

void ROSE_SliderDestroy(ROSE_Slider* slider)
{
	if (slider == NULL)
		return;

	ROSE_ShapeDestroy(slider->track);
	ROSE_ShapeDestroy(slider->knob);
	free(slider);
}

bool ROSE_SliderDraw(ROSE_Slider* slider, ROSE_DrawInfo draw)
{
	float width  = slider->dimensions.x * draw.scale;
	float height = slider->dimensions.y * draw.scale;
	float travel = width - height;
	float range  = slider->max - slider->min;

	ROSE_Vec2 position = draw.position;
	ROSE_Vec2 mouse    = ROSE_MousePosition();

	float t = 0.0f;
	if ((slider->value != NULL) && (range > 0.0f))
		t = (*slider->value - slider->min) / range;

	if (t < 0.0f)
		t = 0.0f;

	if (t > 1.0f)
		t = 1.0f;

	float knob_x = position.x + t * travel;

	slider->hovered =
		(mouse.x >= position.x) &&
		(mouse.x <= position.x + width) &&
		(mouse.y >= position.y) &&
		(mouse.y <= position.y + height);

	if (slider->hovered && ROSE_MousePressed(ROSE_MOUSE_LEFT))
		slider->dragging = true;

	bool changed = false;

	if (slider->dragging && (travel > 0.0f) && (slider->value != NULL)) {
		t = ROSE_Clamp((mouse.x - position.x - height * 0.5f) / travel, 0.0f, 1.0f);
		knob_x = position.x + t * travel;

		float value = ROSE_Lerp(slider->min, slider->max, t);
		changed = (*slider->value != value);
		*slider->value = value;
	}

	if (!ROSE_MousePressing(ROSE_MOUSE_LEFT))
		slider->dragging = false;

	bool active = slider->hovered || slider->dragging;

	ROSE_ShapeDrawInfo track = ROSE_SHAPE_DRAW_DEFAULT;
	track.position = position;
	track.scale    = draw.scale;
	track.color    = tint(ROSE_COLOR_GRAY, draw.color);
	ROSE_ShapeDraw(slider->track, track);

	ROSE_ShapeDrawInfo knob = ROSE_SHAPE_DRAW_DEFAULT;
	knob.position = (ROSE_Vec2) { knob_x, position.y };
	knob.scale    = draw.scale;
	knob.color    = tint(active ? ROSE_COLOR_WHITE : ROSE_COLOR_RGBA(190, 190, 190, 255), draw.color);
	ROSE_ShapeDraw(slider->knob, knob);

	return changed;
}

// INTERNAL

static ROSE_Shape* rect(float width, float height)
{
	ROSE_VertexShape vertices[6] = {
		{ { 0.0f,  0.0f   }, { 0.0f, 0.0f }, ROSE_COLOR_WHITE },
		{ { width, 0.0f   }, { 1.0f, 0.0f }, ROSE_COLOR_WHITE },
		{ { 0.0f,  height }, { 0.0f, 1.0f }, ROSE_COLOR_WHITE },
		{ { width, 0.0f   }, { 1.0f, 0.0f }, ROSE_COLOR_WHITE },
		{ { width, height }, { 1.0f, 1.0f }, ROSE_COLOR_WHITE },
		{ { 0.0f,  height }, { 0.0f, 1.0f }, ROSE_COLOR_WHITE },
	};

	return ROSE_ShapeCreate(vertices, 6);
}

static ROSE_Color tint(ROSE_Color base, ROSE_Color tint)
{
	return (ROSE_Color) {
		base.r * tint.r,
		base.g * tint.g,
		base.b * tint.b,
		base.a * tint.a,
	};
}

