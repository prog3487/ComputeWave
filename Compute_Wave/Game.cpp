//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <vector>
#include "ReadData.h"
#include "VertexTypes.h"
#include "MathHelper.h"
#include <string>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

const size_t size_m = 200;
const size_t size_n = 200;
const float dx = 0.8f;
const float dt = 0.03f;
const float speed = 3.25f;
const float damping = 0.4f;

const float DisturbPeriod = 0.25f;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_11_0)
{
	m_Theta		= 1.5f * XM_PI;
	m_Phi		= 0.1f * XM_PI;
	m_Radius	= 200.0f;
	m_WaveWorld = Matrix::Identity;
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

	mWaves.Init(size_m, size_n, dx, dt, speed, damping);

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */


}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	CalculateFrameStats(timer);

	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	m_view = Matrix::CreateLookAt(Vector3(x, y, z), Vector3::Zero, Vector3::UnitY);

	//
	switch (m_WaveMode)
	{
	case WaveMode::CPU:
		UpdateCPU(timer);
		break;
	case WaveMode::GPU:
		UpdateGPU(timer);
		break;
	default:
		break;
	}
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

	switch (m_WaveMode)
	{
	case WaveMode::CPU:
		RenderCPU();
		break;
	case WaveMode::GPU:
		RenderGPU();
		break;
	default:
		break;
	}

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(0, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
		nullptr,					// specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // Initialize device dependent objects here (independent of window size).
	CreateDeviceDependentResources();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // Initialize windows-size dependent objects here.
	m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, float(backBufferWidth) / float(backBufferHeight), 0.01f, 1000.f);
}

void Game::OnDeviceLost()
{
    // Add Direct3D resource cleanup here.
	m_states.reset();
	m_WaveVB.Reset();
	m_WaveIB.Reset();
	m_InputLayout_cpu.Reset();
	m_VS_wave_cpu.Reset();
	m_PS_wave_cpu.Reset();

	m_VS_wave_gpu.Reset();
	m_PS_wave_gpu.Reset();
	m_CS_wave_gpu.Reset();
	m_InputLayout_gpu.Reset();
	m_CS_NewWave.Reset();

	m_WaveVB_GPU.Reset();

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}

void Game::CreateDeviceDependentResources()
{
	m_states = std::make_unique<DirectX::CommonStates>(m_d3dDevice.Get());

	BuildWavesGeometryBuffers();
	
	CreateShaders();

	m_cbuffer_cpu.Create(m_d3dDevice.Get());
	m_cbuffer_frame_gpu.Create(m_d3dDevice.Get());
	m_cbuffer_disturb.Create(m_d3dDevice.Get());
	m_cbuffer_wave_constant.Create(m_d3dDevice.Get());

	{
		float d = damping * dt + 2.0f;
		float e = (speed * speed) *  (dt * dt) / (dx * dx);

		CBuffer_WaveConstants cbuffer;
		cbuffer.wc0 = (damping * dt - 2.0f) / d;
		cbuffer.wc1 = (4.0f - 8.0f*e) / d;
		cbuffer.wc2 = (2.0f*e) / d;

		m_cbuffer_wave_constant.SetData(m_d3dContext.Get(), cbuffer);
	}

	//
	{
		// texture resources
		CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R32_FLOAT, size_m, size_n, 1, 0, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

		DX::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D(&desc, nullptr, m_prevSolTex.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D(&desc, nullptr, m_currSolTex.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D(&desc, nullptr, m_nextSolTex.ReleaseAndGetAddressOf()));

		// uav
		DX::ThrowIfFailed(
			m_d3dDevice->CreateUnorderedAccessView(m_prevSolTex.Get(), nullptr, m_prevSolUAV.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateUnorderedAccessView(m_currSolTex.Get(), nullptr, m_currSolUAV.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateUnorderedAccessView(m_nextSolTex.Get(), nullptr, m_nextSolUAV.ReleaseAndGetAddressOf()));


		// srv for vs
		DX::ThrowIfFailed(
			m_d3dDevice->CreateShaderResourceView(m_nextSolTex.Get(), nullptr, m_displacement_SRV.ReleaseAndGetAddressOf()));
	}

	{
		CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R32_FLOAT, size_m, size_n, 1, 0, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D(&desc, nullptr, m_texDump.ReleaseAndGetAddressOf()));
	}

}

void Game::BuildWavesGeometryBuffers()
{
	// create vb for cpu
	{
		CD3D11_BUFFER_DESC vbd(
			sizeof(VertexWave) * mWaves.VertexCount(),
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE);

		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(&vbd, nullptr, m_WaveVB.ReleaseAndGetAddressOf()));
	}

	// create ib
	std::vector<uint32_t> indices(3 * mWaves.TriangleCount());	// 3 indices per face

	// Iterate over each quad.
	size_t m = mWaves.RowCount();
	size_t n = mWaves.ColumnCount();
	int k = 0;
	for (size_t i = 0; i < m - 1; ++i)
	{
		for (size_t j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	CD3D11_BUFFER_DESC ibd(sizeof(uint32_t) * indices.size(), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE);
	D3D11_SUBRESOURCE_DATA iinitData = { 0 };
	iinitData.pSysMem = indices.data();

	DX::ThrowIfFailed(
		m_d3dDevice->CreateBuffer(&ibd, &iinitData, m_WaveIB.ReleaseAndGetAddressOf()));

	{	// create vertex buffer for gpu & initialize data
		CD3D11_BUFFER_DESC vbd(
			sizeof(VertexWave_GPU) * mWaves.VertexCount(),
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DEFAULT);

		std::vector<VertexWave_GPU> gpuVBData(mWaves.VertexCount());
		for (size_t i = 0; i < mWaves.VertexCount(); ++i)
		{
			gpuVBData[i].Pos = mWaves[i];
			gpuVBData[i].Color = Colors::Black;
			gpuVBData[i].Tex = mWaves.GetTex(i);
		}

		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.pSysMem = gpuVBData.data();

		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(&vbd, &initData, m_WaveVB_GPU.ReleaseAndGetAddressOf()));
	}
}

void Game::CreateShaders()
{
	{	// create vs & input layout
		auto vs_blob = DX::ReadData(L"VS_wave_cpu.cso");
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(vs_blob.data(), vs_blob.size(), nullptr, m_VS_wave_cpu.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateInputLayout(
				VertexPositionColor::InputElements,
				VertexPositionColor::InputElementCount,
				vs_blob.data(), vs_blob.size(), m_InputLayout_cpu.ReleaseAndGetAddressOf()));
	}

	{	// ps
		auto blob = DX::ReadData(L"PS_wave_cpu.cso");
		DX::ThrowIfFailed(
			m_d3dDevice->CreatePixelShader(blob.data(), blob.size(), nullptr, m_PS_wave_cpu.ReleaseAndGetAddressOf()));
	}

	//--------------------------------------------------------

	{	// create vs & input layout (GPU)
		auto vs_blob = DX::ReadData(L"VS_wave_gpu.cso");
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(vs_blob.data(), vs_blob.size(), nullptr, m_VS_wave_gpu.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(
			m_d3dDevice->CreateInputLayout(
				VertexPositionColorTexture::InputElements,
				VertexPositionColorTexture::InputElementCount,
				vs_blob.data(), vs_blob.size(), m_InputLayout_gpu.ReleaseAndGetAddressOf()));
	}

	{	// ps wave gpu
		auto blob = DX::ReadData(L"PS_wave_gpu.cso");
		DX::ThrowIfFailed(
			m_d3dDevice->CreatePixelShader(blob.data(), blob.size(), nullptr, m_PS_wave_gpu.ReleaseAndGetAddressOf()));
	}

	{	// cs wave gpu
		auto blob = DX::ReadData(L"CS_Wave_gpu.cso");
		DX::ThrowIfFailed(
			m_d3dDevice->CreateComputeShader(blob.data(), blob.size(), nullptr, m_CS_wave_gpu.ReleaseAndGetAddressOf()));

		blob = DX::ReadData(L"CS_NewWave.cso");
		DX::ThrowIfFailed(
			m_d3dDevice->CreateComputeShader(blob.data(), blob.size(), nullptr, m_CS_NewWave.ReleaseAndGetAddressOf()));
	}
}

void Game::UpdateCPU(DX::StepTimer const& timer)
{
	float elapsedTime = float(timer.GetElapsedSeconds());

	//
	// Every quarter second, generate a random wave.
	//
	static double t_base = 0.0f;
	if ((timer.GetTotalSeconds() - t_base) >= DisturbPeriod)
	{
		t_base = timer.GetTotalSeconds();

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(elapsedTime);

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	DX::ThrowIfFailed(
		m_d3dContext->Map(m_WaveVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	VertexWave* v = reinterpret_cast<VertexWave*>(mappedData.pData);
	for (size_t i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Color = Colors::Black;
	}

	m_d3dContext->Unmap(m_WaveVB.Get(), 0);
}

void Game::DumpTexture(ComPtr<ID3D11Texture2D> src, const std::wstring& path)
{
//#define DUMP_TEXTURE_FILE

#ifdef DUMP_TEXTURE_FILE
	m_d3dContext->CopyResource(m_texDump.Get(), src.Get());

	std::ofstream fout(path.c_str());

	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_d3dContext->Map(m_texDump.Get(), 0, D3D11_MAP_READ, 0, &mappedData);

	float* dataView = reinterpret_cast<float*>(mappedData.pData);
	for (size_t i=0; i<size_m; ++i)
	{
		for (size_t j = 0; j < size_n; ++j)
		{
			fout << dataView[i * size_m + j] << " ";
		}
		fout << std::endl;
	}

	m_d3dContext->Unmap(m_texDump.Get(), 0);

	fout.close();
#endif // DUMP_TEXTURE_FILE
}

void Game::UpdateGPU(DX::StepTimer const& timer)
{
	// create new wave using compute shader
	static double t_base = 0.0f;
	if ((timer.GetTotalSeconds() - t_base) >= DisturbPeriod)
	{
		t_base = timer.GetTotalSeconds();

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;
		float r = MathHelper::RandF(1.0f, 2.0f);

		m_d3dContext->CSSetShader(m_CS_NewWave.Get(), nullptr, 0);

		// views
		ID3D11UnorderedAccessView* views[] = { m_currSolUAV.Get() };
		m_d3dContext->CSSetUnorderedAccessViews(0, _countof(views), views, nullptr);

		// cbuffer
		CBuffer_Disturb cbuffer;
		cbuffer.DisturbMag = r;
		cbuffer.DisturbX = i;
		cbuffer.DisturbY = j;
		m_cbuffer_disturb.SetData(m_d3dContext.Get(), cbuffer);
		m_d3dContext->CSSetConstantBuffers(0, 1, m_cbuffer_disturb.GetAddressOf());

		// dispatch
		m_d3dContext->Dispatch(1, 1, 1);

		// unbound
		ID3D11UnorderedAccessView* null_views[] = { nullptr };
		m_d3dContext->CSSetUnorderedAccessViews(0, _countof(null_views), null_views, nullptr);

		// debug dump texture
		DumpTexture(m_currSolTex, L"dumpTex0.txt");
	}

	{	// update wave
		m_d3dContext->CSSetShader(m_CS_wave_gpu.Get(), nullptr, 0);

		// views
		ID3D11UnorderedAccessView* views[] = { m_prevSolUAV.Get(), m_currSolUAV.Get(), m_nextSolUAV.Get() };
		m_d3dContext->CSSetUnorderedAccessViews(0, _countof(views), views, nullptr);

		// cbuffer
		m_d3dContext->CSSetConstantBuffers(0, 1, m_cbuffer_wave_constant.GetAddressOf());

		// calculate how many thread groups 
		// dispatch
		UINT xgroups = size_m / 16;
		UINT ygroups = size_n / 16;
		m_d3dContext->Dispatch(xgroups, ygroups, 1);

		// unbound
		ID3D11UnorderedAccessView* null_views[] = { nullptr, nullptr, nullptr };
		m_d3dContext->CSSetUnorderedAccessViews(0, _countof(null_views), null_views, nullptr);

		// debug dump texture
		DumpTexture(m_nextSolTex, L"dumpTex1.txt");

		// swap solution buffers
		auto resTemp = m_prevSolUAV;
		m_prevSolUAV = m_currSolUAV;
		m_currSolUAV = m_nextSolUAV;
		m_nextSolUAV = resTemp;
	}
}

void Game::RenderCPU()
{
	m_d3dContext->IASetInputLayout(m_InputLayout_cpu.Get());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dContext->VSSetShader(m_VS_wave_cpu.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_PS_wave_cpu.Get(), nullptr, 0);

	m_d3dContext->RSSetState(m_states->Wireframe());

	UINT stride = sizeof(VertexWave);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_WaveVB.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_WaveIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	// set constant buffer
	ConstantBuffer_WaveCPU cbuffer;
	cbuffer.WorldViewProj = (m_WaveWorld * m_view * m_proj).Transpose();
	m_cbuffer_cpu.SetData(m_d3dContext.Get(), cbuffer);

	ID3D11Buffer* buffers[1] = { m_cbuffer_cpu.GetBuffer() };
	m_d3dContext->VSSetConstantBuffers(0, 1, buffers);

	m_d3dContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);
}

void Game::RenderGPU()
{
	m_d3dContext->IASetInputLayout(m_InputLayout_gpu.Get());
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dContext->VSSetShader(m_VS_wave_gpu.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_PS_wave_gpu.Get(), nullptr, 0);

	m_d3dContext->RSSetState(m_states->Wireframe());

	UINT stride = sizeof(VertexWave_GPU);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_WaveVB_GPU.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_WaveIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	// update constant buffer
	CBuffer_WaveGPU_Frame cbuffer;
	cbuffer.World = m_WaveWorld.Transpose();
	cbuffer.ViewProj = (m_view * m_proj).Transpose();
	m_cbuffer_frame_gpu.SetData(m_d3dContext.Get(), cbuffer);

	m_d3dContext->VSSetConstantBuffers(0, 1, m_cbuffer_frame_gpu.GetAddressOf());
	m_d3dContext->VSSetShaderResources(0, 1, m_displacement_SRV.GetAddressOf());

	ID3D11SamplerState* samplers[] = { m_states->LinearWrap() };
	m_d3dContext->VSSetSamplers(0, 1, samplers);

	// draw
	m_d3dContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

	// unbound
	ID3D11ShaderResourceView* nullViews[] = { nullptr };
	m_d3dContext->VSSetShaderResources(0, 1, nullViews);
}

void Game::CalculateFrameStats(DX::StepTimer const& timer)
{
	const static float updateGap = 1.0f;
	static float elapsedTime = 0.0f;

	elapsedTime += (float)timer.GetElapsedSeconds();
	if (elapsedTime >= updateGap)
	{
		std::wstring fpstxt(L"FPS : ");
		fpstxt += std::to_wstring(timer.GetFramesPerSecond());
		::SetWindowText(m_window, fpstxt.c_str());
		elapsedTime -= updateGap;
	}
}
