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
	AddressV = ClAMP;
};

struct OutputVS
{
	float4 posH : POSITION0;
	float2 tex0 : TEXCOORD0;
	float size : PSIZE;
};

OutputVS GunVS(float3 posL : POSITION0,
	float3 vel : TEXCOORD0,
	float size : TEXCOORD1,
	float time : TEXCOORD2,
	float lifeTime : TEXCOORD3,
	float mass : TEXCOORD4,
	float4 color : COLOR0)
{
	OutputVS outVS = (OutputVS)0;
	float t = gTime - time;
	posL = posL + vel * t + 0.5 * gAccel * t * t;

	outVS.posH = mul(float4(posL, 1.0), gWVP);

	float d = distance(posL, gEyePosL);
	outVS.size = gViewportHeight * size / (1.0 + 8.0 * d);

	return outVS;
}

float4 GunPS(float2 tex0 : TEXCOORD0) : COLOR
{
	return tex2D(TexS, tex0);
}

technique GunTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 GunVS();
		pixelShader = compile ps_2_0 GunPS();

		PointSpriteEnable = true;
		AlphaBlendEnable = true;
		SrcBlend = One;
		DestBlend = One;
		ZWriteEnable = false;
	}
}