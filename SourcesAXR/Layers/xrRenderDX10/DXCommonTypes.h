#pragma once

class	dx10State;

typedef	ID3D10Include			ID3DInclude;
typedef	ID3D10Blob				ID3DBlob;

typedef	D3D10_SHADER_MACRO				D3D_SHADER_MACRO;

typedef	D3D11_BLEND						D3D_BLEND;
typedef	D3D11_BLEND_OP					D3D_BLEND_OP;
typedef	D3D11_TEXTURE_ADDRESS_MODE		D3D_TEXTURE_ADDRESS_MODE;
typedef D3D11_COMPARISON_FUNC			D3D_COMPARISON_FUNC;
typedef D3D11_STENCIL_OP				D3D_STENCIL_OP;
typedef D3D11_CULL_MODE					D3D_CULL_MODE;
typedef	D3D11_FILTER					D3D_FILTER;

typedef D3D11_SAMPLER_DESC				D3D_SAMPLER_DESC;
typedef D3D11_RASTERIZER_DESC			D3D_RASTERIZER_DESC;
typedef D3D11_DEPTH_STENCIL_DESC		D3D_DEPTH_STENCIL_DESC;
typedef D3D11_BLEND_DESC				D3D_BLEND_DESC;

typedef D3D11_SUBRESOURCE_DATA			D3D_SUBRESOURCE_DATA;

typedef	D3D11_TEXTURE1D_DESC				D3D_TEXTURE1D_DESC;
typedef D3D11_TEXTURE2D_DESC				D3D_TEXTURE2D_DESC;
typedef D3D11_TEXTURE3D_DESC				D3D_TEXTURE3D_DESC;
typedef D3D11_BUFFER_DESC					D3D_BUFFER_DESC;
typedef D3D11_QUERY_DESC					D3D_QUERY_DESC;
typedef D3D11_SHADER_RESOURCE_VIEW_DESC		D3D_SHADER_RESOURCE_VIEW_DESC;
typedef	D3D11_RENDER_TARGET_VIEW_DESC		D3D_RENDER_TARGET_VIEW_DESC;
typedef	D3D11_DEPTH_STENCIL_VIEW_DESC		D3D_DEPTH_STENCIL_VIEW_DESC;
typedef D3D11_MAPPED_SUBRESOURCE			D3D_MAPPED_TEXTURE2D;
typedef D3D11_MAPPED_SUBRESOURCE			D3D_MAPPED_TEXTURE3D;
typedef D3D11_INPUT_ELEMENT_DESC			D3D_INPUT_ELEMENT_DESC;
typedef D3D11_PRIMITIVE_TOPOLOGY			D3D_PRIMITIVE_TOPOLOGY;
typedef	D3D11_VIEWPORT						D3D_VIEWPORT;
typedef	D3D11_USAGE							D3D_USAGE;

typedef ID3D11RasterizerState					ID3DRasterizerState;
typedef ID3D11DepthStencilState					ID3DDepthStencilState;
typedef ID3D11BlendState						ID3DBlendState;
typedef ID3D11SamplerState						ID3DSamplerState;
typedef ID3D11Buffer							ID3DBuffer;
typedef ID3D11Buffer							ID3DVertexBuffer;
typedef ID3D11Buffer							ID3DIndexBuffer;
typedef	ID3D11InputLayout						ID3DInputLayout;
typedef	ID3D11VertexShader						ID3DVertexShader;
typedef	ID3D11GeometryShader					ID3DGeometryShader;
typedef	ID3D11PixelShader						ID3DPixelShader;
typedef	ID3D11Texture1D							ID3DTexture1D;
typedef	ID3D11Texture2D							ID3DTexture2D;
typedef	ID3D11Texture3D							ID3DTexture3D;
typedef	ID3D11Resource							ID3DBaseTexture;
typedef ID3D11Resource							ID3DResource;
typedef	ID3D11RenderTargetView					ID3DRenderTargetView;
typedef	ID3D11DepthStencilView					ID3DDepthStencilView;
typedef	ID3D11ShaderResourceView				ID3DShaderResourceView;
typedef	ID3D11Query								ID3DQuery;
typedef	ID3D11Device							ID3DDevice;
typedef ID3D11DeviceContext						ID3DDeviceContext;

#define D3D_COMMONSHADER_SAMPLER_SLOT_COUNT		D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT

#define	D3D_REQ_TEXTURE2D_U_OR_V_DIMENSION		D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION

#define	D3D_STANDARD_MULTISAMPLE_PATTERN		D3D11_STANDARD_MULTISAMPLE_PATTERN

//#define	D3D_DRIVER_TYPE_REFERENCE				D3D11_DRIVER_TYPE_REFERENCE
//#define	D3D_DRIVER_TYPE_HARDWARE				D3D11_DRIVER_TYPE_HARDWARE

#define	D3D_PRIMITIVE_TOPOLOGY_UNDEFINED		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED
#define	D3D_PRIMITIVE_TOPOLOGY_POINTLIST		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
#define	D3D_PRIMITIVE_TOPOLOGY_LINELIST			D3D11_PRIMITIVE_TOPOLOGY_LINELIST
#define	D3D_PRIMITIVE_TOPOLOGY_LINESTRIP		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
#define	D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
#define	D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
#define	D3D_PRIMITIVE_TOPOLOGY_UNDEFINED		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED

#define	D3D_USAGE_DEFAULT		D3D11_USAGE_DEFAULT
#define	D3D_USAGE_DYNAMIC		D3D11_USAGE_DYNAMIC
#define	D3D_USAGE_STAGING		D3D11_USAGE_STAGING
#define	D3D_USAGE_IMMUTABLE		D3D11_USAGE_IMMUTABLE

#define	D3D_CULL_NONE		D3D11_CULL_NONE
#define	D3D_CULL_FRONT		D3D11_CULL_FRONT
#define	D3D_CULL_BACK		D3D11_CULL_BACK

#define	D3D_COMPARISON_NEVER			D3D11_COMPARISON_NEVER
#define	D3D_COMPARISON_LESS				D3D11_COMPARISON_LESS
#define	D3D_COMPARISON_EQUAL			D3D11_COMPARISON_EQUAL
#define	D3D_COMPARISON_LESS_EQUAL		D3D11_COMPARISON_LESS_EQUAL
#define	D3D_COMPARISON_GREATER			D3D11_COMPARISON_GREATER
#define	D3D_COMPARISON_NOT_EQUAL		D3D11_COMPARISON_NOT_EQUAL
#define	D3D_COMPARISON_GREATER_EQUAL	D3D11_COMPARISON_GREATER_EQUAL
#define	D3D_COMPARISON_ALWAYS			D3D11_COMPARISON_ALWAYS

#define	D3D_STENCIL_OP_KEEP			D3D11_STENCIL_OP_KEEP
#define	D3D_STENCIL_OP_ZERO			D3D11_STENCIL_OP_ZERO
#define	D3D_STENCIL_OP_REPLACE		D3D11_STENCIL_OP_REPLACE
#define	D3D_STENCIL_OP_INCR_SAT		D3D11_STENCIL_OP_INCR_SAT
#define	D3D_STENCIL_OP_DECR_SAT		D3D11_STENCIL_OP_DECR_SAT
#define	D3D_STENCIL_OP_INVERT		D3D11_STENCIL_OP_INVERT
#define	D3D_STENCIL_OP_INCR			D3D11_STENCIL_OP_INCR
#define	D3D_STENCIL_OP_DECR			D3D11_STENCIL_OP_DECR

#define	D3D_BLEND_ZERO					D3D11_BLEND_ZERO
#define	D3D_BLEND_ONE					D3D11_BLEND_ONE
#define	D3D_BLEND_SRC_COLOR				D3D11_BLEND_SRC_COLOR
#define	D3D_BLEND_INV_SRC_COLOR			D3D11_BLEND_INV_SRC_COLOR
#define	D3D_BLEND_SRC_ALPHA				D3D11_BLEND_SRC_ALPHA
#define	D3D_BLEND_INV_SRC_ALPHA			D3D11_BLEND_INV_SRC_ALPHA
#define	D3D_BLEND_DEST_ALPHA			D3D11_BLEND_DEST_ALPHA
#define	D3D_BLEND_INV_DEST_ALPHA		D3D11_BLEND_INV_DEST_ALPHA
#define	D3D_BLEND_DEST_COLOR			D3D11_BLEND_DEST_COLOR
#define	D3D_BLEND_INV_DEST_COLOR		D3D11_BLEND_INV_DEST_COLOR
#define	D3D_BLEND_SRC_ALPHA_SAT			D3D11_BLEND_SRC_ALPHA_SAT
#define	D3D_BLEND_BLEND_FACTOR			D3D11_BLEND_BLEND_FACTOR
#define	D3D_BLEND_INV_BLEND_FACTOR		D3D11_BLEND_INV_BLEND_FACTOR
#define	D3D_BLEND_SRC1_COLOR			D3D11_BLEND_SRC1_COLOR
#define	D3D_BLEND_INV_SRC1_COLOR		D3D11_BLEND_INV_SRC1_COLOR
#define	D3D_BLEND_SRC1_ALPHA			D3D11_BLEND_SRC1_ALPHA
#define	D3D_BLEND_INV_SRC1_ALPHA		D3D11_BLEND_INV_SRC1_ALPHA

#define	D3D_BLEND_OP_ADD				D3D11_BLEND_OP_ADD
#define	D3D_BLEND_OP_SUBTRACT			D3D11_BLEND_OP_SUBTRACT
#define	D3D_BLEND_OP_REV_SUBTRACT		D3D11_BLEND_OP_REV_SUBTRACT
#define	D3D_BLEND_OP_MIN				D3D11_BLEND_OP_MIN
#define	D3D_BLEND_OP_MAX				D3D11_BLEND_OP_MAX

#define	D3D_TEXTURE_ADDRESS_WRAP		D3D11_TEXTURE_ADDRESS_WRAP
#define	D3D_TEXTURE_ADDRESS_MIRROR		D3D11_TEXTURE_ADDRESS_MIRROR
#define	D3D_TEXTURE_ADDRESS_CLAMP		D3D11_TEXTURE_ADDRESS_CLAMP
#define	D3D_TEXTURE_ADDRESS_BORDER		D3D11_TEXTURE_ADDRESS_BORDER
#define	D3D_TEXTURE_ADDRESS_MIRROR_ONCE	D3D11_TEXTURE_ADDRESS_MIRROR_ONCE

#define	D3D_DEPTH_WRITE_MASK_ALL		D3D11_DEPTH_WRITE_MASK_ALL
#define	D3D_DEPTH_WRITE_MASK_ZERO		D3D11_DEPTH_WRITE_MASK_ZERO
#define	D3D_COLOR_WRITE_ENABLE_ALL		D3D11_COLOR_WRITE_ENABLE_ALL

#define	D3D_FILTER_ANISOTROPIC				D3D11_FILTER_ANISOTROPIC
#define	D3D_FILTER_MIN_MAG_MIP_LINEAR		D3D11_FILTER_MIN_MAG_MIP_LINEAR
#define	D3D_FILTER_COMPARISON_ANISOTROPIC	D3D11_FILTER_COMPARISON_ANISOTROPIC

#define	D3D_FILL_SOLID		D3D11_FILL_SOLID
#define	D3D_FILL_WIREFRAME	D3D11_FILL_WIREFRAME

#define	D3D_INPUT_PER_VERTEX_DATA	D3D11_INPUT_PER_VERTEX_DATA

#define	D3D_BIND_INDEX_BUFFER			D3D11_BIND_INDEX_BUFFER
#define	D3D_BIND_VERTEX_BUFFER			D3D11_BIND_VERTEX_BUFFER
#define	D3D_BIND_CONSTANT_BUFFER		D3D11_BIND_CONSTANT_BUFFER
#define	D3D_BIND_SHADER_RESOURCE		D3D11_BIND_SHADER_RESOURCE
#define	D3D_BIND_DEPTH_STENCIL			D3D11_BIND_DEPTH_STENCIL
#define	D3D_BIND_STREAM_OUTPUT			D3D11_BIND_STREAM_OUTPUT
#define	D3D_BIND_RENDER_TARGET			D3D11_BIND_RENDER_TARGET
#define	D3D_BIND_DEPTH_STENCIL			D3D11_BIND_DEPTH_STENCIL
#define	D3D_BIND_UNORDERED_ACCESS		D3D11_BIND_UNORDERED_ACCESS

#define	D3D_CPU_ACCESS_WRITE		D3D11_CPU_ACCESS_WRITE
#define	D3D_CPU_ACCESS_READ			D3D11_CPU_ACCESS_READ

#define	D3D_MAP_READ					D3D11_MAP_READ
#define	D3D_MAP_WRITE					D3D11_MAP_WRITE
#define	D3D_MAP_READ_WRITE				D3D11_MAP_READ_WRITE
#define	D3D_MAP_WRITE_DISCARD			D3D11_MAP_WRITE_DISCARD
#define	D3D_MAP_WRITE_NO_OVERWRITE		D3D11_MAP_WRITE_NO_OVERWRITE
typedef	D3D11_MAP	D3D_MAP;

#define	D3D_RESOURCE_MISC_GENERATE_MIPS				D3D11_RESOURCE_MISC_GENERATE_MIPS
#define	D3D_RESOURCE_MISC_SHARED					D3D11_RESOURCE_MISC_SHARED
#define	D3D_RESOURCE_MISC_TEXTURECUBE				D3D11_RESOURCE_MISC_TEXTURECUBE
#define	D3D_RESOURCE_MISC_DRAWINDIRECT_ARGS			D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS
#define	D3D_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS	D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS
#define	D3D_RESOURCE_MISC_BUFFER_STRUCTURED			D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
#define	D3D_RESOURCE_MISC_RESOURCE_CLAMP			D3D11_RESOURCE_MISC_RESOURCE_CLAMP
#define	D3D_RESOURCE_MISC_SHARED_KEYEDMUTEX			D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
#define	D3D_RESOURCE_MISC_GDI_COMPATIBLE			D3D11_RESOURCE_MISC_GDI_COMPATIBLE

#define	D3D_CLEAR_DEPTH		D3D11_CLEAR_DEPTH
#define	D3D_CLEAR_STENCIL	D3D11_CLEAR_STENCIL

#define	D3D_QUERY_EVENT							D3D11_QUERY_EVENT
#define	D3D_QUERY_OCCLUSION						D3D11_QUERY_OCCLUSION
#define	D3D_QUERY_TIMESTAMP						D3D11_QUERY_TIMESTAMP
#define	D3D_QUERY_TIMESTAMP_DISJOINT			D3D11_QUERY_TIMESTAMP_DISJOINT
#define	D3D_QUERY_PIPELINE_STATISTICS			D3D11_QUERY_PIPELINE_STATISTICS
#define	D3D_QUERY_OCCLUSION_PREDICATE			D3D11_QUERY_OCCLUSION_PREDICATE
#define	D3D_QUERY_SO_STATISTICS					D3D11_QUERY_SO_STATISTICS
#define	D3D_QUERY_SO_OVERFLOW_PREDICATE			D3D11_QUERY_SO_OVERFLOW_PREDICATE
#define	D3D_QUERY_SO_STATISTICS_STREAM0			D3D11_QUERY_SO_STATISTICS_STREAM0
#define	D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM0	D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0
#define	D3D_QUERY_SO_STATISTICS_STREAM1			D3D11_QUERY_SO_STATISTICS_STREAM1
#define	D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM1	D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1
#define	D3D_QUERY_SO_STATISTICS_STREAM2			D3D11_QUERY_SO_STATISTICS_STREAM2
#define	D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM2	D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2
#define	D3D_QUERY_SO_STATISTICS_STREAM3			D3D11_QUERY_SO_STATISTICS_STREAM3
#define	D3D_QUERY_SO_OVERFLOW_PREDICATE_STREAM3	D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3

#define	D3D_RESOURCE_DIMENSION_UNKNOWN			D3D11_RESOURCE_DIMENSION_UNKNOWN
#define	D3D_RESOURCE_DIMENSION_BUFFER			D3D11_RESOURCE_DIMENSION_BUFFER
#define	D3D_RESOURCE_DIMENSION_TEXTURE1D		D3D11_RESOURCE_DIMENSION_TEXTURE1D
#define	D3D_RESOURCE_DIMENSION_TEXTURE2D		D3D11_RESOURCE_DIMENSION_TEXTURE2D
#define	D3D_RESOURCE_DIMENSION_TEXTURE3D		D3D11_RESOURCE_DIMENSION_TEXTURE3D
typedef D3D11_RESOURCE_DIMENSION	D3D_RESOURCE_DIMENSION;

#define	D3D_SRV_DIMENSION_UNKNOWN				D3D11_SRV_DIMENSION_UNKNOWN
#define	D3D_SRV_DIMENSION_BUFFER				D3D11_SRV_DIMENSION_BUFFER
#define	D3D_SRV_DIMENSION_TEXTURE1D				D3D11_SRV_DIMENSION_TEXTURE1D
#define	D3D_SRV_DIMENSION_TEXTURE1DARRAY		D3D11_SRV_DIMENSION_TEXTURE1DARRAY
#define	D3D_SRV_DIMENSION_TEXTURE2D				D3D11_SRV_DIMENSION_TEXTURE2D
#define	D3D_SRV_DIMENSION_TEXTURE2DARRAY		D3D11_SRV_DIMENSION_TEXTURE2DARRAY
#define	D3D_SRV_DIMENSION_TEXTURE2DMS			D3D11_SRV_DIMENSION_TEXTURE2DMS
#define	D3D_SRV_DIMENSION_TEXTURE2DMSARRAY		D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY
#define	D3D_SRV_DIMENSION_TEXTURE3D				D3D11_SRV_DIMENSION_TEXTURE3D
#define	D3D_SRV_DIMENSION_TEXTURECUBE			D3D11_SRV_DIMENSION_TEXTURECUBE
#define	D3D_SRV_DIMENSION_TEXTURECUBEARRAY		D3D11_SRV_DIMENSION_TEXTURECUBEARRAY
#define	D3D_SRV_DIMENSION_BUFFEREX				D3D11_SRV_DIMENSION_BUFFEREX

#define	D3D_DSV_DIMENSION_UNKNOWN				D3D11_DSV_DIMENSION_UNKNOWN
#define	D3D_DSV_DIMENSION_TEXTURE1D				D3D11_DSV_DIMENSION_TEXTURE1D
#define	D3D_DSV_DIMENSION_TEXTURE1DARRAY		D3D11_DSV_DIMENSION_TEXTURE1DARRAY
#define	D3D_DSV_DIMENSION_TEXTURE2D				D3D11_DSV_DIMENSION_TEXTURE2D
#define	D3D_DSV_DIMENSION_TEXTURE2DARRAY		D3D11_DSV_DIMENSION_TEXTURE2DARRAY
#define	D3D_DSV_DIMENSION_TEXTURE2DMS			D3D11_DSV_DIMENSION_TEXTURE2DMS
#define	D3D_DSV_DIMENSION_TEXTURE2DMSARRAY		D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY

#define	D3D_RTV_DIMENSION_UNKNOWN				D3D11_RTV_DIMENSION_UNKNOWN
#define	D3D_RTV_DIMENSION_BUFFER				D3D11_RTV_DIMENSION_BUFFER
#define	D3D_RTV_DIMENSION_TEXTURE1D				D3D11_RTV_DIMENSION_TEXTURE1D
#define	D3D_RTV_DIMENSION_TEXTURE1DARRAY		D3D11_RTV_DIMENSION_TEXTURE1DARRAY
#define	D3D_RTV_DIMENSION_TEXTURE2D				D3D11_RTV_DIMENSION_TEXTURE2D
#define	D3D_RTV_DIMENSION_TEXTURE2DARRAY		D3D11_RTV_DIMENSION_TEXTURE2DARRAY
#define	D3D_RTV_DIMENSION_TEXTURE2DMS			D3D11_RTV_DIMENSION_TEXTURE2DMS
#define	D3D_RTV_DIMENSION_TEXTURE2DMSARRAY		D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY
#define	D3D_RTV_DIMENSION_TEXTURE3D				D3D11_RTV_DIMENSION_TEXTURE3D

#define	D3D_UAV_DIMENSION_UNKNOWN				D3D11_UAV_DIMENSION_UNKNOWN
#define	D3D_UAV_DIMENSION_BUFFER				D3D11_UAV_DIMENSION_BUFFER
#define	D3D_UAV_DIMENSION_TEXTURE1D				D3D11_UAV_DIMENSION_TEXTURE1D
#define	D3D_UAV_DIMENSION_TEXTURE1DARRAY		D3D11_UAV_DIMENSION_TEXTURE1DARRAY
#define	D3D_UAV_DIMENSION_TEXTURE2D				D3D11_UAV_DIMENSION_TEXTURE2D
#define	D3D_UAV_DIMENSION_TEXTURE2DARRAY		D3D11_UAV_DIMENSION_TEXTURE2DARRAY
#define	D3D_UAV_DIMENSION_TEXTURE3D				D3D11_UAV_DIMENSION_TEXTURE3D

typedef D3D11_SHADER_DESC				D3D_SHADER_DESC;
typedef	D3D11_SHADER_BUFFER_DESC		D3D_SHADER_BUFFER_DESC;
typedef	D3D11_SHADER_VARIABLE_DESC		D3D_SHADER_VARIABLE_DESC;
typedef D3D11_SHADER_INPUT_BIND_DESC	D3D_SHADER_INPUT_BIND_DESC;
typedef	D3D11_SHADER_TYPE_DESC			D3D_SHADER_TYPE_DESC;
typedef D3D11_CBUFFER_TYPE				D3D_CBUFFER_TYPE;

typedef ID3D11ShaderReflection							ID3DShaderReflection;
typedef	ID3D11ShaderReflectionConstantBuffer			ID3DShaderReflectionConstantBuffer;
typedef	ID3D11ShaderReflectionVariable					ID3DShaderReflectionVariable;
typedef	ID3D11ShaderReflectionType						ID3DShaderReflectionType;


#define	IID_ID3DShaderReflection	IID_ID3D11ShaderReflection

typedef	dx10State				ID3DState;
#define DX10_ONLY(expr)			expr
