// Implementation based on the article "Efficient Gaussian blur with linear sampling"
// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

static const float sampleWeights[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
static const float sampleOffsets[3] = { 0.0, 1.3846153846, 3.2307692308 };

texture2D frameTex2D;

sampler frameSampler = sampler_state
{
	texture = <frameTex2D>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

struct VSOUT
{
	float4 vertPos : POSITION;
	float2 UVCoord : TEXCOORD0;
};

struct VSIN
{
	float4 vertPos : POSITION0;
	float2 UVCoord : TEXCOORD0;
};

VSOUT FrameVS(VSIN IN)
{
	VSOUT OUT;
	OUT.vertPos = IN.vertPos;
	OUT.UVCoord = IN.UVCoord;
	return OUT;
}

float4 HGaussianBlurPS(VSOUT IN) : COLOR0
{
	float4 color = tex2D(frameSampler, IN.UVCoord) * sampleWeights[0];
	for(int i = 1; i < 3; ++i) {
		color += tex2D(frameSampler, IN.UVCoord + float2(sampleOffsets[i] * PIXEL_SIZE.x, 0.0)) * sampleWeights[i];
		color += tex2D(frameSampler, IN.UVCoord - float2(sampleOffsets[i] * PIXEL_SIZE.x, 0.0)) * sampleWeights[i];
	}
	return color;
}

float4 VGaussianBlurPS(VSOUT IN) : COLOR0
{
	float4 color = tex2D(frameSampler, IN.UVCoord) * sampleWeights[0];
	for(int i = 1; i < 3; ++i) {
		color += tex2D(frameSampler, IN.UVCoord + float2(0.0, sampleOffsets[i] * PIXEL_SIZE.y)) * sampleWeights[i];
		color += tex2D(frameSampler, IN.UVCoord - float2(0.0, sampleOffsets[i] * PIXEL_SIZE.y)) * sampleWeights[i];
	}
	return color;
}

technique t0
{
	pass P0
	{
		VertexShader = compile vs_3_0 FrameVS();
		PixelShader = compile ps_3_0 HGaussianBlurPS();
        ZEnable = false;
        AlphaBlendEnable = false;
	}

	pass P1
	{
		VertexShader = compile vs_3_0 FrameVS();
		PixelShader = compile ps_3_0 VGaussianBlurPS();
		ZEnable = false;
		AlphaBlendEnable = false;
	}
}
