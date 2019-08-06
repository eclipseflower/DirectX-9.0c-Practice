uniform extern float4x4 gWVP;
uniform extern texture gTex;
uniform extern float3 gEyePosL;
uniform extern float3 gAccel;
uniform extern float gTime;
uniform extern int gViewportHeight;

sampler TexS = sampler_state
{
	Texture = <gTex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct OutputVS
{
	float4 posH : POSITION0;
	float4 color : COLOR0;
	float2 tex0 : TEXCOORD0;
	float size : PSIZE;
};

OutputVS FireRingVS(float3 posL : POSITION0,
	float3 vel : TEXCOORD0,
	float size : TEXCOORD1,
	float time : TEXCOORD2,
	float lifeTime : TEXCOORD3,
	float mass : TEXCOORD4,
	float4 color : COLOR0)
{
	OutputVS outVS = (OutputVS)0;
	float t = gTime - time;
	float sine, cosine;
	sincos(0.5 * mass * t, sine, cosine);
	float x = posL.x * cosine + posL.y * -sine;
	float y = posL.x * sine + posL.y * cosine;

	float s = sin(6.0 * t);
	posL.x = x;
	posL.y = y + mass * s;

	posL = posL + vel * t + 0.5 * gAccel * t * t;

	outVS.posH = mul(float4(posL, 1.0), gWVP);
	size += 8 * t * t;

	float d = distance(posL, gEyePosL);
	outVS.size = gViewportHeight * size / (1.0 + 8.0 * d);

	outVS.color = (1.0 - t / lifeTime);

	return outVS;
}

float4 FireRingPS(float4 color : COLOR0, float2 tex0 : TEXCOORD0) : COLOR
{
	return color * tex2D(TexS, tex0);
}

technique FireRingTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 FireRingVS();
		pixelShader = compile ps_2_0 FireRingPS();

		PointSpriteEnable = true;
		AlphaBlendEnable = true;
		SrcBlend = One;
		DestBlend = One;

		ZWriteEnable = false;
	}
}