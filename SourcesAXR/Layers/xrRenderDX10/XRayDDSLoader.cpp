#include "stdafx.h"
#pragma pack(push,4)
#include "dds.h"
#pragma pack(pop)
#include "XRayDDSLoader.h"

bool XRayDDSLoader::Load(const char * file_name)
{
	auto*F = FS.r_open(file_name);
	int result = Load(F);
	FS.r_close(F);

	if (result != 0)
		Msg("Failed %i", result);

	return result == 0;

}

static u32 ConvertColor(u32 c, u32 inbits, u32 outbits)
{
	if (inbits == 0)
	{
		return 0;
	}
	else if (inbits >= outbits)
	{
		return c >> (inbits - outbits);
	}
	else
	{
		return (c << (outbits - inbits)) | ConvertColor(c, inbits, outbits - inbits);
	}
}

int XRayDDSLoader::Load(IReader * F)
{

	if (F->r_u32() == MAKEFOURCC('D', 'D', 'S', ' '))
	{
		size_t size = F->r_u32();
		DDS_HEADER header;
		FillMemory(&header, sizeof(DDS_HEADER), 0);
		F->seek(4);
		if (size > sizeof(header) || size < 4)
			return 1;

		F->r(&header, size);
		//if (!(header.dwHeaderFlags&DDSD_WIDTH) || !(header.dwHeaderFlags&DDSD_HEIGHT) || !(header.dwHeaderFlags&DDSD_PIXELFORMAT))
		//return 2;

		m_mips = header.dwHeaderFlags&DDSD_MIPMAPCOUNT ? header.dwMipMapCount : 1;
		m_h = header.dwHeight;
		m_w = header.dwWidth;
		m_bCube = !!(header.dwCubemapFlags&DDS_CUBEMAP_ALLFACES);
		m_depth = header.dwHeaderFlags & DDSD_DEPTH ? header.dwDepth : (m_bCube ? 6 : 1);

		if (m_bCube && m_depth != 6)
		{
			return 3;
		}

		if (m_mips == 0)
		{
			return 4;
		}
		if (header.ddspf.dwFlags != DDS_FOURCC)
		{
			bool alpha = header.ddspf.dwFlags&(DDS_RGBA&(~DDS_RGB));
			if (header.ddspf.dwRGBBitCount % 8 || header.ddspf.dwRGBBitCount > 64)
			{
				return 5;
			}
			size_t byte_size_pixel = header.ddspf.dwRGBBitCount / 8;
			size_t size_bit[4], shift_bit[4];
			for (size_t x = 0; x < 4; x++)
				maskShiftAndSize(header.ddspf.dwBitsMask[x], shift_bit + x, size_bit + x);
			if (header.ddspf.dwRBitMask && header.ddspf.dwGBitMask && header.ddspf.dwBBitMask)
				m_px = alpha ? TPF_R8G8B8A8 : TPF_R8G8B8;
			else	if (header.ddspf.dwRBitMask && header.ddspf.dwGBitMask)
				m_px = TPF_R8G8;
			else if (header.ddspf.dwABitMask)
				m_px = TPF_R8G8B8A8;
			else
				m_px = TPF_R8;

			u8 coutComp = GetCountComp(m_px);

			if (coutComp == 1 && header.ddspf.dwBitsMask[3])
			{
				std::swap(size_bit[0], size_bit[3]);

				std::swap(shift_bit[0], shift_bit[3]);
				std::swap(header.ddspf.dwBitsMask[0], header.ddspf.dwBitsMask[3]);
			}

			u32 pixel = 0;
			Create();
			for (size_t d = 0; d < m_depth; d++)
			{
				for (size_t m = 0; m < m_mips; m++)
				{
					size_t h = GetMip(m_h, m);
					size_t w = GetMip(m_w, m);
					u8*data = GetImage(m_data, m_w, m_h, m_mips, d, m, m_px);
					for (size_t x = 0; x < w*h; x++)
					{
						F->r(&pixel, byte_size_pixel);
						for (size_t a = 0; a < coutComp; a++)
						{
							*GetPixelUint8(x, 0, 0, coutComp, a, data) = static_cast<u8>(ConvertColor((pixel & header.ddspf.dwBitsMask[a]) >> shift_bit[a], size_bit[a], 8));;
						}
					}
				}
			}
			return 0;
		}
		else
		{
			if (header.ddspf.dwFourCC == MAKEFOURCC('D', 'X', '1', '0'))
			{
				if (size + sizeof(DDSHeader10) == sizeof(DDS_HEADER))
				{
					F->seek(4);
					F->r(&header, size += sizeof(DDSHeader10));
				}

				switch (header.Header10.dxgiFormat)
				{
				case DXGI_FORMAT_R32G32B32A32_FLOAT:
					m_px = TPF_R32G32B32A32F;
					break;
				case DXGI_FORMAT_R32G32B32_FLOAT:
					m_px = TPF_R32G32B32F;
					break;
				case DXGI_FORMAT_R32G32_FLOAT:
					m_px = TPF_R32G32F;
					break;
				case DXGI_FORMAT_R32_FLOAT:
					m_px = TPF_R32F;
					break;
				case DXGI_FORMAT_R8G8B8A8_UNORM:
					m_px = TPF_R8G8B8A8;
					break;
				case DXGI_FORMAT_R8G8_UNORM:
					m_px = TPF_R8G8;
					break;
				case DXGI_FORMAT_R8_UNORM:
					m_px = TPF_R8;
					break;
				case DXGI_FORMAT_BC1_UNORM:
					m_px = TPF_BC1;
					break;
				case DXGI_FORMAT_BC2_UNORM:
					m_px = TPF_BC2;
					break;
				case DXGI_FORMAT_BC3_UNORM:
					m_px = TPF_BC3;
					break;
				case DXGI_FORMAT_BC4_UNORM:
					m_px = TPF_BC4;
					break;
				case DXGI_FORMAT_BC5_UNORM:
					m_px = TPF_BC5;
					break;
				case DXGI_FORMAT_BC6H_SF16:
					m_px = TPF_BC6;
					break;
				case DXGI_FORMAT_BC7_UNORM:
					m_px = TPF_BC7;
					break;
				default:
					Clear();
					return 6;
				}
			}
			else
			{
				switch (header.ddspf.dwFourCC)
				{
				case  MAKEFOURCC('D', 'X', 'T', '1'):
					m_px = TPF_BC1;
					break;
				case  MAKEFOURCC('D', 'X', 'T', '2'):
				case  MAKEFOURCC('D', 'X', 'T', '3'):
					m_px = TPF_BC2;
					break;
				case  MAKEFOURCC('D', 'X', 'T', '4'):
				case  MAKEFOURCC('D', 'X', 'T', '5'):
					m_px = TPF_BC3;
					break;
				case  MAKEFOURCC('A', 'T', 'I', '1'):
					m_px = TPF_BC4;
					break;
				case  MAKEFOURCC('A', 'T', 'I', '2'):
					m_px = TPF_BC5;
					break;
				default:
					Clear();
					return 7;
				}
			}
			Create();
			F->r(m_data, GetSizeInMemory());
			return 0;
		}

	}
	return 8;
}

void XRayDDSLoader::Create()
{
	Clear();
	m_data = xr_alloc<u8>(GetSizeInMemory());
}

XRayDDSLoader::XRayDDSLoader():m_data(0), m_bCube(0)
{
}

XRayDDSLoader::~XRayDDSLoader()
{
	Clear();
}

void XRayDDSLoader::Clear()
{
	if(m_data)
	xr_free(m_data);
	m_data = 0;
}

void XRayDDSLoader::To(ID3D11Texture2D*& Texture, bool bStaging, LPCSTR fRName)
{
	D3D11_TEXTURE2D_DESC desc;
	FillMemory(&desc, sizeof(desc), 0);
	desc.ArraySize = isCube() ? 6 : 1;
	desc.MipLevels = static_cast<UINT>(m_mips);
	desc.SampleDesc.Count = 1;

	desc.Width = static_cast<UINT>(m_w);
	desc.Height = static_cast<UINT>(m_h);
	if (isCompressor(m_px))
	{

		UINT w = desc.Width % 4;
		if (w)w = 4 - w;
		UINT h = desc.Width % 4;
		if (h)h = 4 - h;
		desc.Width += w;
		desc.Height += h;
	}
	desc.Format = TranslateTextureFromat(m_px);
	desc.MiscFlags = isCube() ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	if (bStaging)
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_STAGING;


	}
	else
	{
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	}

	u8* ptr = 0;
	u8* ptr_free = 0;
	if (m_px == TPF_R8G8B8)
	{
		ptr = xr_alloc<u8>(GetSizeInMemory(m_w, m_h, m_mips, TPF_R8G8B8A8) * m_depth);
		u8* data = ptr;
		u8* old_data = m_data;
		for (size_t d = 0; d < desc.ArraySize; d++)
		{
			for (size_t i = 0; i < m_mips; i++)
			{
				size_t size_old = GetSizeDepth(GetMip(desc.Width, i), GetMip(desc.Height, i), TPF_R8G8B8);
				size_t size_new = GetSizeDepth(GetMip(desc.Width, i), GetMip(desc.Height, i), TPF_R8G8B8A8);
				memcpy(data, old_data, size_old);
				R8G8B8ToR8G8B8A8(data, GetMip(desc.Width, i), GetMip(desc.Height, i));
				data += size_new;
				old_data += size_old;
			}
		}
		ptr_free = ptr;
	}
	else
	{
		ptr = m_data;

	}

	D3D11_SUBRESOURCE_DATA subdata[256];
	FillMemory(&subdata[0], sizeof(D3D11_SUBRESOURCE_DATA) * 256, 0);
	for (size_t d = 0; d < desc.ArraySize; d++)
	{
		for (size_t i = 0; i < m_mips; i++)
		{
			size_t mip_w = GetMip(m_w, i);
			size_t mip_h = GetMip(m_h, i);
			subdata[i + d * m_mips].SysMemPitch = static_cast<UINT>(GetSizeWidth(mip_w, m_px));
			subdata[i + d * m_mips].SysMemSlicePitch = static_cast<UINT>(GetSizeDepth(mip_w, mip_h, m_px));
			subdata[i + d * m_mips].pSysMem = ptr;
			ptr += subdata[i + d * m_mips].SysMemSlicePitch;
		}
	}

#ifdef DEBUG
	R_CHK(HW.pDevice->CreateTexture2D(&desc, subdata, &Texture));
#else
	R_MSG(!HW.pDevice->CreateTexture2D(&desc, subdata, &Texture), "![XRayDDSLoader]: Can`t create 2D texture: [%s]!", fRName);
#endif

	if (m_px == TPF_R8G8B8)
	{
		xr_free(ptr_free);
	}

}
