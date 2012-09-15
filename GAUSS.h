#pragma once

#include <dxgi.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#include "Effect.h"

class GAUSS : public Effect {
public:
    GAUSS(IDirect3DDevice9 *device, int width, int height);
    virtual ~GAUSS();

	void go(IDirect3DTexture9 *input, IDirect3DSurface9 *dst);

private:
	int width, height;

	ID3DXEffect *effect;
	
	IDirect3DTexture9* buffer1Tex;
	IDirect3DSurface9* buffer1Surf;

	D3DXHANDLE frameTexHandle;
};
