uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInverseTranspose;
uniform extern float4x4 gWVP;

uniform extern float4 gAmbientMtrl;
uniform extern float4 gAmbientLight;
uniform extern float4 gDiffuseMtrl;
uniform extern float4 gDiffuseLight;
uniform extern float4 gSpecularMtrl;
uniform extern float4 gSpecularLight;
uniform extern float gSpecularPower;
uniform extern float3 gLightVecW;
uniform extern float3 gEyePosW;

struct OutputVS
{
	float4 posH : POSITION0;
	float4 color : COLOR0;
};

OutputVS AmbientDiffuseSpecVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	OutputVS outVS = (OutputVS)0;
	float3 normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	normalW = normalize(normalW);
	float3 posW = mul(float4(posL, 1.0f), gWorld).xyz;

	float3 toEye = normalize(gEyePosW - posW);
	float3 r = reflect(-gLightVecW, normalW);
	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	float s = max(dot(gLightVecW, normalW), 0.0f);

	float3 spec = t * (gSpecularMtrl * gSpecularLight).rgb;
	float3 diffuse = s * (gDiffuseMtrl * gDiffuseLight).rgb;
	float3 ambient = (gAmbientMtrl * gAmbientLight).rgb;

	outVS.color.rgb = ambient + diffuse + spec;
	outVS.color.a = gDiffuseMtrl.a;
	outVS.posH = mul(float4(posL, 1.0f), gWVP);

	return outVS;
}

float4 AmbientDiffuseSpecPS(float4 c : COLOR) : COLOR
{
	return c;
}

technique AmbientDiffuseSpecTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 AmbientDiffuseSpecVS();
		pixelShader = compile ps_2_0 AmbientDiffuseSpecPS();
	}
}