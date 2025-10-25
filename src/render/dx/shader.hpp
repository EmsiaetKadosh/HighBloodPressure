
#pragma once

/**
 * 着色器程序字符串。
 * vColor 纯色顶点着色器
 * pColor 纯色像素着色器
 * vTexture 纹理顶点着色器
 * pTexture 纹理像素着色器
 */
inline const char* Shader = R"shader(
cbuffer ObjectConstants : register(b0) {
	float4x4 g_matrix;
	uint cbSize;
	uint pbSize;
	uint abSize;
	uint tbSize;
};

struct TextureVertexInput {
	float3 position : POSITION;
	uint color : COLOR;
	float2 uv : TEXCOORD;
	int4 normal : NORMAL;
};

struct TextureVSOutput {
	float4 position : SV_POSITION;
	uint color : COLOR;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	uint index : INDEX;
};

///////////////////////////
// Entry point: vTexture //
///////////////////////////
TextureVSOutput vTexture(TextureVertexInput input) {
	TextureVSOutput output;
	output.position = mul(g_matrix, float4(input.position, 1.0f));
	output.color = input.color;
	output.uv = input.uv;
	output.normal = normalize(float3(input.normal.xyz) / 32767.0);
	output.index = (uint) input.normal.w;
	return output;
}

Texture2D constantTexture[] : register(t0);
Texture2D preloadTexture[] : register(t1);
Texture2D appendTexture[] : register(t2);
Texture2D tempTexture[] : register(t3);
SamplerState textureSampler : register(s0);

Texture2D SelectTexture(uint index) {
	uint mask = index >> 14;
	uint idx = index & 0x3fff;
	switch (mask) {
		case 1: return t0[index];
		case 2: return t1[index];
		case 3: return t2[index];
		case 4: return t3[index];
	}
}

///////////////////////////
// Entry point: pTexture //
///////////////////////////
float4 pTexture(TextureVSOutput input) : SV_TARGET {
	float4 vertexColor = float4(
		float(0x00ff0000 & input.color >> 16) / 255.0,
		float(0x0000ff00 & input.color >> 8 ) / 255.0,
		float(0x000000ff & input.color      ) / 255.0,
		float(0xff000000 & input.color >> 24) / 255.0
	);
	float4 textureColor = SelectTexture(input.index).Sample(textureSampler, input.uv);
	return textureColor * vertexColor;
}

struct ColoredVertexInput {
	float3 position : POSITION;
	uint color : COLOR;
};

struct ColoredVSOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

///////////////////////////
// Entry point: vColored //
///////////////////////////
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

///////////////////////////
// Entry point: pColored //
///////////////////////////
float4 pColored(ColoredVSOutput input) : SV_TARGET {
	return input.color;
}
)shader";
