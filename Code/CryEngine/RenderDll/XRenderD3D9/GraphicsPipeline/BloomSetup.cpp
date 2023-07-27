#include "BloomSetup.h"


CBloomSetupStage::~CBloomSetupStage()
{
	m_tileBloomMaskBuffer.Release();
	m_tileBloomMaskBuffer2.Release();

	m_tileBloomInfoGen_TileInfoBuffer.Release();

	m_tileBloomInfoGen_DispatchThreadCount.Release();
}


void CBloomSetupStage::InitBuffer()
{
	const uint32 BufferSizeX = (width / groupSizeX) + (width % groupSizeX > 0 ? 1 : 0);
	const uint32 BufferSizeY = (height / groupSizeY) + (height % groupSizeY > 0 ? 1 : 0);

	if (!m_tileBloomMaskBuffer.IsAvailable())
	{
		m_tileBloomMaskBuffer.Create(BufferSizeX * BufferSizeY, sizeof(uint32), DXGI_FORMAT_R32_UINT, CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_tileBloomMaskBuffer.SetDebugName("TileBloomMaskBuffer");
	}

	if (!m_tileBloomMaskBuffer2.IsAvailable())
	{
		m_tileBloomMaskBuffer2.Create(BufferSizeX * BufferSizeY / 2 / 2, sizeof(uint32), DXGI_FORMAT_R32_UINT, CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS, NULL);
		m_tileBloomMaskBuffer2.SetDebugName("BloomTileInfoGen_MaskOutputUAV");
	}

	if (!m_tileBloomInfoGen_TileInfoBuffer.IsAvailable())
	{
		m_tileBloomInfoGen_TileInfoBuffer.Create(BufferSizeX * BufferSizeY, sizeof(uint32), DXGI_FORMAT_UNKNOWN/*DXGI_FORMAT_R32_UINT*/,
			CDeviceObjectFactory::USAGE_STRUCTURED | CDeviceObjectFactory::BIND_UNORDERED_ACCESS,
			NULL);
		m_tileBloomInfoGen_TileInfoBuffer.SetDebugName("m_tileBloomInfoGen_TileInfoBuffer");
	}


	//TODO:FixMe
	if (!m_tileBloomInfoGen_DispatchThreadCount.IsAvailable())
	{
		//D3D11 ERROR : ID3D11Device::CreateBuffer : A resource cannot created with both D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGSand D3D11_RESOURCE_MISC_BUFFER_STRUCTURED.[STATE_CREATION ERROR #68: CREATEBUFFER_INVALIDMISCFLAGS]
		//D3D11 ERROR: ID3D11Device::CreateUnorderedAccessView: The Format (0, UNKNOWN) cannot be used when creating a typed View of a Buffer. [ STATE_CREATION ERROR #2097343: CREATEUNORDEREDACCESSVIEW_INVALIDFORMAT]

		m_tileBloomInfoGen_DispatchThreadCount.Create(3, sizeof(uint32), DXGI_FORMAT_R32_UINT,
			CDeviceObjectFactory::USAGE_STRUCTURED | CDeviceObjectFactory::BIND_UNORDERED_ACCESS | CDeviceObjectFactory::USAGE_INDIRECTARGS,//USAGE_RAW
			//CDeviceObjectFactory::BIND_UNORDERED_ACCESS | CDeviceObjectFactory::USAGE_INDIRECTARGS,//USAGE_RAW
			NULL);
		m_tileBloomInfoGen_DispatchThreadCount.SetDebugName("m_tileBloomInfoGen_DispatchThreadCount");
	}
}

void CBloomSetupStage::Resize(int renderWidth, int renderHeight)
{
	if (m_tileBloomMaskBuffer.IsAvailable())
	{
		m_tileBloomMaskBuffer.Release();
	}

	if (m_tileBloomMaskBuffer2.IsAvailable())
	{
		m_tileBloomMaskBuffer2.Release();
	}

	if (m_tileBloomInfoGen_TileInfoBuffer.IsAvailable())
	{
		m_tileBloomInfoGen_TileInfoBuffer.Release();
	}

	if (m_tileBloomInfoGen_DispatchThreadCount.IsAvailable())
	{
		m_tileBloomInfoGen_DispatchThreadCount.Release();
	}

	bBufferInit = false;
}

void CBloomSetupStage::Execute(CTexture* pSrcRT, CTexture* pTiledBloomDestRT, CTexture* pTexBloomOutRT)
{
	FUNCTION_PROFILER_RENDERER();
	PROFILE_LABEL_SCOPE("BLOOM_SETUP");

	if (pTiledBloomDestRT)
	{
		width = pTiledBloomDestRT->GetWidth();
		height = pTiledBloomDestRT->GetHeight();
	}
	else
	{
		return;
	}

	if (!bBufferInit)
	{
		InitBuffer();
		bBufferInit = true;
	}

	//D3D11 ERROR: ID3D11DeviceContext::Dispatch: The Unordered Access View in slot 1 of the Compute Shader unit was not created with the D3D11_BUFFER_UAV_FLAG_RAW flag, however the shader expects a RAW Buffer. 
	//This mismatch is invalid if the shader actually uses the view (e.g. it is not skipped due to shader code branching). [ EXECUTION ERROR #2097380: DEVICE_UNORDEREDACCESSVIEW_RAW_UNSUPPORTED]

	Vec4 screenSize(static_cast<float>(width), static_cast<float>(height), 1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height));
	const uint32 bloomSetUpDispatchSizeX = (width / groupSizeX) + (width % groupSizeX > 0 ? 1 : 0);
	const uint32 bloomSetUpdispatchSizeY = (height / groupSizeY) + (height % groupSizeY > 0 ? 1 : 0);

	//BloomSetUp
	const ColorI nulls = { 0, 0, 0, 0 };

	const bool bAsynchronousCompute = CRenderer::CV_r_D3D12AsynchronousCompute & BIT((eStage_BloomSetup - eStage_FIRST_ASYNC_COMPUTE)) ? true : false;
	{
		m_passBloomSetup.SetTechnique(CShaderMan::s_shBloomSetup, CCryNameTSCRC("BloomSetup"), 0);
		m_passBloomSetup.SetOutputUAV(0, pTiledBloomDestRT);
		m_passBloomSetup.SetOutputUAV(1, &m_tileBloomMaskBuffer);
		m_passBloomSetup.SetTextureSamplerPair(0, pSrcRT, EDefaultSamplerStates::LinearClamp);

		m_parameters->TexSize = screenSize;
		m_parameters->BufferSize = Vec4i(static_cast<int32>(bloomSetUpDispatchSizeX), static_cast<int32>(bloomSetUpdispatchSizeY), 0, 0);
		m_passBloomSetup.SetConstantBuffer(0, m_parameters.GetDeviceConstantBuffer());

		m_passBloomSetup.SetDispatchSize(bloomSetUpDispatchSizeX, bloomSetUpdispatchSizeY, 1);
		m_passBloomSetup.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

		SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
		m_passBloomSetup.Execute(computeCommandList);
	}


	//BloomTileInfo1Gen
	{
		m_passBloomTileInfo1Gen.SetTechnique(CShaderMan::s_shBloomSetup, CCryNameTSCRC("BloomTileInfoGen"), g_HWSR_MaskBit[HWSR_SAMPLE0]);

		m_passBloomTileInfo1Gen.SetOutputUAV(0, &m_tileBloomInfoGen_TileInfoBuffer);
		m_passBloomTileInfo1Gen.SetOutputUAV(1, &m_tileBloomInfoGen_DispatchThreadCount);
		m_passBloomTileInfo1Gen.SetOutputUAV(2, &m_tileBloomMaskBuffer2);

		m_passBloomTileInfo1Gen.SetBuffer(0,&m_tileBloomMaskBuffer);

		m_passBloomTileInfo1Gen.SetConstantBuffer(0, m_parameters.GetDeviceConstantBuffer());


		const uint32 BloomTileInfo1GenDispatchSizeX = (bloomSetUpDispatchSizeX / groupSizeX) + (bloomSetUpDispatchSizeX % groupSizeX > 0 ? 1 : 0);
		const uint32 BloomTileInfo1GenDispatchSizeY = (bloomSetUpdispatchSizeY / groupSizeY) + (bloomSetUpdispatchSizeY % groupSizeY > 0 ? 1 : 0);

		m_passBloomTileInfo1Gen.SetDispatchSize(BloomTileInfo1GenDispatchSizeX, BloomTileInfo1GenDispatchSizeY, 1);
		m_passBloomTileInfo1Gen.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

		SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
		m_passBloomTileInfo1Gen.Execute(computeCommandList);
	}

	static CCryNameR bloomParams0Name("bloomParams0");

	// Pass 1 Horizontal
	CClearSurfacePass::Execute(pTexBloomOutRT, ColorF(0, 0, 0, 0));
	{
		m_pass1H.SetTechnique(CShaderMan::s_shBloomSetup, CCryNameTSCRC("BloomFinal"), 0);
		m_pass1H.SetOutputUAV(0, pTexBloomOutRT);
		m_pass1H.SetOutputUAV(1, &m_tileBloomInfoGen_TileInfoBuffer);
		m_pass1H.SetTextureSamplerPair(0, pTiledBloomDestRT, EDefaultSamplerStates::LinearClamp);
		m_pass1H.SetConstantBuffer(1, m_parameters.GetDeviceConstantBuffer());
		m_pass1H.BeginConstantUpdate();
		
		Vec4 bloomParams0(1.0f* screenSize.z, 0, 0, 0);
		m_pass1H.SetConstant(bloomParams0Name, bloomParams0);
		m_pass1H.SetDispatchIndirectArgs(&m_tileBloomInfoGen_DispatchThreadCount, 0);
		m_pass1H.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

		SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
		m_pass1H.Execute(computeCommandList);
	}

	// Pass 1 Vertical
	{
		m_pass2H.SetTechnique(CShaderMan::s_shBloomSetup, CCryNameTSCRC("BloomFinal"), 0);
		m_pass2H.SetOutputUAV(0, pTiledBloomDestRT);
		m_pass2H.SetOutputUAV(1, &m_tileBloomInfoGen_TileInfoBuffer);
		m_pass2H.SetTextureSamplerPair(0, pTexBloomOutRT, EDefaultSamplerStates::LinearClamp);
		m_pass2H.SetConstantBuffer(1, m_parameters.GetDeviceConstantBuffer());
		m_pass2H.BeginConstantUpdate();

		Vec4 bloomParams0(0, 1.0f * screenSize.w, 0, 0);
		m_pass2H.SetConstant(bloomParams0Name, bloomParams0);
		m_pass2H.SetDispatchIndirectArgs(&m_tileBloomInfoGen_DispatchThreadCount, 0);
		m_pass2H.PrepareResourcesForUse(GetDeviceObjectFactory().GetCoreCommandList());

		SScopedComputeCommandList computeCommandList(bAsynchronousCompute);
		m_pass2H.Execute(computeCommandList);
	}
}

