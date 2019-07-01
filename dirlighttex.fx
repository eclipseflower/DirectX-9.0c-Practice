struct Mtrl
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float specPower;
};

struct DirLight
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float3 dirW;
};

uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans;
uniform extern float4x4 gWVP;

uniform extern Mtrl gMtrl;
uniform extern DirLight gLight;
uniform extern float3 gEyePosW;
uniform extern texture gTex;

static float3 gFogColor = { 0.5f, 0.5f, 0.5f };
static float gFogStart = 1.0f;
static float gFogRange = 250.0f;

sampler TexS = sampler_state
{
	Texture = <gTex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct OutputVS
{
	float4 posH : POSITION0;
	float4 diffuse : COLOR0;
	float4 spec : COLOR1;
	float2 tex0 : TEXCOORD0;
	float fogLerpParam : TEXCOORD1;
};

OutputVS DirLightTexVS(float3 posL : POSITION0, float3 normalL : NORMAL0, float2 tex0 : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	float3 normalW = mul(float4(normalL, 0.0f), gWorldInvTrans).xyz;
	normalW = normalize(normalW);

	float3 posW = mul(float4(posL, 1.0f), gWorld).xyz;

	float3 lightVecW = -gLight.dirW;
	float3 toEyeW = normalize(gEyePosW - posW);
	float3 r = reflect(-lightVecW, normalW);
	float t = pow(max(dot(r, toEyeW), 0.0f), gMtrl.specPower);
	float s = max(dot(lightVecW, normalW), 0.0f);

	float3 spec = t * (gMtrl.spec * gLight.spec).rgb;
	float3 diffuse = s * (gMtrl.diffuse * gLight.diffuse).rgb;
	float3 ambient = (gMtrl.ambient * gLight.ambient).rgb;

	outVS.diffuse.rgb = ambient + diffuse;
	outVS.diffuse.a = gMtrl.diffuse.a;
	outVS.spec = float4(spec, 1.0f);

	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	outVS.tex0 = tex0;
	float dist = distance(posW, gEyePosW);
	outVS.fogLerpParam = saturate((dist - gFogStart) / gFogRange);

	return outVS;
}

float4 DirLightTexPS(float4 c : COLOR0, float4 spec : COLOR1, float2 tex0 : TEXCOORD0, float fogLerpParam : TEXCOORD1) : COLOR
{
	float4 texColor = tex2D(TexS, tex0);
	float3 diffuse = c.rgb * texColor.rgb;

	float3 final = lerp(diffuse + spec.rgb, gFogColor, fogLerpParam);
	return float4(final, texColor.a * c.a);
}

technique DirLightTexTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 DirLightTexVS();
		pixelShader = compile ps_2_0 DirLightTexPS();
	}
}