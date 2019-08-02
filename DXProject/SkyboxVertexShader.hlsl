#include "SkyboxHeader.hlsli"

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_SKYBOX_OUTPUT main(float3 Pos : POSITION)
{
	VS_SKYBOX_OUTPUT output = (VS_SKYBOX_OUTPUT)0; // Create a new vertex to store transformed vertex + colour

	output.Pos = mul(float4(Pos, 1.0f), World); // Transform vertex with respect to mesh world co-ords.
	output.Pos = mul(output.Pos, View); // Now transform vertex with respect to the view
	output.Pos = mul(output.Pos, Projection); // Now transform with respect to field of view

	output.texCoord = Pos.xyz; // texture co-ordinate for skybox is x,y,z to locate
							   // the correct texture from the cubemap.  Kind of like a 3D texture

	return output;
}

