#include "Vertex.h"
#include "D3DUtil.h"

IDirect3DVertexDeclaration9 *VertexPos::decl = NULL;
IDirect3DVertexDeclaration9 *VertexCol::decl = NULL;
IDirect3DVertexDeclaration9 *VertexPN::decl = NULL;
IDirect3DVertexDeclaration9 *VertexPNT::decl = NULL;

void InitAllVertexDeclarations()
{
	D3DVERTEXELEMENT9 vertexPosElements[] = 
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};
	HR(gD3dDevice->CreateVertexDeclaration(vertexPosElements, &VertexPos::decl));

	D3DVERTEXELEMENT9 vertexColElements[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};
	HR(gD3dDevice->CreateVertexDeclaration(vertexColElements, &VertexCol::decl));

	D3DVERTEXELEMENT9 vertexPNElements[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		D3DDECL_END()
	};
	HR(gD3dDevice->CreateVertexDeclaration(vertexPNElements, &VertexPN::decl));

	D3DVERTEXELEMENT9 vertexPNTElements[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	HR(gD3dDevice->CreateVertexDeclaration(vertexPNTElements, &VertexPNT::decl));
}

void DestroyAllVertexDeclarations()
{
	ReleaseCOM(VertexPos::decl);
	ReleaseCOM(VertexCol::decl);
	ReleaseCOM(VertexPN::decl);
	ReleaseCOM(VertexPNT::decl);
}
