//	DX11Renderer - VDemo | DirectX11 Renderer
//	Copyright(C) 2016  - Volkan Ilbeyli
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.
//
//	Contact: volkanilbeyli@gmail.com

#pragma once

#include "RenderingEnums.h"

#include <d3dcompiler.h>

#include <string>
#include <array>
#include <tuple>
#include <vector>
#include <stack>

// CONSTANT BUFFER STRUCTS/ENUMS
//------------------------------------------------------------------
// Current limitations: 
//  todo: revise this
//  - cbuffers with same names in different shaders (PS/VS/GS/...)
//  - cbuffers with same names in the same shader (not tested)


struct ConstantBufferLayout
{	// information used to create GPU/CPU constant buffers
	D3D11_SHADER_BUFFER_DESC					desc;
	std::vector<D3D11_SHADER_VARIABLE_DESC>		variables;
	std::vector<D3D11_SHADER_TYPE_DESC>			types;
	unsigned									buffSize;
	EShaderType									shdType;
	unsigned									bufSlot;
};



// Cached (CPU) constant buffer
// -------------------------------------------------------------------
#define EQUALITY_OPTIMIZED
constexpr size_t MAX_CONSTANT_BUFFERS = 512;

using CPUConstantID = int;

struct CPUConstant
{
	using CPUConstantPool = std::array<CPUConstant, MAX_CONSTANT_BUFFERS>;
	using CPUConstantRefIDPair = std::tuple<CPUConstant&, CPUConstantID>;

	friend class Renderer;
	friend class Shader;
	
	inline static	CPUConstant&			Get(int id) { return s_constants[id]; }
	static			CPUConstantRefIDPair	GetNextAvailable();
	static			void					CleanUp();	// call once

	CPUConstant() : _name(), _size(0), _data(nullptr) {}
	std::string _name;
	size_t		_size;
	void*		_data;

private:
	static CPUConstantPool	s_constants;
	static size_t			s_nextConstIndex;

#ifdef EQUALITY_OPTIMIZED
	inline bool operator==(const CPUConstant& c) const { return (((this->_data == c._data) && this->_size == c._size) && this->_name == c._name); }
	inline bool operator!=(const CPUConstant& c) const { return ((this->_data != c._data) || this->_size != c._size || this->_name != c._name); }
#endif
};

// GPU side constant buffer
struct ConstantBuffer
{
	EShaderType shdType;
	unsigned	bufferSlot;
	ID3D11Buffer* data;
	bool dirty;
};
//-------------------------------------------------------------

// SHADER RESOURCE MANAGEMENT
//-------------------------------------------------------------
struct ShaderTexture
{
	std::string name;
	unsigned bufferSlot;
	EShaderType shdType;
};


struct ShaderSampler
{
	std::string name;
	unsigned bufferSlot;
	EShaderType shdType;
};
//-------------------------------------------------------------

struct InputLayout
{
	std::string		semanticName;
	ELayoutFormat	format;
};

using GPU_ConstantBufferSlotIndex = int;
using ConstantBufferMapping = std::pair<GPU_ConstantBufferSlotIndex, CPUConstantID>;
using ShaderArray = std::array<ShaderID, EShaders::SHADER_COUNT>;

class Shader
{
	friend class Renderer;
public:
	static const ShaderArray&		Get() { return s_shaders; }
	static void						LoadShaders(Renderer* pRenderer);
	static std::stack<std::string>	UnloadShaders(Renderer* pRenderer);
	static void						ReloadShaders(Renderer* pRenderer);
	static bool						IsForwardPassShader(ShaderID shader);
	
	Shader(const std::string& shaderFileName);
	~Shader();

	void ClearConstantBuffers();
	void UpdateConstants(ID3D11DeviceContext* context);

	const std::string&							Name() const;
	ShaderID									ID() const;
	const std::vector<ConstantBufferLayout>&	GetConstantBufferLayouts() const;
	const std::vector<ConstantBuffer>&			GetConstantBuffers() const;

private:
	void RegisterConstantBufferLayout(ID3D11ShaderReflection * sRefl, EShaderType type);
	void CompileShaders(ID3D11Device* device, const std::vector<std::string>& filePaths, const std::vector<InputLayout>& layouts);
	void SetReflections(ID3D10Blob* vsBlob, ID3D10Blob* psBlob, ID3D10Blob* gsBlob);
	void CheckSignatures();
	void SetConstantBuffers(ID3D11Device* device);
	
	ID3D10Blob* Compile(const std::string& filename, const EShaderType& type);

	void LogConstantBufferLayouts() const;

private:
	static ShaderArray s_shaders;

	const std::string					m_name;
	std::vector<ConstantBuffer>			m_cBuffers;	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb509581(v=vs.85).aspx
	ShaderID							m_id;

	ID3D11VertexShader*					m_vertexShader;
	ID3D11PixelShader*					m_pixelShader;
	ID3D11GeometryShader*				m_geometryShader;
	ID3D11HullShader*					m_hullShader;
	ID3D11DomainShader*					m_domainShader;
	ID3D11ComputeShader*				m_computeShader;

	ID3D11ShaderReflection*				m_vsRefl;	// shader reflections, temporary?
	ID3D11ShaderReflection*				m_psRefl;	// shader reflections, temporary?
	ID3D11ShaderReflection*				m_gsRefl;	// shader reflections, temporary?

	ID3D11InputLayout*					m_layout;

	std::vector<ConstantBufferLayout>	m_CBLayouts;
	std::vector<ConstantBufferMapping>	m_constants;
	std::vector<ConstantBufferMapping>	m_constantsUnsorted;	// duplicate data...

	std::vector<ShaderTexture>			m_textures;
	std::vector<ShaderSampler>			m_samplers;
	
};

