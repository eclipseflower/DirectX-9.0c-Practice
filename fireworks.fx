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

OutputVS FireWorksVS(float3 posL : POSITION0,
	float3 vel : TEXCOORD0,
	float size : TEXCOORD1,
	float time : TEXCOORD2,
	float lifeTime : TEXCOORD3,
	float mass : TEXCOORD4,
	float4 color : COLOR0)
{
	OutputVS outVS = (OutputVS)0;
	float t = gTime - time;
	float s = sin(6.0f * t);
	posL.y += mass * s;
	posL = posL + vel * t + 0.5f * mass * gAccel * t * t;

	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	outVS.size = gViewportHeight * size / 600.0f;
	outVS.color = color;

	return outVS;
}

float4 FireWorksPS(float4 color : COLOR0, float2 tex0 : TEXCOORD0) : COLOR
{
	return float4(color.rgb * tex2D(TexS, tex0).rgb, 1.0f);
}

technique FireWorksTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 FireWorksVS();
		pixelShader = compile ps_2_0 FireWorksPS();

		PointSpriteEnable = true;
		AlphaBlendEnable = true;
		SrcBlend = One;
		DestBlend = One;

		ZWriteEnable = false;
	}
}