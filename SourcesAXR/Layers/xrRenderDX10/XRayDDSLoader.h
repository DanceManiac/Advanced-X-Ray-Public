#pragma once
class XRayDDSLoader
{

	static inline size_t GetCountMips(size_t w, size_t h)
	{
		size_t max_size = _max(w, h);
		return static_cast<size_t>(log2(static_cast<double>(max_size)) + 1);
	}
	static inline void maskShiftAndSize(size_t mask, size_t * shift, size_t * size)
	{
		if (!mask)
		{
			*shift = 0;
			*size = 0;
			return;
		}

		*shift = 0;
		while ((mask & 1) == 0) {
			++(*shift);
			mask >>= 1;
		}

		*size = 0;
		while ((mask & 1) == 1) {
			++(*size);
			mask >>= 1;
		}
	}
public:
	enum BearTexturePixelFormat
	{
		TPF_R8 = 0,
		TPF_R8G8,
		TPF_R8G8B8,
		TPF_R8G8B8A8,
		TPF_R32F,
		TPF_R32G32F,
		TPF_R32G32B32F,
		TPF_R32G32B32A32F,
		TPF_DXT_1,//not alpha
		TPF_DXT_1_alpha,//alpha  1 bit
		TPF_DXT_3,//alpga 
		TPF_DXT_5,//alpha
		TPF_BC1 = TPF_DXT_1,
		TPF_BC1a = TPF_DXT_1_alpha,
		TPF_BC2 = TPF_DXT_3,
		TPF_BC3 = TPF_DXT_5,
		TPF_BC4,//R 32 bit 16 pxiel
		TPF_BC5,//RB 64 bit 16 pxiel
		TPF_BC6,//RGBA FP16 64 bit 16 pxiel
		TPF_BC7//RGBA 64 bit 16 pxiel
	};
private:

	static inline u8 GetCountComp(BearTexturePixelFormat format)
	{
		switch (format)
		{
		case TPF_R8:
			return 1;
			break;
		case TPF_R8G8:
			return 2;
			break;
		case TPF_R8G8B8:
			return 3;
			break;
		case TPF_R8G8B8A8:
			return 4;
			break;
		case TPF_R32F:
			return 1;
			break;
		case TPF_R32G32F:
			return 2;
			break;
		case TPF_R32G32B32F:
			return 3;
			break;
		case TPF_R32G32B32A32F:
			return 4;
		default:
			NODEFAULT;
		}
		return 0;
	}
	static inline bool isCompressor(BearTexturePixelFormat format)
	{
		switch (format)
		{
		case TPF_R8:
		case TPF_R8G8:
		case TPF_R8G8B8:
		case TPF_R8G8B8A8:
		case TPF_R32F:
		case TPF_R32G32F:
		case TPF_R32G32B32F:
		case TPF_R32G32B32A32F:
			return false;
			break;
		case TPF_DXT_1:
		case TPF_DXT_1_alpha:
		case TPF_DXT_3:
		case TPF_DXT_5:
		case TPF_BC4:
		case TPF_BC5:
		case TPF_BC6:
		case TPF_BC7:
			return true;
		default:
			break;
		}
		return false;
	}
	static inline size_t  GetSizePixel(BearTexturePixelFormat format)
	{
		switch (format)
		{
		case TPF_R8:
			return 1;
			break;
		case TPF_R8G8:
			return 2;
			break;
		case TPF_R8G8B8:
			return 3;
			break;
		case TPF_R8G8B8A8:
			return 4;
			break;
		case TPF_R32F:
			return 4;
			break;
		case TPF_R32G32F:
			return 8;
			break;
		case TPF_R32G32B32F:
			return 12;
			break;
		case TPF_R32G32B32A32F:
			return 16;
			break;
		default:
			NODEFAULT;
			break;

		}
		return 0;
	}

	static inline size_t GetSizeBlock(BearTexturePixelFormat format)
	{
		switch (format)
		{
		case TPF_DXT_1:
			return 8;
		case TPF_DXT_1_alpha:
			return 8;
		case TPF_DXT_3:
			return 16;
		case TPF_DXT_5:
			return 16;
		case TPF_BC4:
			return 8;
			break;
		case TPF_BC5:
			return 16;
			break;
		case TPF_BC6:
			return 16;
			break;
		case TPF_BC7:
			return 16;
			break;
		default:
			NODEFAULT;
		}
		return 0;
	}

	static inline  DXGI_FORMAT TranslateTextureFromat(BearTexturePixelFormat format)
	{
		switch (format)
		{
		case TPF_R8:
			return DXGI_FORMAT_R8_UNORM;
			break;
		case TPF_R8G8:
			return DXGI_FORMAT_R8G8_UNORM;
			break;
		case TPF_R8G8B8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TPF_R8G8B8A8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TPF_R32F:
			return DXGI_FORMAT_R32_FLOAT;
			break;
		case TPF_R32G32F:
			return DXGI_FORMAT_R32G32_FLOAT;
			break;
		case TPF_R32G32B32F:
			return DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case TPF_R32G32B32A32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case TPF_BC1:
		case TPF_BC1a:
			return DXGI_FORMAT_BC1_UNORM;
		case TPF_BC2:
			return DXGI_FORMAT_BC2_UNORM;
		case TPF_BC3:
			return DXGI_FORMAT_BC3_UNORM;
		case TPF_BC4:
			return DXGI_FORMAT_BC4_UNORM;
		case TPF_BC5:
			return DXGI_FORMAT_BC5_UNORM;
		case TPF_BC6:
			return DXGI_FORMAT_BC6H_UF16;
		case TPF_BC7:
			return DXGI_FORMAT_BC7_UNORM;
		default:
			NODEFAULT;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	static inline size_t GetCountBlock(size_t w)
	{
		return (w + 3) / 4;
	}
	static inline size_t GetMip(size_t w, size_t level)
	{
		size_t mip = static_cast<size_t>((w) / pow(2, static_cast<double>(level)));
		return _max(mip, size_t(1));
	}
	static	inline size_t GetSizeInMemory(size_t w, size_t h, size_t mips, BearTexturePixelFormat format)
	{
		size_t result = 0;
		if (!isCompressor(format))
		{
			size_t pixel_size = GetSizePixel(format);
			//result += w * h * pixel_size;
			for (size_t i = 0; i < mips; i++)
			{
				size_t mip_w = GetMip(w, i);
				size_t mip_h = GetMip(h, i);
				result += mip_w * mip_h * pixel_size;
			}
		}
		else
		{
			size_t size_block = GetSizeBlock(format);
			//	result += GetCountBlock(w)*GetCountBlock(h)*size_block;
			for (size_t i = 0; i < mips; i++)
			{
				size_t mip_w = GetMip(w, i);
				size_t mip_h = GetMip(h, i);
				result += GetCountBlock(mip_w)*GetCountBlock(mip_h)* size_block;
			}
		}
		return result;
	}
	static	inline u8 * GetImage(u8 * data, size_t w, size_t h, size_t mips, size_t depth, size_t mip,BearTexturePixelFormat format)
	{
		return data + GetSizeInMemory(w, h, mips, format)*depth +GetSizeInMemory(w, h, mip, format);
	}
	static	inline u8 * GetPixelUint8(size_t x, size_t y, size_t w, size_t comps, size_t comp_id, u8 * data)
	{
		return  &data[(x + (y * w))*comps + comp_id];
	}

	static inline size_t   GetSizeWidth(size_t w, BearTexturePixelFormat format)
	{
		if (isCompressor(format))
		{
			return GetSizeBlock(format) * GetCountBlock(w);
		}
		else
		{
			return GetSizePixel(format) * w;
		}
	}

	static inline size_t   GetSizeDepth(size_t w, size_t h, BearTexturePixelFormat format)
	{
		if (isCompressor(format))
		{
			return GetSizeBlock(format) * GetCountBlock(w) * GetCountBlock(h);
		}
		else
		{
			return GetSizePixel(format) * w * h;
		}
	}

	static inline void R8G8B8ToR8G8B8A8(u8* data, size_t w, size_t h)
	{
		for (size_t y = h; y != 0;)
		{
			y--;
			for (size_t x = w; x != 0;)
			{
				x--;
				u8* src = GetPixelUint8(x, y, w, 3, 0, data);
				u8* dst = GetPixelUint8(x, y, w, 4, 0, data);
				dst[3] = 255;
				dst[2] = src[2];
				dst[1] = src[1];
				dst[0] = src[0];

			}
		}
	}

	int Load(IReader*F);
	void Create();
public:
	XRayDDSLoader();
	~XRayDDSLoader();
	void To(ID3D11Texture2D*& Texture, bool bStaging, LPCSTR fRName);
	void Clear();
	bool Load(const char*file_name);
	inline size_t GetSizeInMemory() const
	{
		return GetSizeInMemory(m_w, m_h, m_mips, m_px)*m_depth;
	}
	bool isCube()const { return m_bCube; }
private:
	bool m_bCube;
	u8*m_data;
	size_t m_depth;
	size_t m_mips;
	size_t m_h;
	size_t m_w;
	BearTexturePixelFormat m_px;
};
