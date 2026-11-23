
/*
 *   Source [rose_texture.c]
 */

#include "rose_internal.h"

ROSE_Texture* ROSE_TextureCreate(ROSE_Image* image)
{
	ROSE_Texture* texture = calloc(1, sizeof(ROSE_Texture));

	if (image->pixels == NULL)
		return texture;

	D3D11_TEXTURE2D_DESC texture_desc = {
		.Width      = image->width,
		.Height     = image->height,
		.MipLevels  = 1,
		.ArraySize  = 1,
		.Format     = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.Usage      = D3D11_USAGE_IMMUTABLE,
		.BindFlags  = D3D11_BIND_SHADER_RESOURCE,
	};

	D3D11_SUBRESOURCE_DATA texture_data = {
		.pSysMem     = image->pixels,
		.SysMemPitch = image->width * sizeof(uInt32),
	};

	ID3D11Texture2D* texture2d = NULL;
	HRESULT result = ID3D11Device_CreateTexture2D(rose__device, &texture_desc, &texture_data, &texture2d);
	CHECK_HRESULT(result, "ID3D11Device_CreateTexture2D()");

	ID3D11ShaderResourceView* view = NULL;
	result = ID3D11Device_CreateShaderResourceView(rose__device, (ID3D11Resource*)texture2d, NULL, &view);
	CHECK_HRESULT(result, "ID3D11Device_CreateShaderResourceView()");

	ID3D11Texture2D_Release(texture2d);

	texture->view   = view;
	texture->width  = image->width;
	texture->height = image->height;

	return texture;
}

void ROSE_TextureDestroy(ROSE_Texture* texture)
{
	if (texture == NULL)
		return;

	if (texture->view != NULL)
		ID3D11ShaderResourceView_Release(texture->view);

	free(texture);
}

