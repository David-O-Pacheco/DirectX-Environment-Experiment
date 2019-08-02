#include <windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include <list>
#include <XInput.h>
#include "keyprocessor.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")

static wchar_t className[] = L"Direct3D";

LRESULT CALLBACK WinProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

XMMATRIX matWorld;
XMMATRIX matView;
XMMATRIX matProjection;

ID3D11Buffer *vertexBuffer = NULL;
ID3D11VertexShader *vertexShader = NULL;
ID3D11PixelShader *pixelShader = NULL;
ID3D11InputLayout *inputLayout = NULL;
ID3D11Buffer *constantBuffer = NULL;
ID3D11RenderTargetView *backBuffer;
ID3D11Device *d3dDevice;
ID3D11DeviceContext *d3dContext;
IDXGISwapChain *swapChain;
ID3D11ShaderResourceView *CubeMap;
ID3D11InputLayout *skyboxVertexLayout;
ID3D11RasterizerState *rsCullingOff;
ID3D11RasterizerState *rsCullingOn;
ID3D11DepthStencilState *DSDepthOff;
ID3D11DepthStencilState *DSDepthOn;
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilView* depthStencilView;
ID3D11Buffer *skyboxVertexBuffer = NULL;
ID3D11VertexShader *skyboxVertexShader = NULL;
ID3D11PixelShader *skyboxPixelShader = NULL;
ID3D11InputLayout *vertexLayout = NULL;
ID3D11ShaderResourceView *imageData;
ID3D11SamplerState *sampler;
DWORD frameTime;
ID3D11Buffer *terrainVertexBuffer = NULL;
ID3D11Buffer *terrainIndexBuffer = NULL;
int NumIndices;
int NumBuilding1Vertices;
int NumBuilding2Vertices;
int NumCar1Vertices;
int NumTree1Vertices;
int NumLamp1Vertices;
ID3D11ShaderResourceView *terrainTex;
ID3D11RasterizerState *rs;
ID3D11ShaderResourceView *building1Tex;
ID3D11Buffer *building1VertexBuffer;

ID3D11ShaderResourceView *building2Tex;
ID3D11Buffer *building2VertexBuffer;

ID3D11ShaderResourceView *car1Tex;
ID3D11Buffer *car1VertexBuffer;

ID3D11ShaderResourceView *tree1Tex;
ID3D11Buffer *tree1VertexBuffer;

ID3D11ShaderResourceView *lamp1Tex;
ID3D11Buffer *lamp1VertexBuffer;


XMFLOAT3 skybox[] = {

	// Right side face
	XMFLOAT3(-1.0f, -1.0f, -1.0f),
	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),

	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),

	// FRONT
	XMFLOAT3(1.0f, 1.0f, -1.0f),
	XMFLOAT3(1.0f, 1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),

	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),
	XMFLOAT3(1.0f, 1.0f, -1.0f),

	// Back face, note that points are in counter clockwise order
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f, 1.0f, 1.0f),

	XMFLOAT3(1.0f, 1.0f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),

	//// Left side face, note that points are in counter clockwise order
	XMFLOAT3(-1.0f, 1.0f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(-1.0f, -1.0f, -1.0f),

	XMFLOAT3(-1.0f, -1.0f, -1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f),

	//// Top face
	XMFLOAT3(-1.0f, 1.0f, -1.0f),
	XMFLOAT3(-1.0f, 1.0f, 1.0f),
	XMFLOAT3(1.0f, 1.0f, 1.0f),

	XMFLOAT3(1.0f, 1.0f, 1.0f),
	XMFLOAT3(1.0f, 1.0f, -1.0f),
	XMFLOAT3(-1.0f, 1.0f, -1.0f),

	//// Bottom face, note that points are in counter clockwise order
	XMFLOAT3(-1.0f, -1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),

	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, -1.0f),

};

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;

	Vertex() {
	}

	Vertex(float x, float y, float z, float u, float v) {
		pos.x = x;
		pos.y = y;
		pos.z = z;

		texCoord.x = u;
		texCoord.y = v;
	}
};

struct TerrainVertex {
	XMFLOAT3 tPos;
	XMFLOAT2 tTexCoord;

	TerrainVertex(XMFLOAT3 xyz, XMFLOAT2 uv) {
		tPos = xyz;
		tTexCoord = uv;
	}

	TerrainVertex() {
		tPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		tTexCoord = XMFLOAT2(0.0f, 0.0f);
	}

};

struct ConstantBuffer {
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};


int winMidX, winMidY;
int winLeft, winTop;
int curMouseXPos, curMouseYPos;
int clientMidX, clientMidY;
float rotFactorY = 0.0f;
float rotFactorX = 0.0f;

float camXPos = 0.0f;
float camYPos = 2.0f;
float camZPos = -50.0f;

XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

float camRotX = 0.0f;
float camRotY = 0.0f;

const float MOVE_STEP = 20.0f;
const float ROTATE_STEP = (XM_PI / 6);

BOOL InitialiseDirectX(HWND hMainWnd, HINSTANCE hCurInstance) {

	RECT rectDimensions;
	GetClientRect(hMainWnd, &rectDimensions);

	LONG width = rectDimensions.right - rectDimensions.left;
	LONG height = rectDimensions.bottom - rectDimensions.top;

	RECT rectWindow;
	GetWindowRect(hMainWnd, &rectWindow);

	winTop = rectWindow.top;
	winLeft = rectWindow.left;

	winMidX = (rectWindow.right - rectWindow.left) / 2;
	winMidY = (rectWindow.bottom - rectWindow.top) / 2;

	clientMidX = width / 2;
	clientMidY = height / 2;

	curMouseXPos = clientMidX;
	curMouseYPos = clientMidY;

	SetCursorPos(winLeft + winMidX, winTop + winMidY);

	rotFactorY = XM_PIDIV2 / width;

	D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1};

	int numLevels = sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hMainWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	int creationFlags = 0;

	HRESULT result;

	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, NULL, NULL, NULL, D3D11_SDK_VERSION,
		&swapChainDesc, &swapChain, &d3dDevice, NULL, &d3dContext);

	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to initialise DX11!"), className, NULL);

		return false;
	}

	ID3D11Texture2D *backBufferTexture;

	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&backBufferTexture);

	if (result != S_OK) {
		MessageBox(hMainWnd, L"Failed to get back buffer!", className, NULL);

		return false;
	}

	result = d3dDevice->CreateRenderTargetView(backBufferTexture, 0, &backBuffer);

	if (backBufferTexture != NULL) {
		backBufferTexture->Release();
	}

	if (result != S_OK) {
		MessageBox(hMainWnd, L"Failed to get render target!", className, NULL);

		return false;
	}

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	d3dDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);

	d3dDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	d3dContext->OMSetRenderTargets(1, &backBuffer, depthStencilView);

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	d3dContext->RSSetViewports(1, &viewport);


	XMVECTOR camPos = XMVectorSet(camXPos, camYPos, camZPos, 0.0f);
	matView = XMMatrixLookAtLH(camPos, camTarget, camUp);

	matProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2 / 2.0f, width / (FLOAT)height, 0.01f, 10000.0f);
	
	return true;
}

bool LoadFace(const wchar_t *filename, int faceIdx, ID3D11Texture2D *cubemapTex) {
	// Load the texture into a memory
	ID3D11ShaderResourceView *faceData;
	ID3D11Resource *faceRes;
	HRESULT hr = CreateWICTextureFromFile(d3dDevice, d3dContext, filename, &faceRes, &faceData, 0);

	if (hr != S_OK) {
		return false;
	}

	// Get the texture object from the shader resource loaded
	ID3D11Texture2D *tex;

	hr = faceRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID *)&tex);

	if (hr != S_OK) {
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	tex->GetDesc(&texDesc);

	// Copy texture from shader resource to cubemap face specified by faceIdx
	D3D11_BOX srcRegion;

	srcRegion.front = 0;
	srcRegion.back = 1;
	srcRegion.top = 0;
	srcRegion.left = 0;
	srcRegion.bottom = texDesc.Height;
	srcRegion.right = texDesc.Width;

	// Determine the subresource object corresponding to the face id
	int face = D3D11CalcSubresource(0, faceIdx, 1);
	d3dContext->CopySubresourceRegion(cubemapTex, face, 0, 0, 0, faceRes, 0, &srcRegion);

	// Release the face texture object
	faceData->Release();

	return true;
}

bool CreateCubemapSkybox(const wchar_t *up_fname, const wchar_t *down_fname, const wchar_t *left_fname, const wchar_t *right_fname, const wchar_t *front_fname, const wchar_t *back_fname, int lengthOfSide) {

	D3D11_TEXTURE2D_DESC cubemapDesc;
	ZeroMemory(&cubemapDesc, sizeof(D3D11_TEXTURE2D_DESC));

	cubemapDesc.Width = lengthOfSide;
	cubemapDesc.Height = lengthOfSide;
	cubemapDesc.MipLevels = 1;
	cubemapDesc.ArraySize = 6;
	cubemapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cubemapDesc.SampleDesc.Count = 1;
	cubemapDesc.SampleDesc.Quality = 0;
	cubemapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubemapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cubemapDesc.CPUAccessFlags = 0;
	cubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D *cubemapBuffer;

	HRESULT hr = d3dDevice->CreateTexture2D(&cubemapDesc, nullptr, &cubemapBuffer);

	if (hr != S_OK) {
		return false;
	}

	LoadFace(up_fname, 2, cubemapBuffer);
	LoadFace(down_fname, 3, cubemapBuffer);
	LoadFace(left_fname, 1, cubemapBuffer);
	LoadFace(right_fname, 0, cubemapBuffer);
	LoadFace(front_fname, 4, cubemapBuffer);
	LoadFace(back_fname, 5, cubemapBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	ZeroMemory(&srDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	srDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;

	hr = d3dDevice->CreateShaderResourceView(cubemapBuffer, &srDesc, &CubeMap);

	if (hr != S_OK) {
		return false;
	}

	cubemapBuffer->Release();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(XMFLOAT3) * ARRAYSIZE(skybox);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	InitData.pSysMem = skybox;
	hr = d3dDevice->CreateBuffer(&bd, &InitData, &skyboxVertexBuffer);

	if (hr != S_OK) {
		return false;
	}

	return true;
}

bool CreateShaders(HWND hMainWnd) {
	ID3DBlob *pVSBlob = NULL;
	HRESULT hr = D3DReadFileToBlob(L".\\SkyboxVertexShader.cso", &pVSBlob);
	if (hr != S_OK)
	{
		MessageBox(hMainWnd, L"Problem loading skybox vertex shader.  Check shader file (VertexShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", className, MB_OK);

		return false;
	}

	// Create the vertex shader in DirectX
	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &skyboxVertexShader);
	// Check creating vertex shader was successful
	if (hr != S_OK)
	{
		pVSBlob->Release();

		MessageBox(hMainWnd, L"The vertex shader object cannot be created", className, MB_OK);

		return false;
	}

	D3D11_INPUT_ELEMENT_DESC skybox_layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = 1;

	// Create the input layout for input assembler based on description and the vertex shader to be used
	hr = d3dDevice->CreateInputLayout(skybox_layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &skyboxVertexLayout);
	// Check layout was created successfully

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Problems creating input layout", className, MB_OK);

		return false;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = D3DReadFileToBlob(L".\\SkyboxPixelShader.cso", &pPSBlob);
	// Check pixel shader was loaded successfully
	if (hr != S_OK)
	{
		// If program has got this far then the problem is more likely to be in shader file, e.g. mistyped name, or wrong shader version
		MessageBox(hMainWnd, L"Problem loading pixel shader.  Check shader file (PixelShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", className, MB_OK);

		return false;
	}

	// Create the pixel shader in DirectX
	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &skyboxPixelShader);
	pPSBlob->Release();
	// Check creating pixel shader was successful
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"The pixel shader object cannot be created", className, MB_OK);

		return false;
	}

	hr = D3DReadFileToBlob(L".\\VertexShader.cso", &pVSBlob);
	if (hr != S_OK)
	{
		MessageBox(hMainWnd, L"Problem loading vertex shader.  Check shader file (VertexShader.cos) is in the project directory (sub folder of main solution directory) and shader is valid", className, MB_OK);

		return false;
	}

	// Create the vertex shader in DirectX
	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertexShader);
	// Check creating vertex shader was successful
	if (hr != S_OK)
	{
		pVSBlob->Release();

		MessageBox(hMainWnd, L"The vertex shader object cannot be created", className, MB_OK);

		return false;
	}

	// Need to tell DirectX how vertices are structured in terms of colour format and order of data, that is individual vertices
	// The POSITION is called the semantic name, basically a meaningful identifier used internally to map vertex elements to shader 
	// parameters.  Semantic name text must obey the rules of C identifiers as shader language uses C syntax
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	numElements = 2;

	// Create the input layout for input assembler based on description and the vertex shader to be used
	hr = d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &vertexLayout);
	// Check layout was created successfully
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Problems creating input layout", className, MB_OK);

		return false;
	}

	// Compile the pixel shader
	hr = D3DReadFileToBlob(L".\\PixelShader.cso", &pPSBlob);
	// Check pixel shader was loaded successfully
	if (hr != S_OK)
	{
		// If program has got this far then the problem is more likely to be in shader file, e.g. mistyped name, or wrong shader version
		MessageBox(hMainWnd, L"Problem loading pixel shader.  Check shader file (PixelShader.cso) is in the project directory (sub folder of main solution directory) and shader is valid", className, MB_OK);

		return false;
	}

	// Create the pixel shader in DirectX
	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixelShader);
	pPSBlob->Release();
	// Check creating pixel shader was successful
	if (hr != S_OK) {
		MessageBox(hMainWnd, L"The pixel shader object cannot be created", className, MB_OK);

		return false;
	}

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\grass.jpg", 0, &terrainTex, 0);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", className, MB_OK);

		return false;
	}

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\building6.jpg", 0, &building1Tex, 0);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", className, MB_OK);

		return false;
	} 

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\building4.jpg", 0, &building2Tex, 0);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", className, MB_OK);

		return false;
	}

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\car.jpg", 0, &car1Tex, 0);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", className, MB_OK);

		return false;
	}

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\street_lamp.jpg", 0, &lamp1Tex, 0);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", className, MB_OK);

		return false;
	}

	hr = CreateWICTextureFromFile(d3dDevice, d3dContext, L".\\assets\\tree.jpg", 0, &tree1Tex, 0);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Unable to load texture", className, MB_OK);

		return false;
	}

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = d3dDevice->CreateSamplerState(&sampDesc, &sampler);
	if (hr != S_OK) {
		MessageBox(hMainWnd, TEXT("Unable to create sampler"), className, MB_OK);

		return false;
	}

	// If we've got this far then have valid shaders.
	// Return a successful result
	return true;
}

void LoadHeightMap(const char *filename, float MaxHeight, TerrainVertex **mesh) {
	// This example expects a texture file of 256 pixels width and height
	// More flexible version would obtain the width and height from
	// file and create terrain from this data as terrain vertices
	// map onto pixel data
	// Also presumes height map is 24 bit texture file
	const int Size = 256;

	FILE *file_ptr = fopen(filename, "rb");

	if (file_ptr == NULL) {
		return;
	}

	// Read the file's header information
	// so we can create a buffer to hold the pixel colour data
	BITMAPFILEHEADER bmHeader;
	BITMAPINFOHEADER bmInfo;

	fread(&bmHeader, sizeof(BITMAPFILEHEADER), 1, file_ptr);

	fread(&bmInfo, sizeof(BITMAPINFOHEADER), 1, file_ptr);

	// Move file pointer to start of pixel colour data
	fseek(file_ptr, bmHeader.bfOffBits, SEEK_SET);

	// Work out number of bytes per pixel for array
	int NumBytesPerPixel = bmInfo.biBitCount / 8;

	// Calculate maximum colour value up 2^24 colour
	float MaxColourVal = (float)pow(2.0f, min(24.0f, (float)bmInfo.biBitCount));

	// Create array for pixel data
	unsigned char *pixel_data = new unsigned char[Size * Size * NumBytesPerPixel];

	// Read pixel data from file into memory
	fread(pixel_data, sizeof(unsigned char), Size * Size * NumBytesPerPixel, file_ptr);

	// Release the file
	fclose(file_ptr);

	//[13] Modify each vertex's y component based on the pixel value in proportion to max colour value
	// in the corresponding pixel
	int idx = 0;
	TerrainVertex *vertArray = *mesh;
	for (int row = 0; row < Size; row++) {
		for (int col = 0; col < Size; col++) {
			int pixel_val = pixel_data[idx * NumBytesPerPixel]
				+ (pixel_data[(idx * NumBytesPerPixel) + 1] << 8)
				+ (pixel_data[(idx * NumBytesPerPixel) + 2] << 16);

			vertArray[idx].tPos.y = MaxHeight * (pixel_val / MaxColourVal);

			idx++;
		}
	}

	// Release pixel data as don't need it any more
	delete[] pixel_data;
}

void CreateTerrainMesh(int NumVertices, TerrainVertex **mesh, int *TotalNumVert, int **indices, int *NumIndices) {

	*TotalNumVert = NumVertices * NumVertices;
	*mesh = new TerrainVertex[*TotalNumVert];

	float step = 2.0f;

	float xPos = -1.0f;
	float zPos = -1.0f;

	float uvStep = 1.0f / NumVertices;

	float u = 0.0f;
	float v = 0.0f;

	int idx = 0;
	TerrainVertex *vertArray = *mesh;

	for (int row = 0; row < NumVertices; row++) {
		for (int col = 0; col < NumVertices; col++) {
			vertArray[idx] = TerrainVertex(XMFLOAT3(xPos, 0.0f, zPos), XMFLOAT2(u, v));

			xPos += step;
			u += uvStep;
			idx++;
		
		}

		u = 0.0f;
		v += uvStep;
		xPos = -1.0f;
		zPos += step;
	}

	*NumIndices = (NumVertices - 1) * (NumVertices - 1) * 6;
	*indices = new int[*NumIndices];

	int *p = *indices;

	p[0] = 0;
	p[1] = NumVertices;
	p[2] = NumVertices + 1;
	p[3] = NumVertices + 1;
	p[4] = 1;
	p[5] = 0;

	int NextRow = (NumVertices - 1) * 6;
	for (int idx = 6; idx < *NumIndices; idx++) {

		if (idx % NextRow == 0) {
			p[idx] = (idx / NextRow) * NumVertices;
			p[idx + 1] = p[idx] + NumVertices;
			p[idx + 2] = p[idx + 1] + 1;
			p[idx + 3] = p[idx + 2];
			p[idx + 4] = p[idx] + 1;
			p[idx + 5] = p[idx];

			idx += 5;
		}
		else {
			p[idx] = p[idx - 6] + 1;
		}

	}

}

bool CreateMeshes(HWND hMainWnd) {
	TerrainVertex *terrainVertices;
	int *terrainIndices;
	int NumVert;

	CreateTerrainMesh(256, &terrainVertices, &NumVert, &terrainIndices, &NumIndices);
	LoadHeightMap(".\\assets\\heightmap.bmp", 10.0f, &terrainVertices);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(TerrainVertex) * NumVert;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// vertexBuffer is a global variable that points to the DirectX vertex buffer created for program
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = terrainVertices;

	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &terrainVertexBuffer);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create vertex buffer", className, NULL);

		return false;
	}

	//[17] Define the index buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(int) * NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));

	// Needs access to the triangle structure defined above
	InitData.pSysMem = terrainIndices;

	hr = d3dDevice->CreateBuffer(&bd, &InitData, &terrainIndexBuffer);

	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create index buffer", className, NULL);

		return false;
	}

	delete[] terrainVertices;
	delete[] terrainIndices;

	//[9] Create the constant buffer.  Similar to vertex buffer except no initial transfer of data
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = d3dDevice->CreateBuffer(&bd, NULL, &constantBuffer);


	if (hr != S_OK) {
		MessageBox(hMainWnd, L"Failed to create constant buffer", className, NULL);

		return false;
	}

	return true;
}

bool CreateBuilding1() {
	Assimp::Importer imp;

	const aiScene *pScene = imp.ReadFile(".\\assets\\building6.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs);

	const aiMesh *mesh = pScene->mMeshes[0];
	NumBuilding1Vertices = mesh->mNumFaces * 3;
	Vertex *shape = new Vertex[NumBuilding1Vertices];

	int vertCount = 0;

	for (int faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
		const aiFace &face = mesh->mFaces[faceIdx];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos = &mesh->mVertices[face.mIndices[vertIdx]];
			const aiVector3D *tex = &mesh->mTextureCoords[0][face.mIndices[vertIdx]];

			shape[vertCount] = Vertex(pos->x, pos->y, pos->z, tex->x, tex->y);
			vertCount++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) *NumBuilding1Vertices; 
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = shape;

	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &building1VertexBuffer);

	delete[] shape;

	return true;
}

bool CreateBuilding2() {
	Assimp::Importer imp1;

	const aiScene *pScene = imp1.ReadFile(".\\assets\\building4.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs); //4, tree, street_lamp, car, 

	const aiMesh *mesh1 = pScene->mMeshes[0];
	NumBuilding2Vertices = mesh1->mNumFaces * 3;
	Vertex *shape1 = new Vertex[NumBuilding2Vertices];

	int vertCount = 0;

	for (int faceIdx = 0; faceIdx < mesh1->mNumFaces; faceIdx++) {
		const aiFace &face = mesh1->mFaces[faceIdx];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos = &mesh1->mVertices[face.mIndices[vertIdx]];
			const aiVector3D *tex = &mesh1->mTextureCoords[0][face.mIndices[vertIdx]];

			shape1[vertCount] = Vertex(pos->x, pos->y, pos->z, tex->x, tex->y);
			vertCount++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) *NumBuilding2Vertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = shape1;

	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &building2VertexBuffer);

	delete[] shape1;

	return true;
}

bool CreateCar1() {
	Assimp::Importer imp2;

	const aiScene *pScene = imp2.ReadFile(".\\assets\\car.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs); //4, tree, street_lamp, car, 


	const aiMesh *mesh2 = pScene->mMeshes[0];
	NumCar1Vertices = mesh2->mNumFaces * 3;
	Vertex *shape2 = new Vertex[NumCar1Vertices];

	int vertCount = 0;

	for (int faceIdx = 0; faceIdx < mesh2->mNumFaces; faceIdx++) {
		const aiFace &face = mesh2->mFaces[faceIdx];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos = &mesh2->mVertices[face.mIndices[vertIdx]];
			const aiVector3D *tex = &mesh2->mTextureCoords[0][face.mIndices[vertIdx]];

			shape2[vertCount] = Vertex(pos->x, pos->y, pos->z, tex->x, tex->y);
			vertCount++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) *NumCar1Vertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = shape2;

	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &car1VertexBuffer);

	delete[] shape2;

	return true;
}

bool CreateTree1() {
	Assimp::Importer imp3;

	const aiScene *pScene = imp3.ReadFile(".\\assets\\tree.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs); //4, tree, street_lamp, car, 


	const aiMesh *mesh3 = pScene->mMeshes[0];
	NumTree1Vertices = mesh3->mNumFaces * 3;
	Vertex *shape3 = new Vertex[NumCar1Vertices];

	int vertCount = 0;

	for (int faceIdx = 0; faceIdx < mesh3->mNumFaces; faceIdx++) {
		const aiFace &face = mesh3->mFaces[faceIdx];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos = &mesh3->mVertices[face.mIndices[vertIdx]];
			const aiVector3D *tex = &mesh3->mTextureCoords[0][face.mIndices[vertIdx]];

			shape3[vertCount] = Vertex(pos->x, pos->y, pos->z, tex->x, tex->y);
			vertCount++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) *NumTree1Vertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = shape3;

	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &tree1VertexBuffer);

	delete[] shape3;

	return true;
}

bool CreateLamp1() {
	Assimp::Importer imp4;

	const aiScene *pScene = imp4.ReadFile(".\\assets\\street_lamp.obj", aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs); //4, tree, street_lamp, car, 


	const aiMesh *mesh4 = pScene->mMeshes[0];
	NumLamp1Vertices = mesh4->mNumFaces * 3;
	Vertex *shape4 = new Vertex[NumCar1Vertices];

	int vertCount = 0;

	for (int faceIdx = 0; faceIdx < mesh4->mNumFaces; faceIdx++) {
		const aiFace &face = mesh4->mFaces[faceIdx];

		for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
			const aiVector3D *pos = &mesh4->mVertices[face.mIndices[vertIdx]];
			const aiVector3D *tex = &mesh4->mTextureCoords[0][face.mIndices[vertIdx]];

			shape4[vertCount] = Vertex(pos->x, pos->y, pos->z, tex->x, tex->y);
			vertCount++;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) *NumLamp1Vertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = shape4;

	HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &lamp1VertexBuffer);

	delete[] shape4;

	return true;
}

bool SetRasterizerState(HWND hMainWnd) {
	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = true;

	HRESULT result = d3dDevice->CreateRasterizerState(&rasDesc, &rs);

	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), className, NULL);
	}

	return true;
}

void Update() {
	DWORD now = GetTickCount();
	DWORD diff = now - frameTime;

	// Store the calculated amount of movement
	// note that there is no rotation variable as this is calculated from mouse movement
	float forwards_backwards = 0.0f;
	float left_right = 0.0f;
	float up_down = 0.0f;

	// Calculate the amount of movement forwards and backwards
	if (IsKeyDown('W')) {
		forwards_backwards = MOVE_STEP * (diff / 1000.0f);
	}

	if (IsKeyDown('S')) {
		forwards_backwards = -MOVE_STEP * (diff / 1000.0f);
	}

	// Calculate the change mouse X pos
	int deltaX = clientMidX - curMouseXPos;

	int deltaY = clientMidY - curMouseYPos;

	// Update the rotation 
	camRotY -= deltaX * rotFactorY;
	camRotX -= deltaY * rotFactorX;

	// Reset cursor position to middle of window
	SetCursorPos(winLeft + winMidX, winTop + winMidY);

	XMVECTOR oldCamTarget = camTarget;
	float oldCamX = camXPos, oldCamY = camYPos, oldCamZ = camZPos;

	// Calculate distance to move camera with respect to movement on Z axis and rotation
	XMMATRIX camMove = XMMatrixTranslation(0.0f, 0.0f, forwards_backwards) * XMMatrixRotationRollPitchYaw(camRotX, camRotY, 0.0f);

	// Obtain the translation from the move matrix
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, camMove);

	// Update the camera position X, Y and Z
	camXPos += XMVectorGetX(trans);
	camYPos += XMVectorGetY(trans);
	camZPos += XMVectorGetZ(trans);

	XMVECTOR camPos = XMVectorSet(camXPos, camYPos, camZPos, 0.0f);

	// Calculate the relative distance from the camera to look at with respect to distance and rotation
	XMMATRIX camDist = XMMatrixTranslation(0.0f, 0.0f, 10.0f) *
		XMMatrixRotationRollPitchYaw(camRotX, camRotY, 0.0f);

	// Obtain the translation and calculate target
	XMMatrixDecompose(&scale, &rot, &trans, camDist);

	// Calculate a new target to look at with respect to the camera position
	camTarget = XMVectorSet(camXPos + XMVectorGetX(trans),
		camYPos + XMVectorGetY(trans),
		camZPos + XMVectorGetZ(trans), 0.0f);

	// Recalculate the view matrix based on the camera's new position and target vector
	matView = XMMatrixLookAtLH(camPos, camTarget, camUp);

	// Store time in order to calculate frame time in the next frame
	frameTime = now;
}

void Draw() {
	if (d3dContext == NULL) {
		return;
	}

	float colour[] = { 0.392156f, 0.584313f, 0.929411f, 1.0f };

	d3dContext->ClearRenderTargetView(backBuffer, colour);
	// Clear the depth of depthstencil buffer to 1.0f for new frame
	d3dContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Used to pass the calculated WVP matrices to the graphics pipeline constant buffer
	ConstantBuffer cb;

	//[10] Calculate WVP for skybox based on camera position
	matWorld = XMMatrixTranslation(camXPos, camYPos, camZPos);

	cb.world = XMMatrixTranspose(matWorld);
	cb.view = XMMatrixTranspose(matView);
	cb.projection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	// Select the WVP constant buffer into the graphics pipeline
	d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	//[11] Set the skybox shaders
	// Set up to render skybox, remember skybox has different shaders to
	// other models because it uses a cubemap
	d3dContext->VSSetShader(skyboxVertexShader, NULL, 0);
	d3dContext->PSSetShader(skyboxPixelShader, NULL, 0);
	d3dContext->PSSetSamplers(0, 1, &sampler);

	//d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//[12] Set the skybox cubemap texture
	d3dContext->PSSetShaderResources(1, 1, &CubeMap);

	//[13] Set the skybox input layout for input assembler
	d3dContext->IASetInputLayout(skyboxVertexLayout);

	//[14] Set the input layout for the skybox vertex shader
	// remember the skybox shader does not use the same layout
	// as a regular model as the xyz position is used to directly
	// select pixels from cubemap that is like a 3D texture
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	d3dContext->IASetVertexBuffers(0, 1, &skyboxVertexBuffer, &stride, &offset);

	//[15] Turn off the depth test as cubebox will be drawn at maximum depth
	d3dContext->OMSetDepthStencilState(DSDepthOff, 0);


	//[16] Turn off culling otherwise would not see inside of cube
	d3dContext->RSSetState(rsCullingOff);


	//[17] Draw skybox
	d3dContext->Draw(ARRAYSIZE(skybox), 0);


	//[18] Restore depth test and backface culling before drawing other objects
	d3dContext->RSSetState(rsCullingOn);
	d3dContext->OMSetDepthStencilState(DSDepthOn, 0);

	//d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->IASetInputLayout(vertexLayout);

	//d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/*d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);*/
	// -------------------------------------------------------------------------------
	//cb.view = XMMatrixTranspose(matView);
	//cb.projection = XMMatrixTranspose(matProjection);
	
	d3dContext->VSSetShader(vertexShader, NULL, 0);
	//d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	d3dContext->PSSetShader(pixelShader, NULL, 0);
	//d3dContext->PSSetShaderResources(1, 1, &imageData);

	//d3dContext->PSSetSamplers(0, 1, &sampler);

	//d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	matWorld = XMMatrixScaling(0.5f, 2.0f, 0.5f) * XMMatrixTranslation(-80, 0, -80);
	cb.world = XMMatrixTranspose(matWorld);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	//[21] Select the terrain vertex buffer
	UINT tStride = sizeof(TerrainVertex);
	UINT tOffset = 0;

	d3dContext->IASetVertexBuffers(0, 1, &terrainVertexBuffer, &tStride, &tOffset);


	//[22] Select the input buffer
	d3dContext->IASetIndexBuffer(terrainIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//[23] Set the image data from which the sampler will
	// map onto mesh pixels
	d3dContext->PSSetShaderResources(1, 1, &terrainTex);


	// Show in wireframe, comment out the following line to show solid
	//d3dContext->RSSetState(rs);

	//[24] Draw the terrain
	d3dContext->DrawIndexed(NumIndices, 0, 0);


	stride = sizeof(TerrainVertex);
	offset = 0;

	//MODEL 1 -----------------------

	matWorld = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 0.0f, 20.0f);
	cb.world = XMMatrixTranspose(matWorld);
	cb.view = XMMatrixTranspose(matView);
	cb.projection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &building1VertexBuffer, &stride, &offset);
	// Also tell input assembler how the vertices are to be treated.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &building1Tex);

	d3dContext->Draw(NumBuilding1Vertices, 0);

	// END OF MODEL 1 ---------------

	//MODEL 2 -----------------------

	matWorld = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(25.0f, 0.0f, 20.0f);
	cb.world = XMMatrixTranspose(matWorld);
	cb.view = XMMatrixTranspose(matView);
	cb.projection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &building2VertexBuffer, &stride, &offset);
	// Also tell input assembler how the vertices are to be treated.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &building2Tex);
	
	d3dContext->Draw(NumBuilding2Vertices, 0);

	// END OF MODEL 2 ---------------

	//MODEL 3 -----------------------

	matWorld = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(50.0f, 0.0f, 20.0f);
	cb.world = XMMatrixTranspose(matWorld);
	cb.view = XMMatrixTranspose(matView);
	cb.projection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &car1VertexBuffer, &stride, &offset);
	// Also tell input assembler how the vertices are to be treated.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &car1Tex);

	d3dContext->Draw(NumCar1Vertices, 0);

	// END OF MODEL 3 ---------------

	//MODEL 4 -----------------------

	matWorld = XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixTranslation(75.0f, 0.0f, 20.0f);
	cb.world = XMMatrixTranspose(matWorld);
	cb.view = XMMatrixTranspose(matView);
	cb.projection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &tree1VertexBuffer, &stride, &offset);
	// Also tell input assembler how the vertices are to be treated.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &tree1Tex);

	d3dContext->Draw(NumTree1Vertices, 0);

	// END OF MODEL 4 ---------------

	//MODEL 5 -----------------------

	matWorld = XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(100.0f, 0.0f, 20.0f);
	cb.world = XMMatrixTranspose(matWorld);
	cb.view = XMMatrixTranspose(matView);
	cb.projection = XMMatrixTranspose(matProjection);
	d3dContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	d3dContext->IASetVertexBuffers(0, 1, &lamp1VertexBuffer, &stride, &offset);
	// Also tell input assembler how the vertices are to be treated.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dContext->PSSetShaderResources(1, 1, &lamp1Tex);

	d3dContext->Draw(NumTree1Vertices, 0);

	// END OF MODEL 5 ---------------
	
	// Display frame immediately
	swapChain->Present(0, 0);
	// -------------------------------------------------------------------------------

}

void ShutdownDirectX() {
	//[20] Release skybox objects
	if (terrainIndexBuffer) {
		terrainIndexBuffer->Release();
	}

	if (terrainVertexBuffer) {
		terrainVertexBuffer->Release();
	}
	
	if (CubeMap) {
		CubeMap->Release();
	}

	if (rsCullingOn) {
		rsCullingOn->Release();
	}

	if (rsCullingOff) {
		rsCullingOff->Release();
	}

	if (DSDepthOn) {
		DSDepthOn->Release();
	}

	if (DSDepthOff) {
		DSDepthOff->Release();
	}

	if (skyboxVertexBuffer) {
		skyboxVertexBuffer->Release();
	}

	if (skyboxVertexLayout) {
		skyboxVertexLayout->Release();
	}

	if (skyboxPixelShader) {
		skyboxPixelShader->Release();
	}

	if (skyboxVertexShader) {
		skyboxVertexShader->Release();
	}

	if (imageData) {
		imageData->Release();
	}

	if (sampler) {
		sampler->Release();
	}

	if (terrainTex)
		terrainTex->Release();

	if (vertexShader != NULL)
		vertexShader->Release();

	if (pixelShader != NULL)
		pixelShader->Release();


	if (vertexBuffer != NULL)
		vertexBuffer->Release();

	if (skyboxVertexShader != NULL)
		skyboxVertexShader->Release();

	if (skyboxPixelShader != NULL)
		skyboxPixelShader->Release();

	if (vertexLayout != NULL)
		vertexLayout->Release();

	if (backBuffer) {
		backBuffer->Release();
	}

	if (depthStencilView) {
		depthStencilView->Release();
	}

	if (depthStencilBuffer) {
		depthStencilBuffer->Release();
	}

	if (swapChain) {
		swapChain->Release();
	}

	if (d3dContext) {
		d3dContext->Release();
	}

	if (d3dDevice) {
		d3dDevice->Release();
	}

	backBuffer = 0;
	swapChain = 0;
	d3dContext = 0;
	d3dDevice = 0;
}

bool CreateCullingRasterizersOnDepthTestStates(HWND hMainWnd) {
	// Define a rasterizer that turns of culling and fills using solid mode
	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = true;

	// Create the rasterizer
	HRESULT result = d3dDevice->CreateRasterizerState(&rasDesc, &rsCullingOff);
	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), className, NULL);
	}

	// Now create a rasterizer that turns culling on
	ZeroMemory(&rasDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_BACK;
	result = d3dDevice->CreateRasterizerState(&rasDesc, &rsCullingOn);
	if (result != S_OK) {
		MessageBox(hMainWnd, TEXT("Failed to create rasterizer state"), className, NULL);
	}

	// Create depth stencil object for disabling depth test
	// used when rendering skybox
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = false;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	d3dDevice->CreateDepthStencilState(&dssDesc, &DSDepthOff);

	// Create depth stencil object for enable depth test
	// used when rendering everything else
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;

	d3dDevice->CreateDepthStencilState(&dssDesc, &DSDepthOn);

	return true;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLineArgs, int nInitialWinShowState) {
	HRESULT hrInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Check initialised ok, if not then exit program as may get spurious errors
	// such as memory access violations if not initialised
	if (hrInit != S_OK) {
		MessageBox(NULL, L"Unable to initialise for DirectXTK WIC", className, 0);

		return 0;
	}
	
	HWND mWnd;
	MSG nMsg = { 0 };
	WNDCLASS wClass;
	wchar_t fps[64];
	ZeroMemory(fps, 64);

	wClass.style = CS_HREDRAW | CS_VREDRAW;
	wClass.lpfnWndProc = WinProc;
	wClass.cbClsExtra = 0;
	wClass.cbWndExtra = 0;
	wClass.hInstance = hInstance;
	wClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wClass.lpszMenuName = NULL;
	wClass.lpszClassName = className;

	if (!RegisterClass(&wClass)) {
		MessageBox(NULL, L"Unable to register class for application", className, 0);

		return 0;
	}

	// Find out what CreateWindows does
	mWnd = CreateWindow(className,
		L"Direct X Progress Demonstration",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!mWnd) {
		MessageBox(NULL, L"Unable able to create the application's main window", className, 0);

		return 0;
	}

	ShowWindow(mWnd, nInitialWinShowState);

	if (!InitialiseDirectX(mWnd, hInstance)) {
		MessageBox(NULL, L"Failed to initialise DirectX", className, 0);

		return 0;
	}

	// Create shaders
	if (!CreateShaders(mWnd)) {
		// Exit program if an error occurred
		return 0;
	}

	// Create the vertices of the triangle in DirectX
	if (!CreateMeshes(mWnd)) {
		// Exit program if an error occurred
		return 0;
	}

	//[19] Create the skybox
	if (!CreateCubemapSkybox(L".\\assets\\skybox_up.jpg", L".\\assets\\skybox_down.jpg", L".\\assets\\skybox_left.jpg", L".\\assets\\skybox_right.jpg", L".\\assets\\skybox_front.jpg", L".\\assets\\skybox_back.jpg", 512)) {
		return 0;
	}


	if (!CreateBuilding1()) {
		return 0;
	}

	if (!CreateBuilding2()) {
		return 0;
	}

	if (!CreateCar1()) {
		return 0;
	}

	if (!CreateTree1()) {
		return 0;
	}

	if (!CreateLamp1()) {
		return 0;
	}
	// Exit program if an error occurred


	// Create the rasterizers and depth states for the skybox rendering
	CreateCullingRasterizersOnDepthTestStates(mWnd);

	// Slightly different message loop
	DWORD current = GetTickCount();

	frameTime = GetTickCount();

	int count = 0;

	while (nMsg.message != WM_QUIT) {
		if (PeekMessage(&nMsg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&nMsg);
			DispatchMessage(&nMsg);
		}
		else {
			Update();

			Draw();

			count++;

			DWORD now = GetTickCount();
			if (now - current > 1000) {
				wsprintf(fps, L"FPS = %d", count);

				SetWindowText(mWnd, fps);

				count = 0;

				current = now;
			}

		}
	}

	ShutdownDirectX();

	return 0;
}

LRESULT CALLBACK WinProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {


	case WM_KEYDOWN:
		ProcessKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		ProcessKeyUp(wParam);
		return 0;


	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_MOUSEMOVE:
		curMouseXPos = LOWORD(lParam);
		curMouseYPos = HIWORD(lParam);

		return 0;

	}

	return DefWindowProc(wnd, msg, wParam, lParam);
}

