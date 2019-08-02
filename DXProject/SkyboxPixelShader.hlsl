#include "SkyboxHeader.hlsli"

// Cube map variable
TextureCube cubeMap : register(t1);
// Sampler.  Use the regular sampler
SamplerState colourSampler : register(s0);

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(VS_SKYBOX_OUTPUT input) : SV_Target
{
	return cubeMap.Sample(colourSampler, input.texCoord);
}

