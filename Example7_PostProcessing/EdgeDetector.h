#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class EdgeDetector : public BaseShader
{
	struct EdgeBufferType
	{
		float screenHeight;
		float screenWidth;
		float thickness;
		float lowerTol;
		float upperTol;
		XMFLOAT3 padding;
	};

public:

	EdgeDetector(ID3D11Device* device, HWND hwnd);
	~EdgeDetector();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, float width, float height, float thickness, float lowerTol, float upperTol);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* edgeState;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* edgeBuffer;
};

