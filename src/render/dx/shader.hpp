
#pragma once

/**
 * 着色器程序字符串。
 * vColor 纯色顶点着色器
 * pColor 纯色像素着色器
 * vTexture 纹理顶点着色器
 * pTexture 纹理像素着色器
 */

#define HBP_SHADER_TEXTURE (HBP_SHADER_GLOBAL HBP_SHADER_TEXTURE_DEFINE HBP_SHADER_TEXTURE_ENTRY)
#define HBP_SHADER_COLORED (HBP_SHADER_GLOBAL HBP_SHADER_COLORED_DEFINE HBP_SHADER_COLORED_ENTRY)

#define HBP_SHADER_GLOBAL R"shader(
cbuffer ObjectConstants : register(b0) {
	float4x4 g_matrix;
};
)shader"

#define HBP_SHADER_TEXTURE_DEFINE R"shader(
struct TextureVertexInput {
	float3 position : POSITION; // 12
	uint color : COLOR;         // 4  ~16
	float2 uv : TEXCOORD;       // 8  ~24
	min16int4 normal : NORMAL;       // 64 ~88
};

struct TextureVSOutput {
	float4 position : SV_POSITION; // 16
	uint color : COLOR;            // 4  ~20
	float2 uv : TEXCOORD;          // 8  ~28
	float3 normal : NORMAL;        // 12 ~40
	uint index : INDEX;            // 4  ~44
};

Texture2DArray constantTexture : register(t0);
Texture2DArray preloadTexture : register(t1);
Texture2DArray appendTexture : register(t2);
Texture2DArray tempTexture : register(t3);
SamplerState textureSampler : register(s0);
)shader"

#define HBP_SHADER_TEXTURE_ENTRY R"shader(
TextureVSOutput vTexture(TextureVertexInput input) {
	TextureVSOutput output;
	output.position = mul(g_matrix, float4(input.position, 1.0f));
	output.color = input.color;
	output.uv = input.uv;
	output.normal = normalize(float3(input.normal.xyz) / 32767.0);
	output.index = (uint) input.normal.w;
	return output;
}

float4 pTexture(TextureVSOutput input) : SV_TARGET {
	float4 vertexColor = float4(
		float(0xff & (input.color >> 16)) / 255.0,
		float(0xff & (input.color >> 8 )) / 255.0,
		float(0xff & (input.color      )) / 255.0,
		float(0xff & (input.color >> 24)) / 255.0
	);
	uint mask = input.index >> 14;
	uint idx = input.index & 0x3fff;
	float4 textureColor = 0;
	switch (mask) {
		case 1: textureColor = constantTexture.Sample(textureSampler, float3(input.uv, idx)); break;
		case 2: textureColor = preloadTexture.Sample(textureSampler, float3(input.uv, idx)); break;
		case 3: textureColor = appendTexture.Sample(textureSampler, float3(input.uv, idx)); break;
		case 4: textureColor = tempTexture.Sample(textureSampler, float3(input.uv, idx)); break;
		default: break;
	}
	return textureColor * vertexColor;
}

)shader"

#define HBP_SHADER_COLORED_DEFINE R"shader(
struct ColoredVertexInput {
	float3 position : POSITION;
	uint color : COLOR;
};

struct ColoredVSOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
)shader"

#define HBP_SHADER_COLORED_ENTRY R"shader(
ColoredVSOutput vColored(ColoredVertexInput input) {
	ColoredVSOutput output;
	output.position = mul(g_matrix, float4(input.position, 1.0f));
	output.color = float4(
		float((0x00ff0000 & input.color) >> 16) / 255.0,
		float((0x0000ff00 & input.color) >> 8 ) / 255.0,
		float((0x000000ff & input.color)      ) / 255.0,
		float((0xff000000 & input.color) >> 24) / 255.0
	);
	return output;
}

float4 pColored(ColoredVSOutput input) : SV_TARGET {
	return input.color;
}
)shader"
