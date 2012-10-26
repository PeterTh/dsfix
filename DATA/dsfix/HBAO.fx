// Cosine Weighted HBAO
// Originally based off of NVIDIA's HBAO, implemented and heavily modified by Tomerk
// Adapted and tweaked for Dark Souls by Durante

/***User-controlled variables***/
#define N_DIRECTIONS 8 //number of directions to sample in, currently do not change.
#define N_STEPS 6 //number of steps to raymarch, you may change The higher this is the higher the quality and the lower the performance.

extern float aoRadiusMultiplier = 5.0; //Linearly multiplies the radius of the AO Sampling
extern float Attenuation_Factor = 0.1; //Affects units in space the AO will extend to

extern float FOV = 85; //Field of View in Degrees

extern float luminosity_threshold = 0.3;

#ifndef SCALE
#define SCALE 1.0
#endif

#ifndef SSAO_STRENGTH_LOW
#ifndef SSAO_STRENGTH_MEDIUM
#ifndef SSAO_STRENGTH_HIGH
#define SSAO_STRENGTH_MEDIUM 1
#endif
#endif
#endif

#ifdef SSAO_STRENGTH_LOW
extern float aoClamp = 0.75;
extern float aoStrengthMultiplier = 2.0;
#endif

#ifdef SSAO_STRENGTH_MEDIUM
extern float aoClamp = 0.4;
extern float aoStrengthMultiplier = 4.5;
#endif

#ifdef SSAO_STRENGTH_HIGH
extern float aoClamp = 0.15;
extern float aoStrengthMultiplier = 6.0;
#endif


#define LUMINANCE_CONSIDERATION //comment this line to not take pixel brightness into account
//#define RAW_SSAO //uncomment this line to show the raw ssao

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
	VSOUT OUT = (VSOUT)0.0f;	// initialize to zero, avoid complaints.
	OUT.vertPos = IN.vertPos;
	OUT.UVCoord = IN.UVCoord;
	return OUT;
}

static float2 sample_offset[N_DIRECTIONS] =
{
//#if N_DIRECTIONS >= 9
	float2(1, 0),
	float2(0.7071f, 0.7071f),
	float2(0, 1),
	float2(-0.7071f, 0.7071f),
	float2(-1, 0),
	float2(-0.7071f, -0.7071f),
	float2(0, -1),
	float2(0.7071f, -0.7071f)
//#endif
};

float2 rand(in float2 uv) {
	float noiseX = (frac(sin(dot(uv, float2(12.9898,78.233)*2.0)) * 43758.5453));
	float noiseY = sqrt(1-noiseX*noiseX);
	return float2(noiseX,noiseY);
}

float readDepth(in float2 coord : TEXCOORD0) {
	float4 col = tex2D(depthSampler, coord);	
	float posZ = ((1.0-col.z) + (1.0-col.y)*256.0 + (1.0-col.x)*(257.0*256.0));
	return 1 - (posZ-nearZ)/farZ;
}

float3 getPosition(in float2 uv, in float eye_z) {
   uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
   float3 pos = float3(uv * g_InvFocalLen * eye_z, eye_z );
   return pos;
}

float4 ssao_Main( VSOUT IN ) : COLOR0 {
	clip(1/SCALE-IN.UVCoord.x);
	clip(1/SCALE-IN.UVCoord.y);	
	IN.UVCoord.xy *= SCALE;

	float depth = readDepth(IN.UVCoord);
	float3 pos = getPosition(IN.UVCoord, depth);
	float3 dx = ddx(pos);
	float3 dy = ddy(pos);
	float3 norm = normalize(cross(dx,dy));

	float sample_depth;
	float3 sample_pos;

	float ao = 0.0;
	float s = 0.0;
	
	float2 rand_vec = rand(IN.UVCoord);
	float2 sample_vec_divisor = g_InvFocalLen*depth*depthRange/(aoRadiusMultiplier*5000*rcpres);
	float2 sample_center = IN.UVCoord;
	
	for(int i = 0; i < N_DIRECTIONS; i++)
	{
		float theta = 0;
		float temp_theta = 0;

		float temp_ao = 0;
		float curr_ao = 0;
		
		float3 occlusion_vector = float3(0,0,0);

		float2 sample_vec = reflect(sample_offset[i], rand_vec);
		sample_vec /= sample_vec_divisor;
		float2 sample_coords = (sample_vec*float2(1,aspect))/N_STEPS;
		
		for(int k = 1; k <= N_STEPS; k++)
		{
			sample_depth = readDepth(sample_center + sample_coords*(k-0.5*(i%2)) );
			sample_pos = getPosition(sample_center + sample_coords*(k-0.5*(i%2)), sample_depth);
			occlusion_vector = sample_pos - pos;
			temp_theta = dot(norm, normalize(occlusion_vector));			

			if(temp_theta > theta)
			{
				theta = temp_theta;
				temp_ao = 1-sqrt(1 - theta*theta );
				float dfactor = clamp(depth-0.8, 0.0, 1.0)*2;
				ao += (1/ (1 + (Attenuation_Factor+dfactor) * pow(length(occlusion_vector)/aoRadiusMultiplier*depthRange,2)) )*(temp_ao-curr_ao);
				curr_ao = temp_ao;
			}
		}
		s += 1;
	}

	ao /= s;
	
	// adjust for close and far away
	if(depth>0.98) ao = lerp(ao, 0.0, (depth-0.98)*100.0);

	ao = 1.0-ao*aoStrengthMultiplier;

	return float4(clamp(ao,aoClamp,1),1,1,1);
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
	float ao = tex2D(passSampler, IN.UVCoord/SCALE).r;
	ao = clamp(ao, aoClamp, 1.0);

	#ifdef LUMINANCE_CONSIDERATION
	float luminance = color.r*0.3+color.g*0.59+color.b*0.11;
	float white = 1.0;
	float black = 0;

	luminance = clamp(max(black,luminance-luminosity_threshold)+max(black,luminance-luminosity_threshold)+max(black,luminance-luminosity_threshold),0.0,1.0);
	ao = lerp(ao,white,luminance);
	#endif
	
	color *= 1.05;
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
