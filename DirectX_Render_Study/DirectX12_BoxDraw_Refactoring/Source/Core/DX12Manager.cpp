//===== ?C???N???[?h =====
#include "DX12Manager.h"

#include <d3dcompiler.h>
#include <iostream>

#include "InputManager.h"

#include "Windows.h"
#include "imgui.h"

#include "imgui_impl_dx12.h"
#include "D2DTextRenderer.h"

#include "ObjectManager.h"
#include "Camera.h"


// ?O???{??h???C?o?iNVIDIA / AMD?j??????A????A?v???N??????O??GPU??????g?p???????`???Ëx?@
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

//===== ???E?}?N????` =====
const UINT DX12Manager::m_FrameBufferCount = FRAME_BUFFER_COUNT;   //?t???[???o?b?t?@???

//===== ???\?b?h??` =====

//?C???X?^???X?èÔ
DX12Manager& DX12Manager::GetInstance()
{
	static DX12Manager instance;
	return instance;
}

// <???????A?I??????>
//??????????
bool DX12Manager::Initialize(HWND hwnd)
{
	HRESULT hr;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	//DXGI Factory ??
	//hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
	//if (FAILED(hr))
	//	return false;

	////?A?_?v?^?èÔ
	//ComPtr<IDXGIAdapter1> adapter;

	//for (UINT i = 0;
	//	m_factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND;
	//	++i)
	//{
	//	DXGI_ADAPTER_DESC1 desc;
	//	adapter->GetDesc1(&desc);

	//	// ?\?t?g?E?F?A?iMicrosoft Basic Render Driver???j??X?L?b?v
	//	if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
	//		continue;
	//	break;
	//}

	// ----- ?????\??O???{???I???????C?? -----

	//DXGI Factory ?? (??????????g??????AIDXGIFactory6 ???????????? CreateDXGIFactory1 ??g?p)
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
	if (FAILED(hr))
		return false;

	// ?A?_?v?^?èÔ
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory6;

	// Factory??IDXGIFactory6??L???X?g????AEnumAdapterByGpuPreference??g??????????
	if (SUCCEEDED(m_factory.As(&factory6)))
	{
		// DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE ??w???Á∑???A
		// ?????\???????iVRAM???????O??GPU???j????O???{???????????
		for (UINT i = 0;
			factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
			++i)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			// ?\?t?g?E?F?A?iMicrosoft Basic Render Driver???j??X?L?b?v
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			// ???p?t?H?[?}???X??n?[?h?E?F?AGPU??????????????_??m??
			break;
		}
	}
	else
	{
		// ?A?OS????IDXGIFactory6???g?????????t?H?[???o?b?N?i????R?[?h??????j
		for (UINT i = 0;
			m_factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND;
			++i)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			break;
		}
	}

	//?f?o?C?X??
	hr = D3D12CreateDevice(
		adapter.Get(),
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	);

	if (FAILED(hr))
		return false;

	//?R?}???h?L???[??
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	hr = m_device->CreateCommandQueue(
		&queueDesc,
		IID_PPV_ARGS(&m_commandQueue)
	);

	if (FAILED(hr))
		return false;


	// <?X???b?v?`?F?[????>
	
	//?X???b?v?`?F?[??????
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_FrameBufferCount;
	//0??E?B???h?E?????????
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	//?X???b?v?`?F?[????
	ComPtr<IDXGISwapChain1> swapChain1;

	hr = m_factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1
	);

	if (FAILED(hr))
		return false;

	//IDXGISwapChain4 ????
	swapChain1.As(&m_swapChain);


	// <RTV??>
	//RTV?q?[?v????A??
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_FrameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = m_device->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvHeap)
	);

	if (FAILED(hr))
		return false;

	//?f?B?X?N???v?^?T?C?Y?èÔ
	m_rtvDescriptorSize =
		m_device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);

	//?o?b?N?o?b?t?@?èÔ??RTV??
	//?q?[?v????n???h???èÔ
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_FrameBufferCount; ++i)
	{
		//?o?b?N?o?b?t?@?èÔ
		hr = m_swapChain->GetBuffer(
			i,
			IID_PPV_ARGS(&m_renderTargets[i])
		);

		if (FAILED(hr))
			return false;

		//RTV??
		m_device->CreateRenderTargetView(
			m_renderTargets[i].Get(),
			nullptr,
			rtvHandle
		);

		// ????f?B?X?N???v?^????
		rtvHandle.ptr += m_rtvDescriptorSize;
	}


	//?R?}???h?I?u?W?F?N?g??
	CreateCommandObjects();


	//?t?F???X??
	CreateFence();

	//ImGUi???A??????






	// ===== ?[?x?o?b?t?@?? =====
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC depthResourceDesc = {};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = SCREEN_WIDTH;
	depthResourceDesc.Height = SCREEN_HEIGHT;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.SampleDesc.Quality = 0;
	depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthBuffer)
	);


	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 0;

	m_device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&m_dsvHeap)
	);


	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_device->CreateDepthStencilView(
		m_depthBuffer.Get(),
		&dsvDesc,
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);



	//SRV
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 128;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));

	m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_srvAllocator.Create(m_device.Get(), m_srvHeap.Get());

	//?n???h???e?X?g
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();




	//view,proj???????
	//view
	m_view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0, 2.5, -5, 1),
		DirectX::XMVectorSet(0, 2, 0, 1),
		DirectX::XMVectorSet(0, 1, 0, 0));
;
//Todo : Model??`??e?X?g?????????A???????????


	//?????‡Õ??p
	//m_view = DirectX::XMMatrixLookAtLH(
	//	DirectX::XMVectorSet(40, 0, 0, 1),
	//	DirectX::XMVectorSet(0, 0, 0, 1),
	//	DirectX::XMVectorSet(0, 1, 0, 0));

	//proj
	m_proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,
		0.1f,
		1000.0f);
	
	//m_commandQueue->ExecuteCommandLists(m_commandList);

	if (!D2DTextRenderer::GetInstance().Initialize(m_device.Get(), m_commandQueue.Get(), m_swapChain.Get(), m_Width, m_Height, m_FrameBufferCount)) { return false; }
	return true;
}

//?I??????
void DX12Manager::Finalize()
{
	m_srvAllocator.Destroy();
	D2DTextRenderer::GetInstance().Finalize();
	m_commandQueue.Reset();
	m_device.Reset();
	m_factory.Reset();
}


//----- ?X?V???? -----
void DX12Manager::Update()
{
}

void DX12Manager::ForceWait()
{
	// ???????????i?R?s?[???j?????????
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

	// ?????????B??????ACPU????S???~????????
	m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	WaitForSingleObject(m_fenceEvent, INFINITE);

	// ????????l??X?V???????
	m_fenceValue++;
}

//????view,proj??Getter
DirectX::XMMATRIX DX12Manager::GetView() { return ObjectManager::GetInstance().GetCamera()->GetView(); }

DirectX::XMMATRIX DX12Manager::GetProj() { return ObjectManager::GetInstance().GetCamera()->GetProj(); }


//----- ?`???? -----
void DX12Manager::BeginDraw()
{





	// GPU???O??t???[?????????I????????
	if (m_fence->GetCompletedValue() < m_fenceValue - 1)
	{
		m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}


	// ???????U?A????R?}???h???X?g?????????I??N???A?????
	//m_commandAllocator->Reset();
	//m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	//???????Z?b?g
	ResetIsOccluded();

	// 1. ?t???[???C???f?b?N?X?X?V
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// 2. ???Z?b?g
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);


	// 3. PRESENT ?? RENDER_TARGET ??J??
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &barrier);

	// 4. RTV?n???h???èÔ
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;






	D3D12_VIEWPORT viewport{};
	viewport.Width = (float)m_Width;
	viewport.Height = (float)m_Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = m_Width;
	scissorRect.bottom = m_Height;

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	// 5. DSV?n???h???èÔ
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart();


	// 6. ?N???A
	//????F
	FLOAT clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

	//?????_?[?^?[?Q?b?g??Z?b?g
	m_commandList->OMSetRenderTargets(
		1,
		&rtvHandle,
		FALSE,
		&dsvHandle
	);

	//RtV,DSV??Z?b?g
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(
		dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr
	);




}

void DX12Manager::EndDraw()
{
	m_commandList->Close();

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, commandLists);

	D2DTextRenderer::GetInstance().Render(m_frameIndex);

	HRESULT hr = m_swapChain->Present(1, 0);
	//HRESULT hr = g_pSwapChain->Present(0, g_SwapChainTearingSupport ? DXGI_PRESENT_ALLOW_TEARING : 0); // Present without vsync
	m_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

	// 5. ?t?F???X??L???[??}??
	const UINT64 fenceToWaitFor = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
	m_fenceValue++;
}


//???????p?????B
void DX12Manager::CreateCommandObjects()
{

	//?R?}???h?A???P?[?^?[??
	m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_commandAllocator)
	);

	//?R?}???h???X?g??
	m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)
	);

	//?R?}???h???X?g???????? recording ??????AClose ???????
	m_commandList->Close();
}

void DX12Manager::CreateFence()
{
	m_device->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_fence)
	);

	m_fenceValue = 1;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}


void DX12Manager::ResizeRenderTarget(LPARAM lParam)
{
	D2DTextRenderer::GetInstance().Finalize();
	CleanupRenderTarget();              //RenderTarget??j??
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	m_swapChain->GetDesc1(&desc);
	HRESULT result = m_swapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), desc.Format, desc.Flags);   //?o?b?t?@?[????T?C?Y
	IM_ASSERT(SUCCEEDED(result) && "Failed to resize swapchain.");

	//m_commandAllocator->Reset();
	//m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	CreateRenderTarget();

	D2DTextRenderer::GetInstance().Initialize(m_device.Get(), m_commandQueue.Get(), m_swapChain.Get(), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), m_FrameBufferCount);
}
void DX12Manager::ResizeViewPort(LPARAM lParam)
{
	D3D12_VIEWPORT viewport{};
	viewport.Width = LOWORD(lParam);
	viewport.Height = HIWORD(lParam);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = LOWORD(lParam);
	scissorRect.bottom = HIWORD(lParam);

	//m_commandList->RSSetViewports(1, &viewport);
	//m_commandList->RSSetScissorRects(1, &scissorRect);
}
void DX12Manager::ResizeDepthBuffer(LPARAM lParam)
{

	// ?A????\?[?X?j??
	m_depthBuffer.Reset();
	m_dsvHeap.Reset();

	// 1. ?q?[?v?v???p?e?B?i??????????????j
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// 2. ???T?C?Y??? DepthStencil ????\?[?X??
	D3D12_RESOURCE_DESC depthResourceDesc = {};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = LOWORD(lParam);
	depthResourceDesc.Height = HIWORD(lParam);
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.SampleDesc.Quality = 0;
	depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// Create depth buffer texture
	HRESULT hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthBuffer)
	);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth buffer!\n");
		return;
	}

	// 3. DSV ?q?[?v??
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = m_device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&m_dsvHeap)
	);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create DSV heap!\n");
		return;
	}

	// 4. DSV ???
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	m_device->CreateDepthStencilView(
		m_depthBuffer.Get(),
		&dsvDesc,
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);
}
void DX12Manager::CreateRenderTarget()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_FrameBufferCount; ++i)
	{
		//?o?b?N?o?b?t?@?èÔ
		HRESULT hr = m_swapChain->GetBuffer(
			i,
			IID_PPV_ARGS(&m_renderTargets[i])
		);

		if (FAILED(hr))
			return;



		//RTV??
		m_device->CreateRenderTargetView(
			m_renderTargets[i].Get(),
			nullptr,
			rtvHandle
		);

		// ????f?B?X?N???v?^????
		rtvHandle.ptr += m_rtvDescriptorSize;
	}
}
void DX12Manager::CleanupRenderTarget()
{
	WaitForPendingOperations();

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
		m_renderTargets[i].Reset(); 
}

void DX12Manager::WaitForPendingOperations()
{

	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_fenceValue++;

}

D3D12_GPU_DESCRIPTOR_HANDLE DX12Manager::GetHeadGpuSrvHandle()
{
	return m_srvHeap->GetGPUDescriptorHandleForHeapStart();
}


//
bool DX12Manager::IsOccluded(HWND hwnd)
{
	return ((m_SwapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) || IsIconic(hwnd));
}

void DX12Manager::ResetIsOccluded()
{
	m_SwapChainOccluded = false;
}








