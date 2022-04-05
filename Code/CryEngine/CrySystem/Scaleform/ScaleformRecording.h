// Copyright 2016-2021 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A wrapper around Scaleform's GRenderer interface to delegate all rendering to CryEngine's IRenderer interface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef INCLUDE_SCALEFORM3_SDK

#pragma warning(push)
#pragma warning(disable : 6326)   // Potential comparison of a constant with another constant
#pragma warning(disable : 6011)   // Dereferencing NULL pointer
#include <CryCore/Platform/CryWindows.h>
#include <GRenderer.h>            // includes <windows.h>
#include <CryRenderer/IScaleform.h>
#pragma warning(pop)

#include <vector>
#include <stack>
#include "GMemorySTLAlloc.h"
#include "GTexture_Impl.h"

#define ENABLE_FLASH_FILTERS

class GTextureXRender;
struct ITexture;
struct GRendererCommandBuffer;

class CCachedDataStore;

//////////////////////////////////////////////////////////////////////
struct IScaleformRecording : public GRenderer
{
	// GRenderer interface
public:
	virtual bool           GetRenderCaps(RenderCaps* pCaps) override = 0;
	virtual GTexture*      CreateTexture() override = 0;
	virtual GTexture*      CreateTextureYUV() override = 0;

	virtual GRenderTarget* CreateRenderTarget() override = 0;
	virtual void           SetDisplayRenderTarget(GRenderTarget* pRT, bool setState = 1) override = 0;
	virtual void           PushRenderTarget(const GRectF& frameRect, GRenderTarget* pRT) override = 0;
	virtual void           PopRenderTarget() override = 0;
	virtual GTexture*      PushTempRenderTarget(const GRectF& frameRect, UInt targetW, UInt targetH, bool wantStencil = 0) override = 0;

	virtual void           BeginDisplay(GColor backgroundColor, const GViewport& viewport, Float x0, Float x1, Float y0, Float y1) override = 0;
	virtual void           EndDisplay() override = 0;

	virtual void           SetMatrix(const GMatrix2D& m) override = 0;
	virtual void           SetUserMatrix(const GMatrix2D& m) override = 0;
	virtual void           SetCxform(const Cxform& cx) override = 0;

	virtual void           PushBlendMode(BlendType mode) override = 0;
	virtual void           PopBlendMode() override = 0;

	virtual void           SetPerspective3D(const GMatrix3D& projMatIn) override = 0;
	virtual void           SetView3D(const GMatrix3D& viewMatIn) override = 0;
	virtual void           SetWorld3D(const GMatrix3D* pWorldMatIn) override = 0;

	virtual void           SetVertexData(const void* pVertices, int numVertices, VertexFormat vf, CacheProvider* pCache = 0) override = 0;
	virtual void           SetIndexData(const void* pIndices, int numIndices, IndexFormat idxf, CacheProvider* pCache = 0) override = 0;
	virtual void           ReleaseCachedData(CachedData* pData, CachedDataType type) override = 0;

	virtual void           DrawIndexedTriList(int baseVertexIndex, int minVertexIndex, int numVertices, int startIndex, int triangleCount) override = 0;
	virtual void           DrawLineStrip(int baseVertexIndex, int lineCount) override = 0;

	virtual void           LineStyleDisable() override = 0;
	virtual void           LineStyleColor(GColor color) override = 0;

	virtual void           FillStyleDisable() override = 0;
	virtual void           FillStyleColor(GColor color) override = 0;
	virtual void           FillStyleBitmap(const FillTexture* pFill) override = 0;
	virtual void           FillStyleGouraud(GouraudFillType fillType, const FillTexture* pTexture0 = 0, const FillTexture* pTexture1 = 0, const FillTexture* pTexture2 = 0) override = 0;

	virtual void           DrawBitmaps(BitmapDesc* pBitmapList, int listSize, int startIndex, int count, const GTexture* pTi, const GMatrix2D& m, CacheProvider* pCache = 0) override = 0;

	virtual void           BeginSubmitMask(SubmitMaskMode maskMode) override = 0;
	virtual void           EndSubmitMask() override = 0;
	virtual void           DisableMask() override = 0;

	virtual UInt           CheckFilterSupport(const BlurFilterParams& params) override = 0;
	virtual void           DrawBlurRect(GTexture* pSrcIn, const GRectF& inSrcRect, const GRectF& inDestRect, const BlurFilterParams& params, bool isLast = false) override = 0;
	virtual void           DrawColorMatrixRect(GTexture* pSrcIn, const GRectF& inSrcRect, const GRectF& inDestRect, const Float* pMatrix, bool isLast = false) override = 0;

	virtual void           GetRenderStats(Stats* pStats, bool resetStats = false) override = 0;
	virtual void           GetStats(GStatBag* pBag, bool reset = true) override = 0;
	virtual void           ReleaseResources() override = 0;

	// IScaleformRenderer interface
public:
	virtual IScaleformPlayback*    GetPlayback() const = 0;

	virtual void                   SetClearFlags(uint32 clearFlags, ColorF clearColor = Clr_Transparent) = 0;
	virtual void                   SetCompositingDepth(float depth) = 0;

	virtual void                   SetStereoMode(bool stereo, bool isLeft) = 0;
	virtual void                   StereoEnforceFixedProjectionDepth(bool enforce) = 0;
	virtual void                   StereoSetCustomMaxParallax(float maxParallax) = 0;

	virtual void                   AvoidStencilClear(bool avoid) = 0;
	virtual void                   EnableMaskedRendering(bool enable) = 0;
	virtual void                   ExtendCanvasToViewport(bool extend) = 0;

	virtual void                   SetThreadIDs(uint32 mainThreadID, uint32 renderThreadID) = 0;
	virtual void                   SetRecordingCommandBuffer(GRendererCommandBuffer* pCmdBuf) = 0;
	virtual bool                   IsMainThread() const = 0;
	virtual bool                   IsRenderThread() const = 0;

	virtual void                   GetMemoryUsage(ICrySizer* pSizer) const = 0;

	virtual std::vector<ITexture*> GetTempRenderTargets() const = 0;
};

//////////////////////////////////////////////////////////////////////
struct GRendererCommandBuffer : private GRendererCommandBufferReadOnly
{
	friend struct IScaleformRecording;
	friend class CScaleformRecording;

	template<class T> friend void SF_Playback(T* pRenderer, GRendererCommandBuffer* pBuffer);

public:
	GRendererCommandBuffer(GMemoryHeap* pHeap)
		: GRendererCommandBufferReadOnly()
		, m_curReadPos(0)
		, m_curWritePos(0)
		, m_rawData(GMemorySTLAlloc<unsigned char>(pHeap))
		, m_refTextures(GMemorySTLAlloc<GTexture*>(pHeap))
		, m_refCaches(GMemorySTLAlloc<GTexture*>(pHeap))
	{
	}

	virtual ~GRendererCommandBuffer()
	{
		DropResourceRefs();
	}

	void Reset(size_t initCmdBufSize)
	{
		m_curReadPos = 0;
		m_curWritePos = 0;
		const size_t initCap = initCmdBufSize > 0 ? initCmdBufSize : 2048;
		if (m_rawData.capacity() < initCap)
			m_rawData.reserve(initCap);
	}

	size_t Size() const
	{
		return m_curWritePos;
	}

	size_t Capacity() const
	{
		return m_rawData.capacity();
	}

	void Render(IScaleformPlayback* pRenderer)
	{
		Assign(m_rawData.data(), m_rawData.size());

		gEnv->pRenderer->SF_Playback(pRenderer, this);
	}

	void DropResourceRefs()
	{
		gEnv->pRenderer->SF_Drain(this);

		m_refTextures.clear();
		m_refCaches.clear();
	}

private:
	template<class T> void Playback(T* pRenderer);

	void ResetReadPos()
	{
		m_curReadPos = 0;
	}

	size_t GetReadPos() const
	{
		assert(IsAligned(m_curReadPos));
		return m_curReadPos;
	}

	const void* GetReadPosPtr() const
	{
		assert(m_curReadPos < Size());
		assert(IsAligned(m_curReadPos));
		return Size() > 0 ? &m_rawData[m_curReadPos] : 0;
	}

	template<typename T>
	void Write(const T& arg)
	{
		assert(IsAligned(m_curWritePos));

		const size_t cellSize = Align(sizeof(T));
		Grow(cellSize);

		const size_t writePos = m_curWritePos;
		*((T*)&m_rawData[writePos]) = arg;
		m_curWritePos += cellSize;
	}

	void Write(const void* pData, const size_t size)
	{
		assert(IsAligned(m_curWritePos));

		if (pData && size)
		{
			Write(size);

			const size_t cellSize = Align(size);
			Grow(cellSize);
			memcpy(&m_rawData[m_curWritePos], pData, size);
			m_curWritePos += cellSize;
		}
		else
		{
			Write((size_t)0);
		}
	}

#if defined(_DEBUG)
	void Patch(size_t writePos, size_t data)
	{
		assert(IsAligned(writePos));
		assert(writePos + sizeof(size_t) <= Size());
		*((size_t*)&m_rawData[writePos]) = data;
	}
#endif

	template<typename T>
	T Read()
	{
		const size_t readPos = m_curReadPos;
		const size_t cellSize = Align(sizeof(T));

		assert(IsAligned(readPos));
		assert(readPos + cellSize <= Size());

		T arg = *((T*)&m_rawData[readPos]);
		m_curReadPos += cellSize;

		return arg;
	}

	template<typename T>
	const T& ReadRef()
	{
		const size_t readPos = m_curReadPos;
		const size_t cellSize = Align(sizeof(T));

		assert(IsAligned(readPos));
		assert(readPos + cellSize <= Size());

		const T& arg = *((const T*)&m_rawData[readPos]);
		m_curReadPos += cellSize;

		return arg;
	}

	void Skip(const size_t size, const bool alignSize)
	{
		assert(IsAligned(m_curReadPos));
		const size_t cellSize = alignSize ? Align(size) : size;
		assert(m_curReadPos + cellSize <= Size());
		m_curReadPos += cellSize;
	}

	void Grow(size_t toWrite)
	{
		size_t bufSize = m_rawData.size();
		size_t writePos = m_curWritePos;
		assert(writePos <= bufSize);
		if (writePos + toWrite > bufSize)
			m_rawData.resize(bufSize + toWrite);
	}

	size_t Align(size_t pos) const
	{
		return (pos + 3) & ~3;
	}

	bool IsAligned(size_t pos) const
	{
		return (pos & 3) == 0;
	}

	void AddRefCountedObject(GTexture* pTex)
	{
		m_refTextures.push_back(pTex);
	}

	void AddRefCountedObject(CCachedData* pCache)
	{
		m_refCaches.push_back(pCache);
	}

private:
	size_t m_curReadPos;
	size_t m_curWritePos;
	std::vector<unsigned char, GMemorySTLAlloc<unsigned char>> m_rawData;

	std::vector<_smart_ptr<GTexture>, GMemorySTLAlloc<_smart_ptr<GTexture>>> m_refTextures;
	std::vector<_smart_ptr<CCachedData>, GMemorySTLAlloc<_smart_ptr<CCachedData>>> m_refCaches;
};

//////////////////////////////////////////////////////////////////////
class CScaleformRecording final : public IScaleformRecording
{
	// GRenderer interface
public:

	virtual bool      GetRenderCaps(RenderCaps* pCaps) override;
	virtual GTexture* CreateTexture() override;
	virtual GTexture* CreateTextureYUV() override;

	virtual GRenderTarget* CreateRenderTarget() override;
	virtual void           SetDisplayRenderTarget(GRenderTarget* pRT, bool setState = 1) override;
	virtual void           PushRenderTarget(const GRectF& frameRect, GRenderTarget* pRT) override;
	virtual void           PopRenderTarget() override;
	virtual GTexture*      PushTempRenderTarget(const GRectF& frameRect, UInt targetW, UInt targetH, bool wantStencil = 0) override;

	virtual void BeginDisplay(GColor backgroundColor, const GViewport& viewport, Float x0, Float x1, Float y0, Float y1) override;
	virtual void EndDisplay() override;

	virtual void SetMatrix(const GMatrix2D& m) override;
	virtual void SetUserMatrix(const GMatrix2D& m) override;
	virtual void SetCxform(const Cxform& cx) override;

	virtual void PushBlendMode(BlendType mode) override;
	virtual void PopBlendMode() override;

	virtual void SetPerspective3D(const GMatrix3D& projMatIn) override;
	virtual void SetView3D(const GMatrix3D& viewMatIn) override;
	virtual void SetWorld3D(const GMatrix3D* pWorldMatIn) override;

	virtual void SetVertexData(const void* pVertices, int numVertices, VertexFormat vf, CacheProvider* pCache = 0) override;
	virtual void SetIndexData(const void* pIndices, int numIndices, IndexFormat idxf, CacheProvider* pCache = 0) override;
	virtual void ReleaseCachedData(CachedData* pData, CachedDataType type) override;

	virtual void DrawIndexedTriList(int baseVertexIndex, int minVertexIndex, int numVertices, int startIndex, int triangleCount) override;
	virtual void DrawLineStrip(int baseVertexIndex, int lineCount) override;

	virtual void LineStyleDisable() override;
	virtual void LineStyleColor(GColor color) override;

	virtual void FillStyleDisable() override;
	virtual void FillStyleColor(GColor color) override;
	virtual void FillStyleBitmap(const FillTexture* pFill) override;
	virtual void FillStyleGouraud(GouraudFillType fillType, const FillTexture* pTexture0 = 0, const FillTexture* pTexture1 = 0, const FillTexture* pTexture2 = 0) override;

	virtual void DrawBitmaps(BitmapDesc* pBitmapList, int listSize, int startIndex, int count, const GTexture* pTi, const GMatrix2D& m, CacheProvider* pCache = 0) override;

	virtual void BeginSubmitMask(SubmitMaskMode maskMode) override;
	virtual void EndSubmitMask() override;
	virtual void DisableMask() override;

	virtual UInt CheckFilterSupport(const BlurFilterParams& params) override;
	virtual void DrawBlurRect(GTexture* pSrcIn, const GRectF& inSrcRect, const GRectF& inDestRect, const BlurFilterParams& params, bool isLast = false) override;
	virtual void DrawColorMatrixRect(GTexture* pSrcIn, const GRectF& inSrcRect, const GRectF& inDestRect, const Float* pMatrix, bool isLast = false) override;

	virtual void GetRenderStats(Stats* pStats, bool resetStats = false) override;
	virtual void GetStats(GStatBag* pBag, bool reset = true) override;
	virtual void ReleaseResources() override;

public:
	static void InitCVars();

public:
	CScaleformRecording();

	virtual ~CScaleformRecording();
	virtual IScaleformPlayback* GetPlayback() const override { return m_pPlayback; }

	// IFlashPlayer
	virtual void SetClearFlags(uint32 clearFlags, ColorF clearColor) override;
	virtual void SetCompositingDepth(float depth) override;

	virtual void SetStereoMode(bool stereo, bool isLeft) override;
	virtual void StereoEnforceFixedProjectionDepth(bool enforce) override;
	virtual void StereoSetCustomMaxParallax(float maxParallax) override;

	virtual void AvoidStencilClear(bool avoid) override;
	virtual void EnableMaskedRendering(bool enable) override;
	virtual void ExtendCanvasToViewport(bool extend) override;

	virtual void SetThreadIDs(uint32 mainThreadID, uint32 renderThreadID) override;
	virtual void SetRecordingCommandBuffer(GRendererCommandBuffer* pCmdBuf) override;
	virtual bool IsMainThread() const override;
	virtual bool IsRenderThread() const override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		// !!! don't have to add anything, all allocations go through GFx memory interface !!!
	}

	virtual std::vector<ITexture*> GetTempRenderTargets() const override;

private:
	int m_maxVertices;
	int m_maxIndices;

	// render stats
	threadID m_rsIdx;
	Stats m_renderStats[2];

	// resource usage tracker
	_smart_ptr<CCachedDataStore> m_pDataStore[3];
	std::stack<GTextureXRenderTempRT*> m_pTempRTs;
	std::stack<GTextureXRenderTempRTLockless*> m_pTempRTsLL;

	// lockless rendering
	threadID m_mainThreadID;
	threadID m_renderThreadID;
	GRendererCommandBuffer* m_pCmdBuf;
	IScaleformPlayback* m_pPlayback;
};

//////////////////////////////////////////////////////////////////////
class CCachedDataStore : public CCachedData
{
public:
	static CCachedDataStore* Create(GRenderer* pRenderer, GRenderer::CacheProvider* pCache, GRenderer::CachedDataType type, IScaleformPlayback* pOwner, const IScaleformPlayback::DeviceData* pData);

	static CCachedDataStore* CreateOrReuse(GRenderer* pRenderer, GRenderer::CacheProvider* pCache, GRenderer::CachedDataType type, IScaleformPlayback* pOwner, const void* pVertices, int numVertices, GRenderer::VertexFormat vf);
	static CCachedDataStore* CreateOrReuse(GRenderer* pRenderer, GRenderer::CacheProvider* pCache, GRenderer::CachedDataType type, IScaleformPlayback* pOwner, const void* pIndices, int numIndices, GRenderer::IndexFormat idxf);
	static CCachedDataStore* CreateOrReuse(GRenderer* pRenderer, GRenderer::CacheProvider* pCache, GRenderer::CachedDataType type, IScaleformPlayback* pOwner, const GRenderer::BitmapDesc* pBitmapList, int numBitmaps);

public:
	GFC_MEMORY_REDEFINE_NEW(CCachedDataStore, GStat_Default_Mem)

private:
	void LockForBackup()
	{
		LockInternal(-1);
	}

public:
	void BackupAndRelease()
	{
		LockForBackup();
		Unlock();
		Release();
	}

private:
	CCachedDataStore(GRenderer::CachedDataType type, IScaleformPlayback* pOwner, const IScaleformPlayback::DeviceData* pData)
		: CCachedData(pData)
		, m_type(type)
		, m_Owner(pOwner)
	{
		CryInterlockedIncrement(&ms_numInst);
	}

	virtual ~CCachedDataStore()
	{
		CryInterlockedDecrement(&ms_numInst);
		if (m_pData)
		{
			m_Owner->ReleaseDeviceData(const_cast<IScaleformPlayback::DeviceData*>(m_pData));
		}
	}

private:
	static volatile int ms_numInst;

private:
	GRenderer::CachedDataType m_type;
	IScaleformPlayback* m_Owner;
};

#endif // #ifdef INCLUDE_SCALEFORM3_SDK
