#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class SpinBlur : public BaseShader
{
private:


public:

	SpinBlur(ID3D11Device* device, HWND hwnd);
	~SpinBlur();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
};