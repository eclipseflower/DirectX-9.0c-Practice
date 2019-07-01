uniform extern float4x4 gWorld;
uniform extern float4x4 gWVP;
uniform extern float3 gEyePosW;

static float3 gFogColor = { 0.5f, 0.5f, 0.5f };
static float gFogStart = 1.0f;
static float gFogRange = 250.0f;

struct OutputVS
{
	float4 posH : POSITION0;
	float fogLerpParam : TEXCOORD0;
};

OutputVS WaterVS(float3 posL : POSITION0)
{
	OutputVS outVS = (OutputVS)0;
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	float3 posW = mul(float4(posL, 1.0f), gWorld);

	float dist = distance(posW, gEyePosW);
	outVS.fogLerpParam = saturate((dist - gFogStart) / gFogRange);

	return outVS;
}

float4 WaterPS(float fogLerpParam : TEXCOORD0) : COLOR
{
	float3 color = float3(0.0f, 0.2f, 0.4f);
	float3 final = lerp(color, gFogColor, fogLerpParam);

	return float4(final, 0.5f);
}

technique WaterTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 WaterVS();
		pixelShader = compile ps_2_0 WaterPS();

		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
	}
}