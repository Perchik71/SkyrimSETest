#include "../../../../common.h"
#include "../BSVertexShader.h"
#include "../BSPixelShader.h"
#include "../BSShader.h"
#include "../../BSGraphicsRenderer.h"
#include "BSDistantTreeShader.h"
#include "../../NiMain/NiColor.h"
#include "../../NiMain/NiTransform.h"
#include "../BSShaderUtil.h"
#include "../BSShaderManager.h"
#include "../../NiMain/BSGeometry.h"
#include "../../NiMain/NiSourceTexture.h"

//
// Shader notes:
//
// - Destructor is not implemented
// - VS Parameter 0 is never set (InstanceData)
// - VS Parameter 7 is never set (DiffuseDir)
// - VS Parameter 8 is never set (IndexScale)
// - A global variable update was manually removed in SetupGeometry()
// - SetupTechnique() has an unknown pixel shader parameter 7 (TODO)
//
using namespace DirectX;

AutoPtr(bool, bUseEarlyZ, 0x30528E5);
AutoPtr(BYTE, byte_141E32FE0, 0x1E32FE0);
AutoPtr(float, dword_141E32FBC, 0x1E32FBC);
AutoPtr(__int64, qword_141E32F20, 0x1E32F20);
AutoPtr(NiSourceTexture *, qword_1432A7F58, 0x32A7F58);

void TestHook2()
{
	Detours::X64::DetourFunctionClass((PBYTE)(g_ModuleBase + 0x1318050), &BSDistantTreeShader::__ctor__);
}

BSDistantTreeShader::BSDistantTreeShader() : BSShader("DistantTree")
{
	m_Type = 9;
	pInstance = this;
}

BSDistantTreeShader::~BSDistantTreeShader()
{
	__debugbreak();
}

bool BSDistantTreeShader::SetupTechnique(uint32_t Technique)
{
	BSSHADER_FORWARD_CALL(TECHNIQUE, &BSDistantTreeShader::SetupTechnique, Technique);

	// Check if shaders exist
	uint32_t rawTechnique = GetRawTechnique(Technique);
	uint32_t vertexShaderTechnique = GetVertexTechnique(rawTechnique);
	uint32_t pixelShaderTechnique = GetPixelTechnique(rawTechnique);

	if (!BeginTechnique(vertexShaderTechnique, pixelShaderTechnique, false))
		return false;

	auto *renderer = BSGraphics::Renderer::GetGlobals();
	auto vertexCG = renderer->GetShaderConstantGroup(renderer->m_CurrentVertexShader, BSGraphics::CONSTANT_GROUP_LEVEL_TECHNIQUE);
	auto pixelCG = renderer->GetShaderConstantGroup(renderer->m_CurrentPixelShader, BSGraphics::CONSTANT_GROUP_LEVEL_TECHNIQUE);

	// fogParams is of type NiFogProperty *
	auto sub_1412AC860 = (uintptr_t(__fastcall *)(BYTE))(g_ModuleBase + 0x12AC860);
	uintptr_t fogParams = sub_1412AC860(byte_141E32FE0);

	if (fogParams)
	{
		XMVECTOR& vs_fogParam = vertexCG.ParamVS<XMVECTOR, 4>();	// VS: p4 float4 FogParam
		XMVECTOR& vs_fogNearColor = vertexCG.ParamVS<XMVECTOR, 5>();// VS: p5 float4 FogNearColor
		XMVECTOR& vs_fogFarColor = vertexCG.ParamVS<XMVECTOR, 6>();	// VS: p6 float4 FogFarColor

		float v19 = *(float *)(fogParams + 84);
		float v20 = *(float *)(fogParams + 80);
		float v21 = *(float *)(fogParams + 132);
		float v22 = *(float *)(fogParams + 136);

		if (v19 != 0.0f || v20 != 0.0f)
		{
			BSGraphics::Utility::CopyNiColorAToFloat(&vs_fogParam,
				NiColorA((1.0f / (v19 - v20)) * v20, 1.0f / (v19 - v20), v21, v22));

			// NiFogProperty::GetFogNearColor(v6);
			BSGraphics::Utility::CopyNiColorAToFloat(&vs_fogNearColor,
				NiColorA(*(float *)(fogParams + 56), *(float *)(fogParams + 60), *(float *)(fogParams + 64), dword_141E32FBC));

			// NiFogProperty::GetFogFarColor(v6);
			BSGraphics::Utility::CopyNiColorAToFloat(&vs_fogFarColor,
				NiColorA(*(float *)(fogParams + 68), *(float *)(fogParams + 72), *(float *)(fogParams + 76), 1.0f));
		}
		else
		{
			BSGraphics::Utility::CopyNiColorAToFloat(&vs_fogParam, NiColorA(500000.0f, 0.0f, v21, 0.0f));
			BSGraphics::Utility::CopyNiColorAToFloat(&vs_fogNearColor, NiColorA::BLACK);
			BSGraphics::Utility::CopyNiColorAToFloat(&vs_fogFarColor, NiColorA::BLACK);
		}
	}

	// lightSource is of type NiDirectionalLight *
	uintptr_t lightSource = *(uintptr_t *)(*(uintptr_t *)(qword_141E32F20 + 512) + 72i64);

	if (lightSource)
	{
		XMVECTOR& ps_DiffuseColor = pixelCG.ParamPS<XMVECTOR, 0>();	// PS: p0 float4 DiffuseColor
		XMVECTOR& ps_AmbientColor = pixelCG.ParamPS<XMVECTOR, 1>();	// PS: p1 float4 AmbientColor
		XMVECTOR& ps_Unknown = pixelCG.ParamPS<XMVECTOR, 7>();		// TODO: WHAT IS THIS PARAM??? It should be for vertex instead of pixel?

		// NiPoint3 normalizedDir = NiDirectionalLight::GetWorldDirection().Unitize();
		NiPoint3 normalizedDir(*(const NiPoint3 *)(lightSource + 320));
		normalizedDir.Unitize();

		// NiLight::GetAmbientColor(v21);
		BSGraphics::Utility::CopyNiColorAToFloat(&ps_AmbientColor,
			NiColorA(*(float *)(lightSource + 272), *(float *)(lightSource + 276), *(float *)(lightSource + 280), dword_141E32FBC));

		// NiLight::GetDiffuseColor(v21);
		BSGraphics::Utility::CopyNiColorAToFloat(&ps_DiffuseColor,
			NiColorA(*(float *)(lightSource + 284), *(float *)(lightSource + 288), *(float *)(lightSource + 292), 1.0f));

		// I think they are calling abs(float) on these, but compiler optimized it to an XOR? Verified correct assembly.
		BSGraphics::Utility::CopyNiColorAToFloat(&ps_Unknown,
			NiColorA(-normalizedDir.x, -normalizedDir.y, -normalizedDir.z, 1.0f));
	}

	// if (!qword_1432A7F58)
	// bAssert("LOD tree texture atlas for current worldspace not found.");

	renderer->SetTexture(0, qword_1432A7F58->QRendererTexture());// Diffuse
	renderer->SetTextureAddressMode(0, 0);

	renderer->FlushConstantGroupVSPS(&vertexCG, &pixelCG);
	renderer->ApplyConstantGroupVSPS(&vertexCG, &pixelCG, BSGraphics::CONSTANT_GROUP_LEVEL_TECHNIQUE);
	return true;
}

void BSDistantTreeShader::RestoreTechnique(uint32_t Technique)
{
	BSSHADER_FORWARD_CALL(TECHNIQUE, &BSDistantTreeShader::RestoreTechnique, Technique);
}

void BSDistantTreeShader::SetupGeometry(BSRenderPass *Pass, uint32_t RenderFlags)
{
	BSSHADER_FORWARD_CALL(GEOMETRY, &BSDistantTreeShader::SetupGeometry, Pass, RenderFlags);

	auto *renderer = BSGraphics::Renderer::GetGlobals();
	auto vertexCG = renderer->GetShaderConstantGroup(renderer->m_CurrentVertexShader, BSGraphics::CONSTANT_GROUP_LEVEL_GEOMETRY);

	// Standard world transformation matrix
	XMMATRIX geoTransform = BSShaderUtil::GetXMFromNi(Pass->m_Geometry->GetWorldTransform());

	//
	// GetXMFromNiPosAdjust is a custom function to remove the original global variable
	// references...I'm copying what FO4 did and passing in another local argument.
	//
	// The original code copies a global Point3 (and restores it afterwards):
	//		flt_14304E20C = flt_14304E218;
	//		flt_14304E210 = flt_14304E21C;
	//		flt_14304E214 = flt_14304E220;
	//
	XMMATRIX prevGeoTransform = BSShaderUtil::GetXMFromNiPosAdjust(Pass->m_Geometry->GetWorldTransform(), *(NiPoint3 *)&renderer->__zz2[40]);

	//
	// VS: p1 float4x4 WorldViewProj
	// VS: p2 float4x4 World
	// VS: p3 float4x4 PreviousWorld
	//
	vertexCG.ParamVS<XMMATRIX, 1>() = XMMatrixMultiplyTranspose(geoTransform, *(XMMATRIX *)&renderer->__zz2[240]);
	vertexCG.ParamVS<XMMATRIX, 2>() = XMMatrixTranspose(geoTransform);
	vertexCG.ParamVS<XMMATRIX, 3>() = XMMatrixTranspose(prevGeoTransform);

	renderer->FlushConstantGroupVSPS(&vertexCG, nullptr);
	renderer->ApplyConstantGroupVSPS(&vertexCG, nullptr, BSGraphics::CONSTANT_GROUP_LEVEL_GEOMETRY);
}

void BSDistantTreeShader::RestoreGeometry(BSRenderPass *Pass, uint32_t RenderFlags)
{
	BSSHADER_FORWARD_CALL(GEOMETRY, &BSDistantTreeShader::RestoreGeometry, Pass, RenderFlags);
}

uint32_t BSDistantTreeShader::GetRawTechnique(uint32_t Technique)
{
	uint32_t outputTech = 0;

	if (Technique == BSSM_DISTANTTREE)
		outputTech = RAW_TECHNIQUE_BLOCK;
	else if (Technique == BSSM_DISTANTTREE_DEPTH)
		outputTech = RAW_TECHNIQUE_DEPTH;
	//else
	// bAssert("BSDistantTreeShader: bad technique ID");

	if (bUseEarlyZ)
		outputTech |= RAW_FLAG_DO_ALPHA;
	
	return outputTech;
}

uint32_t BSDistantTreeShader::GetVertexTechnique(uint32_t RawTechnique)
{
	return RawTechnique;
}

uint32_t BSDistantTreeShader::GetPixelTechnique(uint32_t RawTechnique)
{
	return RawTechnique;
}