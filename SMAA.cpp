/**
 * Copyright (C) 2011 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2011 Belen Masia (bmasia@unizar.es) 
 * Copyright (C) 2011 Jose I. Echevarria (joseignacioechevarria@gmail.com) 
 * Copyright (C) 2011 Fernando Navarro (fernandn@microsoft.com) 
 * Copyright (C) 2011 Diego Gutierrez (diegog@unizar.es)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the following disclaimer
 *       in the documentation and/or other materials provided with the 
 *       distribution:
 * 
 *      "Uses SMAA. Copyright (C) 2011 by Jorge Jimenez, Jose I. Echevarria,
 *       Belen Masia, Fernando Navarro and Diego Gutierrez."
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS 
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holders.
 */


#include <vector>
#include <sstream>
#include "SMAA.h"
#include "SearchTex.h"
#include "AreaTex.h"
using namespace std;

#include "Settings.h"


#pragma region Useful Macros from DXUT (copy-pasted here as we prefer this to be as self-contained as possible)
#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x) { hr = (x); if (FAILED(hr)) { DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x) { hr = (x); if (FAILED(hr)) { return DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); } }
#endif
#else
#ifndef V
#define V(x) { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x) { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif
#pragma endregion

SMAA::SMAA(IDirect3DDevice9 *device, int width, int height, Preset preset, const ExternalStorage &storage)
        : Effect(device),
          threshold(0.1f),
          maxSearchSteps(8),
          width(width), height(height) {
    HRESULT hr;

    // Setup the defines for compiling the effect.
    vector<D3DXMACRO> defines;
    stringstream s;

    // Setup pixel size macro
    s << "float2(1.0 / " << width << ", 1.0 / " << height << ")";
    string pixelSizeText = s.str();
    D3DXMACRO pixelSizeMacro = { "SMAA_PIXEL_SIZE", pixelSizeText.c_str() };
    defines.push_back(pixelSizeMacro);

    // Setup preset macro
    D3DXMACRO presetMacros[] = {
        { "SMAA_PRESET_LOW", "1" },
        { "SMAA_PRESET_MEDIUM", "1" },
        { "SMAA_PRESET_HIGH", "1" },
        { "SMAA_PRESET_ULTRA", "1" },
        { "SMAA_PRESET_CUSTOM", "1" }
    };
    defines.push_back(presetMacros[int(preset)]);

    D3DXMACRO null = { NULL, NULL };
    defines.push_back(null);

    // Setup the flags for the effect.
    DWORD flags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_OPTIMIZATION_LEVEL3;
    #ifdef D3DXFX_LARGEADDRESS_HANDLE
    flags |= D3DXFX_LARGEADDRESSAWARE;
    #endif

	// Load effect from file
	SDLOG(0, "SMAA load\n");	
	ID3DXBuffer* errors;
	hr = D3DXCreateEffectFromFile(device, GetDirectoryFile("dsfix\\SMAA.fx"), &defines.front(), NULL, flags, NULL, &effect, &errors);
	if(hr != D3D_OK) SDLOG(0, "ERRORS:\n %s\n", errors->GetBufferPointer());

    // If storage for the edges is not specified we will create it.
    if (storage.edgeTex != NULL && storage.edgeSurface != NULL) {
        edgeTex = storage.edgeTex;
        edgeSurface = storage.edgeSurface;
        releaseEdgeResources = false;
    } else {
        V(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &edgeTex, NULL));
        V(edgeTex->GetSurfaceLevel(0, &edgeSurface));
        releaseEdgeResources = true;
    }

    // Same for blending weights.
    if (storage.blendTex != NULL && storage.blendSurface != NULL) {
        blendTex = storage.blendTex;
        blendSurface = storage.blendSurface;
        releaseBlendResources = false;
    } else {
        V(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &blendTex, NULL));
        V(blendTex->GetSurfaceLevel(0, &blendSurface));
        releaseBlendResources = true;
    }

    // Load the precomputed textures.
    loadAreaTex();
    loadSearchTex();

    // Create some handles for techniques and variables.
    thresholdHandle = effect->GetParameterByName(NULL, "threshold");
    maxSearchStepsHandle = effect->GetParameterByName(NULL, "maxSearchSteps");
    areaTexHandle = effect->GetParameterByName(NULL, "areaTex2D");
    searchTexHandle = effect->GetParameterByName(NULL, "searchTex2D");
    colorTexHandle = effect->GetParameterByName(NULL, "colorTex2D");
    depthTexHandle = effect->GetParameterByName(NULL, "depthTex2D");
    edgesTexHandle = effect->GetParameterByName(NULL, "edgesTex2D");
    blendTexHandle = effect->GetParameterByName(NULL, "blendTex2D");
    lumaEdgeDetectionHandle = effect->GetTechniqueByName("LumaEdgeDetection");
    colorEdgeDetectionHandle = effect->GetTechniqueByName("ColorEdgeDetection");
    depthEdgeDetectionHandle = effect->GetTechniqueByName("DepthEdgeDetection");
    blendWeightCalculationHandle = effect->GetTechniqueByName("BlendWeightCalculation");
    neighborhoodBlendingHandle = effect->GetTechniqueByName("NeighborhoodBlending");
}


SMAA::~SMAA() {
    SAFERELEASE(effect);

    if(releaseEdgeResources) { // We will be releasing these things *only* if we created them.
        SAFERELEASE(edgeTex);
        SAFERELEASE(edgeSurface);
    }

    if(releaseBlendResources) { // Same applies over here.
        SAFERELEASE(blendTex);
        SAFERELEASE(blendSurface);
    }

    SAFERELEASE(areaTex);
    SAFERELEASE(searchTex);
}


void SMAA::go(IDirect3DTexture9 *edges,
              IDirect3DTexture9 *src, 
              IDirect3DSurface9 *dst,
              Input input) {
    HRESULT hr;

    // Setup the layout for our fullscreen quad.
    V(device->SetVertexDeclaration(vertexDeclaration));

    // And here we go!
    edgesDetectionPass(edges, input); 
    blendingWeightsCalculationPass();
    neighborhoodBlendingPass(src, dst);
}


void SMAA::loadAreaTex() {
    HRESULT hr;
    V(device->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8L8, D3DPOOL_DEFAULT, &areaTex, NULL));
    D3DLOCKED_RECT rect;
    V(areaTex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD));
    for (int i = 0; i < AREATEX_HEIGHT; i++)
        CopyMemory(((char *) rect.pBits) + i * rect.Pitch, areaTexBytes + i * AREATEX_PITCH, AREATEX_PITCH);
    V(areaTex->UnlockRect(0));
}


void SMAA::loadSearchTex() {
    HRESULT hr;
    V(device->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &searchTex, NULL));
    D3DLOCKED_RECT rect;
    V(searchTex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD));
    for (int i = 0; i < SEARCHTEX_HEIGHT; i++)
        CopyMemory(((char *) rect.pBits) + i * rect.Pitch, searchTexBytes + i * SEARCHTEX_PITCH, SEARCHTEX_PITCH);
    V(searchTex->UnlockRect(0));
}


void SMAA::edgesDetectionPass(IDirect3DTexture9 *edges, Input input) {
    //D3DPERF_BeginEvent(D3DCOLOR_XRGB(0, 0, 0), L"SMAA: 1st pass");
    HRESULT hr;

    // Set the render target and clear both the color and the stencil buffers.
    V(device->SetRenderTarget(0, edgeSurface));
    V(device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0));

    // Setup variables.
    V(effect->SetFloat(thresholdHandle, threshold));
    V(effect->SetFloat(maxSearchStepsHandle, float(maxSearchSteps)));

    // Select the technique accordingly.
    switch (input) {
        case INPUT_LUMA:
            V(effect->SetTexture(colorTexHandle, edges));
            V(effect->SetTechnique(lumaEdgeDetectionHandle));
            break;
        case INPUT_COLOR:
            V(effect->SetTexture(colorTexHandle, edges));
            V(effect->SetTechnique(colorEdgeDetectionHandle));
            break;
        case INPUT_DEPTH:
            V(effect->SetTexture(depthTexHandle, edges));
            V(effect->SetTechnique(depthEdgeDetectionHandle));
            break;
        default:
            throw logic_error("unexpected error");
    }

    // Do it!
    UINT passes;
    V(effect->Begin(&passes, 0));
    V(effect->BeginPass(0));
    quad(width, height);
    V(effect->EndPass());
    V(effect->End());

    //D3DPERF_EndEvent();
}


void SMAA::blendingWeightsCalculationPass() {
    //D3DPERF_BeginEvent(D3DCOLOR_XRGB(0, 0, 0), L"SMAA: 2nd pass");
    HRESULT hr;

    // Set the render target and clear it.
    V(device->SetRenderTarget(0, blendSurface));
    V(device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0));

    // Setup the variables and the technique (yet again).
    V(effect->SetTexture(edgesTexHandle, edgeTex));
    V(effect->SetTexture(areaTexHandle, areaTex));
    V(effect->SetTexture(searchTexHandle, searchTex));
    V(effect->SetTechnique(blendWeightCalculationHandle));

    // And here we go!
    UINT passes;
    V(effect->Begin(&passes, 0));
    V(effect->BeginPass(0));
    quad(width, height);
    V(effect->EndPass());
    V(effect->End());

    //D3DPERF_EndEvent();
}


void SMAA::neighborhoodBlendingPass(IDirect3DTexture9 *src, IDirect3DSurface9 *dst) {
    //D3DPERF_BeginEvent(D3DCOLOR_XRGB(0, 0, 0), L"SMAA: 3rd pass");
    HRESULT hr;

    // Blah blah blah
    V(device->SetRenderTarget(0, dst));
    V(effect->SetTexture(colorTexHandle, src));
    V(effect->SetTexture(blendTexHandle, blendTex));
    V(effect->SetTechnique(neighborhoodBlendingHandle));

    // Yeah! We will finally have the antialiased image :D
    UINT passes;
    V(effect->Begin(&passes, 0));
    V(effect->BeginPass(0));
    quad(width, height);
    V(effect->EndPass());
    V(effect->End());

    //D3DPERF_EndEvent();
}
