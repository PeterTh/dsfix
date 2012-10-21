// Volumetric SSAO
// Implemented by Tomerk for OBGE
// Adapted and tweaked for Dark Souls by Durante

/***User-controlled variables***/
#define N_SAMPLES 9 //number of samples, currently do not change.

extern float scale = 1; //downsampling scale, 1 is highest quality but slowest
extern float aoRadiusMultiplier = 1.0; //Linearly multiplies the radius of the AO Sampling
extern float ThicknessModel = 25.0; //units in space the AO assumes objects' thicknesses are
extern float FOV = 85; //Field of View in Degrees
extern float luminosity_threshold = 0.3;

#ifndef SSAO_STRENGTH_LOW
#ifndef SSAO_STRENGTH_MEDIUM
#ifndef SSAO_STRENGTH_HIGH
#define SSAO_STRENGTH_MEDIUM 1
#endif
#endif
#endif

#ifdef SSAO_STRENGTH_LOW
extern float aoClamp = 0.75;
extern float aoStrengthMultiplier = 0.6;
#endif

#ifdef SSAO_STRENGTH_MEDIUM
extern float aoClamp = 0.5;
extern float aoStrengthMultiplier = 0.8;
#endif

#ifdef SSAO_STRENGTH_HIGH
extern float aoClamp = 0.2;
extern float aoStrengthMultiplier = 1.2;
#endif


#define LUMINANCE_CONSIDERATION //comment this line to not take pixel brightness into account

/***End Of User-controlled Variables***/
static float2 rcpres = PIXEL_SIZE;
static float aspect = rcpres.y/rcpres.x;
static const float nearZ = 1.0;
static const float farZ = 5000.0;
static const float2 g_InvFocalLen = { tan(0.5f*radians(FOV)) / rcpres.y * rcpres.x, tan(0.5f*radians(FOV)) };
static const float depthRange = nearZ-farZ;

texture2D depthTex2D;
texture2D frameTex2D;
texture2D prevPassTex2D;
texture1D thicknessTex1D;

sampler depthSampler = sampler_state
{
	texture = <depthTex2D>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

sampler frameSampler = sampler_state
{
	texture = <frameTex2D>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

sampler passSampler = sampler_state
{
	texture = <prevPassTex2D>;
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

static float2 sample_offset[N_SAMPLES] =
{
//#if N_SAMPLES >= 9
	float2(-0.1376476f, 0.2842022f),
	float2(-0.626618f, 0.4594115f),
	float2(-0.8903138f, -0.05865424f),
	float2(0.2871419f, 0.8511679f),
	float2(-0.1525251f, -0.3870117f),
	float2(0.6978705f, -0.2176773f),
	float2(0.7343006f, 0.3774331f),
	float2(0.1408805f, -0.88915f),
	float2(-0.6642616f, -0.543601f)
//#endif
};

static float sample_radius[N_SAMPLES] =
{
//#if N_SAMPLES >= 9
	0.948832,
	0.629516,
	0.451554,
	0.439389,
	0.909372,
	0.682344,
	0.5642,
	0.4353,
	0.5130
//#endif
};


float2 rand(in float2 uv) {
	float noiseX = (frac(sin(dot(uv, float2(12.9898,78.233)*2.0)) * 43758.5453));
	float noiseY = sqrt(1-noiseX*noiseX);
	//float noiseY = (frac(sin(dot(uv, float2(12.9898,78.233))) * 43758.5453));
	return float2(noiseX, noiseY);
}

float readDepth(in float2 coord : TEXCOORD0) {
	float4 col = tex2D(depthSampler, coord);
	//float posZ = clamp((((1.0-col.z)/(256.0) + (1.0-col.y) + (1.0-col.x)*256))/5.0, 0.0, 1.0);
	//float posZ = ((1.0-col.y) + 256.0*(1.0-col.x))/(5.0);
	//return ((2.0f * nearZ) / (nearZ + farZ - posZ * (farZ - nearZ)))*20.0;
	
	float posZ = ((1.0-col.z) + (1.0-col.y)*256.0 + (1.0-col.x)*(257.0*256.0));
	//return ((2.0f * nearZ) / (nearZ + farZ - posZ * (farZ - nearZ)));
	return (posZ-nearZ)/farZ;
}

float3 getPosition(in float2 uv, in float eye_z) {
   uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
   float3 pos = float3(uv * g_InvFocalLen * eye_z, eye_z );
   return pos;
}

float4 ssao_Main( VSOUT IN ) : COLOR0 {
	clip(1/scale-IN.UVCoord.x);
	clip(1/scale-IN.UVCoord.y);	
	IN.UVCoord.xy *= scale;

	float depth = readDepth(IN.UVCoord);
	float3 pos = getPosition(IN.UVCoord, depth);
	float3 dx = ddx(pos);
	float3 dy = ddy(pos);
	float3 norm = normalize(cross(dx,dy));
	norm.y *= -1;

	float sample_depth;

	float ao=0;
	float s=0.0;

	float2 rand_vec = rand(IN.UVCoord);
	float2 sample_vec_divisor = g_InvFocalLen*depth*depthRange/(aoRadiusMultiplier*5000*rcpres);
	float2 sample_center = IN.UVCoord + norm.xy/sample_vec_divisor*float2(1,aspect);
	float sample_center_depth = depth*depthRange + norm.z*aoRadiusMultiplier*10;
	
	for(int i = 0; i < N_SAMPLES; i++) {
		float2 sample_vec = reflect(sample_offset[i], rand_vec);
		sample_vec /= sample_vec_divisor;
		float2 sample_coords = sample_center + sample_vec*float2(1,aspect);
		
		float curr_sample_radius = sample_radius[i]*aoRadiusMultiplier*10;
		float curr_sample_depth = depthRange*readDepth(sample_coords);
		
		ao += clamp(0,curr_sample_radius+sample_center_depth-curr_sample_depth,2*curr_sample_radius);
		ao -= clamp(0,curr_sample_radius+sample_center_depth-curr_sample_depth-ThicknessModel,2*curr_sample_radius);
		s += 2*curr_sample_radius;
	}

	ao /= s;
	
	// adjust for close and far away
	if(depth<0.1) ao = lerp(ao, 0.0, (0.1-depth)*10.0);
	//if(depth>0.1) ao = lerp(ao, 0.0, 1.0);

	ao = 1.0-ao*aoStrengthMultiplier;
	
	//return float4(depth,depth,depth,1);
	return float4(ao,ao,ao,1);
}

float4 HBlur( VSOUT IN ) : COLOR0 {
	float color = tex2D(passSampler, IN.UVCoord).r;

	float blurred = color*0.2270270270;
	blurred += tex2D(passSampler, IN.UVCoord + float2(rcpres.x*1.3846153846, 0)).r * 0.3162162162;
	blurred += tex2D(passSampler, IN.UVCoord - float2(rcpres.x*1.3846153846, 0)).r * 0.3162162162;
	blurred += tex2D(passSampler, IN.UVCoord + float2(rcpres.x*3.2307692308, 0)).r * 0.0702702703;
	blurred += tex2D(passSampler, IN.UVCoord - float2(rcpres.x*3.2307692308, 0)).r * 0.0702702703;
	
	return blurred;
}

float4 VBlur( VSOUT IN ) : COLOR0 {
	float color = tex2D(passSampler, IN.UVCoord).r;

	float blurred = color*0.2270270270;
	blurred += tex2D(passSampler, IN.UVCoord + float2(0, rcpres.y*1.3846153846)).r * 0.3162162162;
	blurred += tex2D(passSampler, IN.UVCoord - float2(0, rcpres.y*1.3846153846)).r * 0.3162162162;
	blurred += tex2D(passSampler, IN.UVCoord + float2(0, rcpres.y*3.2307692308)).r * 0.0702702703;
	blurred += tex2D(passSampler, IN.UVCoord - float2(0, rcpres.y*3.2307692308)).r * 0.0702702703;
	
	return blurred;
}

float4 Combine( VSOUT IN ) : COLOR0 {
	float3 color = tex2D(frameSampler, IN.UVCoord).rgb;
	float ao = tex2D(passSampler, IN.UVCoord/scale).r;
	ao = clamp(ao, aoClamp, 1.0);

	#ifdef LUMINANCE_CONSIDERATION
	float luminance = color.r*0.3+color.g*0.59+color.b*0.11;
	float white = 1.0;
	float black = 0;

	luminance = clamp(max(black,luminance-luminosity_threshold)+max(black,luminance-luminosity_threshold)+max(black,luminance-luminosity_threshold),0.0,1.0);
	ao = lerp(ao,white,luminance);
	#endif

	color *= ao;

	//return float4(ao, ao, ao,1);
	return float4(color,1);
}

technique t0 
{
	pass p0
	{
		VertexShader = compile vs_3_0 FrameVS();
		PixelShader = compile ps_3_0 ssao_Main();
	}
	pass p1
	{
		VertexShader = compile vs_3_0 FrameVS();
		PixelShader = compile ps_3_0 HBlur();
	}
	pass p2
	{
		VertexShader = compile vs_3_0 FrameVS();
		PixelShader = compile ps_3_0 VBlur();
	}
	pass p3
	{
		VertexShader = compile vs_1_1 FrameVS();
		PixelShader = compile ps_2_0 Combine();
	}
}
