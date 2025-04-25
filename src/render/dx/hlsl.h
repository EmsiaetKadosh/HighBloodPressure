//
// Created by EmsiaetKadosh on 25-4-8.
//

#pragma once

#define SHADER_SOURCE_CODE(...) #__VA_ARGS__

inline const char* VertexShader = SHADER_SOURCE_CODE(
	// 输入结构：从顶点缓冲读取的数据
	struct VSInput {
		float3 position : POSITION; // 顶点位置（语义：POSITION）
		float2 uv : TEXCOORD; // UV坐标（语义：TEXCOORD）
	};

	// 输出结构：传递给像素着色器的数据
	struct PSInput {
		float4 position : SV_POSITION; // 裁剪空间坐标（必须）
		float2 uv : TEXCOORD; // 传递UV坐标
	};

	// 顶点着色器主函数
	PSInput VSMain(VSInput input) {
		PSInput output;
		output.position = float4(input.position, 1.0); // 转换为齐次坐标
		output.uv = input.uv; // 直接传递UV
		return output;
	}
);

inline const char* PixelShader = SHADER_SOURCE_CODE(
	Texture2D g_texture : register(t0);
	SamplerState g_sampler : register(s0);

	float4 PSMain(PSInput input) : SV_TARGET {
		return g_texture.Sample(g_sampler, input.uv);
	}
);

#undef SHADER_SOURCE_CODE
