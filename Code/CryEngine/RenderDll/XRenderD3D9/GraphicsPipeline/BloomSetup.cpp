#include "BloomSetup.h"


void CBloomSetupStage::Init()
{
	
}

void CBloomSetupStage::Execute(CTexture* pSrcRT, CTexture* pAutoExposureDestRT, CTexture* pTiledBloomDestRT)
{
	FUNCTION_PROFILER_RENDERER();
	PROFILE_LABEL_SCOPE("BLOOM_SETUP");

	m_passBloomSetup.SetTechnique(CShaderMan::s_shBloomSetup, CCryNameTSCRC("BloomSetup"), 0);
	m_passBloomSetup.SetOutputUAV(0, pAutoExposureDestRT);
	m_passBloomSetup.SetTextureSamplerPair(0, pSrcRT, EDefaultSamplerStates::LinearClamp);

	int width = 0; 
	int height = 0; 
	
	if (pAutoExposureDestRT)
	{
		width = pAutoExposureDestRT->GetWidth();
		height = pAutoExposureDestRT->GetHeight();
	}
	else if (pTiledBloomDestRT)
	{
		width = pTiledBloomDestRT->GetWidth();
		height = pTiledBloomDestRT->GetHeight();
	}
	else
	{
		return;
	}

	//only execute this function with flag : eFlags_ReflectConstantBuffersFromShader
	//m_passBloomSetup.BeginConstantUpdate();

	Vec4 screenSize(static_cast<float>(width), static_cast<float>(height), 1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height));
	m_parameters->TexSize = screenSize;
	m_passBloomSetup.SetConstantBuffer(0, m_parameters.GetDeviceConstantBuffer());
	
	const uint32 groupSizeX = 16;
	const uint32 groupSizeY = 16;

	const uint32 dispatchSizeX = (width / groupSizeX) + (width % groupSizeX > 0 ? 1 : 0);
	const uint32 dispatchSizeY = (height / groupSizeY) + (height % groupSizeY > 0 ? 1 : 0);

	m_passBloomSetup.SetDispatchSize(dispatchSizeX, dispatchSizeY, 1);
	m_passBloomSetup.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

	{
		const bool bAsynchronousCompute = CRenderer::CV_r_D3D12AsynchronousCompute & BIT((eStage_BloomSetup - eStage_FIRST_ASYNC_COMPUTE)) ? true : false;
		SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
		m_passBloomSetup.Execute(computeCommandList);
	}
	
}
