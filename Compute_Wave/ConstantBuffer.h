#pragma once


namespace Bruce
{
	// Strongly typed wrapper around a D3D constant buffer.
	template<typename T>
	class ConstantBuffer
	{
	public:
		// Constructor.
		ConstantBuffer() = default;
		explicit ConstantBuffer(_In_ ID3D11Device* device)
		{
			Create(device);
		}

		ConstantBuffer(ConstantBuffer const&) = delete;
		ConstantBuffer& operator= (ConstantBuffer const&) = delete;

		void Create(_In_ ID3D11Device* device)
		{
			D3D11_BUFFER_DESC desc = {};

			desc.ByteWidth = sizeof(T);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			DX::ThrowIfFailed(
				device->CreateBuffer(&desc, nullptr, mConstantBuffer.ReleaseAndGetAddressOf())
			);
		}


		// Writes new data into the constant buffer.
		void SetData(_In_ ID3D11DeviceContext* deviceContext, T const& value)
		{
			assert(mConstantBuffer);

			D3D11_MAPPED_SUBRESOURCE mappedResource;

			DX::ThrowIfFailed(
				deviceContext->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)
			);

			*static_cast<T*>(mappedResource.pData) = value;

			deviceContext->Unmap(mConstantBuffer.Get(), 0);
		}

		// Looks up the underlying D3D constant buffer.
		ID3D11Buffer* GetBuffer()
		{
			return mConstantBuffer.Get();
		}

		ID3D11Buffer* const* GetAddressOf() const throw()
		{
			return mConstantBuffer.GetAddressOf();
		}

		ID3D11Buffer** GetAddressOf() throw()
		{
			return mConstantBuffer.GetAddressOf();
		}


	private:
		// The underlying D3D object.
		Microsoft::WRL::ComPtr<ID3D11Buffer> mConstantBuffer;
	};
}
