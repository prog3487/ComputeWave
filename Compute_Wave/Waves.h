//***************************************************************************************
// HillsDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs the calculations for the wave simulation.  After the simulation has been
// updated, the client must copy the current solution into vertex buffers for rendering.
// This class only does the calculations, it does not do any drawing.
//***************************************************************************************

#ifndef WAVES_H
#define WAVES_H

#include <Windows.h>
#include <DirectXMath.h>

class Waves
{
public:
	Waves();
	~Waves();

	size_t RowCount()const;
	size_t ColumnCount()const;
	size_t VertexCount()const;
	size_t TriangleCount()const;

	// Returns the solution at the ith grid point.
	const DirectX::XMFLOAT3& operator[](int i)const { return mCurrSolution[i]; }

	//
	const DirectX::XMFLOAT2& GetTex(size_t i) const { return mTex[i]; }

	void Init(size_t m, size_t n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(size_t i, size_t j, float magnitude);

private:
	size_t mNumRows;
	size_t mNumCols;

	size_t mVertexCount;
	size_t mTriangleCount;

	// Simulation constants we can precompute.
	float mK1;
	float mK2;
	float mK3;

	float mTimeStep;
	float mSpatialStep;

	DirectX::XMFLOAT3* mPrevSolution;
	DirectX::XMFLOAT3* mCurrSolution;
	DirectX::XMFLOAT2* mTex;
};

#endif // WAVES_H