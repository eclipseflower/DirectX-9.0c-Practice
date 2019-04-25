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

sampler TexS = sampler_state
{
	Texture = <gTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct OutputVS
{
	float4 posH : POSITION0;
	float3 normalW : TEXCOORD0;
	float3 toEyeW : TEXCOORD1;
	float2 tex0 : TEXCOORD2;
};

OutputVS PhongDirLtTexVS(float3 posL : POSITION0, float3 normalL : NORMAL0, float2 tex0 : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;
	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInvTrans).xyz;
	float3 posW = mul(float4(posL, 1.0f), gWorld).xyz;
	outVS.toEyeW = gEyePosW - posW;
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	outVS.tex0 = tex0;

	return outVS;
}

float4 PhongDirLtTexPS(float3 normalW : TEXCOORD0, float3 toEyeW : TEXCOORD1, float2 tex0 : TEXCOORD2) : COLOR
{
	normalW = normalize(normalW);
	toEyeW = normalize(toEyeW);

	float3 lightVecW = -gLight.dirW;
	float3 r = reflect(-lightVecW, normalW);

	float t = pow(max(dot(r, toEyeW), 0.0f), gMtrl.specPower);
	float s = max(dot(lightVecW, normalW), 0.0f);

	float3 spec = t * (gMtrl.spec * gLight.spec).rgb;
	float3 diffuse = s * (gMtrl.diffuse * gLight.diffuse).rgb;
	float3 ambient = gMtrl.ambient * gLight.ambient;

	float4 texColor = tex2D(TexS, tex0);
	float3 color = (ambient + diffuse) * texColor.rgb + spec;

	return float4(color, gMtrl.diffuse.a * texColor.a);
}

technique PhongDirLtTexTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 PhongDirLtTexVS();
		pixelShader = compile ps_2_0 PhongDirLtTexPS();
	}
}