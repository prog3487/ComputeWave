//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "SimpleMath.h"
#include "CommonStates.h"
#include "Waves.h"
#include "Structures.h"
#include "ConstantBuffer.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game() noexcept;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

	void CreateDeviceDependentResources();
	void BuildWavesGeometryBuffers();
	void CreateShaders();

	void UpdateCPU(DX::StepTimer const& timer);
	void DumpTexture(Microsoft::WRL::ComPtr<ID3D11Texture2D> src, const std::wstring& path);
	void UpdateGPU(DX::StepTimer const& timer);
	void RenderCPU();
	void RenderGPU();


    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

	//
	Waves mWaves;

	//
	std::unique_ptr<DirectX::CommonStates> m_states;

	DirectX::SimpleMath::Matrix m_WaveWorld;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_WaveVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_WaveIB;

	// cpu
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS_wave_cpu;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS_wave_cpu;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout_cpu;
	Bruce::ConstantBuffer<ConstantBuffer_WaveCPU> m_cbuffer_cpu;

	// gpu
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS_wave_gpu;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS_wave_gpu;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_CS_wave_gpu;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_CS_NewWave;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout_gpu;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_WaveVB_GPU;

	Bruce::ConstantBuffer<CBuffer_WaveGPU_Frame> m_cbuffer_frame_gpu;
	Bruce::ConstantBuffer<CBuffer_Disturb> m_cbuffer_disturb;
	Bruce::ConstantBuffer<CBuffer_WaveConstants> m_cbuffer_wave_constant;
	

	// cs resource
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_prevSolTex;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_currSolTex;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_nextSolTex;

	// for debug dump
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texDump;

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_prevSolUAV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_currSolUAV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_nextSolUAV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_displacement_SRV;

	
	// camera info
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	//
	enum class WaveMode
	{
		CPU,
		GPU,
	};

	WaveMode m_WaveMode = WaveMode::CPU;
};