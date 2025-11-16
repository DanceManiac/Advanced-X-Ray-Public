////////////////////////////////////////////////////////////////////////////
//	Module 		: Spheremap.h
//	Created 	: 06.08.2024
//  Modified 	: 06.08.2024
//	Authors		: yuriks, Dance Maniac (M.F.S. Team)
//	Description : Glues cubemap screenshots into a 360 panorama
////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "stb_image.hpp"
#include "stb_image_write.hpp"

typedef uint8_t u8;
typedef uint32_t u32;

inline void splitColor(u32 col, u8& r, u8& g, u8& b)
{
	r = col >> 0  & 0xFF;
	g = col >> 8  & 0xFF;
	b = col >> 16 & 0xFF;
}

inline u32 makeColor(u8 r, u8 g, u8 b)
{
	return r | g << 8 | b << 16 | 0xFF << 24;
}

inline float lerp(float a, float b, float t)
{
	return a * (1.f - t) + b * t;
}

struct Image
{
	int width, height;
	std::unique_ptr<u8, std::function<void(u8*)>> data;

	Image() :
		width(-1), height(-1)
	{}

	Image(const std::string& filename) : data(stbi_load(filename.c_str(), &width, &height, nullptr, 4), stbi_image_free)
	{
		if (data == nullptr)
			R_ASSERT2("[SphereGen]: Failed to open [%s]", filename);
	}

	Image& operator= (Image&& o)
	{
		width = o.width;
		height = o.height;
		data.swap(o.data);

		return *this;
	}

private:
	Image& operator= (const Image&);
};

struct Colorf
{
	float r, g, b;

	Colorf() { }
	Colorf(float r, float g, float b) : r(r), g(g), b(b) { }

	explicit Colorf(u32 col)
	{
		u8 rb, gb, bb;
		splitColor(col, rb, gb, bb);
		r = rb / 255.f;
		g = gb / 255.f;
		b = bb / 255.f;
	}

	u32 toU32() const
	{
		return makeColor(
			static_cast<u8>(r * 255),
			static_cast<u8>(g * 255),
			static_cast<u8>(b * 255));
	}

	static Colorf mix(const Colorf& a, const Colorf& b, float t)
	{
		return Colorf(
			lerp(a.r, b.r, t),
			lerp(a.g, b.g, t),
			lerp(a.b, b.b, t));
	}
};

struct Cubemap
{
	enum CubeFace
	{
		FACE_POS_X, FACE_NEG_X,
		FACE_POS_Y, FACE_NEG_Y,
		FACE_POS_Z, FACE_NEG_Z,
		NUM_FACES
	};

	Image faces[NUM_FACES];

	Cubemap(const std::vector<std::string> image_paths)
	{
		faces[FACE_POS_X] = Image(image_paths[0]);
		faces[FACE_NEG_X] = Image(image_paths[1]);
		faces[FACE_POS_Y] = Image(image_paths[2]);
		faces[FACE_NEG_Y] = Image(image_paths[3]);
		faces[FACE_POS_Z] = Image(image_paths[4]);
		faces[FACE_NEG_Z] = Image(image_paths[5]);
	}

	u32 readTexel(CubeFace face, int x, int y)
	{
		assert(face < NUM_FACES);
		const Image& face_img = faces[face];

		assert(x < face_img.width);
		assert(y < face_img.height);

		const u32* img_data = reinterpret_cast<const u32*>(face_img.data.get());
		return img_data[y * face_img.width + x];
	}

	void computeTexCoords(float x, float y, float z, CubeFace& out_face, float& out_s, float& out_t)
	{
		int major_axis;

		float v[3] = { x, y, z };
		float a[3] = { std::abs(x), std::abs(y), std::abs(z) };
		
		if (a[0] >= a[1] && a[0] >= a[2])
		{
			major_axis = 0;
		} 
		else if (a[1] >= a[0] && a[1] >= a[2])
		{
			major_axis = 1;
		} 
		else if (a[2] >= a[0] && a[2] >= a[1])
		{
			major_axis = 2;
		}

		if (v[major_axis] < 0.0f)
			major_axis = major_axis * 2 + 1;
		else
			major_axis *= 2;

		float tmp_s, tmp_t, m;
		switch (major_axis)
		{
			/* +X */ case 0: tmp_s = -z; tmp_t = -y; m = a[0]; break;
			/* -X */ case 1: tmp_s =  z; tmp_t = -y; m = a[0]; break;
			/* +Y */ case 2: tmp_s =  x; tmp_t =  z; m = a[1]; break;
			/* -Y */ case 3: tmp_s =  x; tmp_t = -z; m = a[1]; break;
			/* +Z */ case 4: tmp_s =  x; tmp_t = -y; m = a[2]; break;
			/* -Z */ case 5: tmp_s = -x; tmp_t = -y; m = a[2]; break;
		}

		out_face = CubeFace(major_axis);
		out_s = 0.5f * (tmp_s / m + 1.0f);
		out_t = 0.5f * (tmp_t / m + 1.0f);
	}

	u32 readTexelClamped(CubeFace face, int x, int y)
	{
		const Image& face_img = faces[face];

		x = std::max(std::min(x, face_img.width  - 1), 0);
		y = std::max(std::min(y, face_img.height - 1), 0);
		return readTexel(face, x, y);
	}

	u32 sampleFace(CubeFace face, float s, float t)
	{
		const Image& face_img = faces[face];

		const float x = s * face_img.width;
		const float y = t * face_img.height;

		const int x_base = static_cast<int>(x);
		const int y_base = static_cast<int>(y);
		const float x_fract = x - x_base;
		const float y_fract = y - y_base;

		const Colorf sample_00(readTexelClamped(face, x_base,     y_base));
		const Colorf sample_10(readTexelClamped(face, x_base + 1, y_base));
		const Colorf sample_01(readTexelClamped(face, x_base,     y_base + 1));
		const Colorf sample_11(readTexelClamped(face, x_base + 1, y_base + 1));

		const Colorf mix_0 = Colorf::mix(sample_00, sample_10, x_fract);
		const Colorf mix_1 = Colorf::mix(sample_01, sample_11, x_fract);

		const Colorf mix_final = Colorf::mix(mix_0, mix_1, y_fract);
		return mix_final.toU32();
	}
};

inline float unlerp(int val, int max)
{
	return (val + 0.5f) / max;
}

template <typename T>
inline typename T::value_type pop_from(T& container)
{
	typename T::value_type val = container.back();
	container.pop_back();
	return val;
}

static const float aa_pattern_none[] = { 0.f, 0.f };
static const float aa_pattern_5x[] =
{
	0.0f   , 0.0f   ,
	-.1875f, -.375f ,
	0.375f , -.1875f,
	0.1875f, 0.375f ,
	-.375f , 0.1875f,
};

static const float aa_pattern_16x[] =
{
	-.375, -.375,
	-.375, -.175,
	-.375, 0.175,
	-.375, 0.375,

	-.175, -.375,
	-.175, -.175,
	-.175, 0.175,
	-.175, 0.375,

	0.125, -.375,
	0.125, -.175,
	0.125, 0.175,
	0.125, 0.375,

	0.375, -.375,
	0.375, -.175,
	0.375, 0.175,
	0.375, 0.375,
};
  
int GenerateSpheremap(string_path file_name, int opt_size = 1024, u32 opt_aa = 1)
{
	int num_aa_samples = 1;
	const float* aa_sample_pattern = aa_pattern_none;

	switch (opt_aa)
	{
	case 1:
		aa_sample_pattern = aa_pattern_none;
		break;
	case 5:
		aa_sample_pattern = aa_pattern_5x;
		break;
	case 16:
		aa_sample_pattern = aa_pattern_16x;
		break;
	default:
		R_ASSERT("[SphereGen]: Invalid AA sample pattern.");
		return 1;
	} 

	string_path fileName;
	std::vector<std::string> image_paths{};

	for (u32 i = 0; i < 6; ++i)
	{
		if (FS.path_exist("$screenshots$"))
		{
			string128 file_name{};
			strconcat(sizeof(file_name), file_name, std::to_string(i + 1).c_str(), ".tga");
			FS.update_path(fileName, "$screenshots$", file_name);
			image_paths.push_back(fileName);
		}
	}

	float output_pixel_size = 1.f / opt_size;
	int output_size_x = opt_size;
	float output_pixel_size_x = 1.f / output_size_x;
	std::vector<u32> out_data(output_size_x * opt_size);

	{
		Cubemap input_cubemap(image_paths);

		for (int y = 0; y < opt_size; ++y)
		{
			for (int x = 0; x < opt_size; ++x)
			{
				float center_s = unlerp(x, opt_size);
				float center_t = unlerp(y, opt_size);

				u32 sample_r = 0, sample_g = 0, sample_b = 0;

				for (int sample = 0; sample < num_aa_samples; ++sample)
				{
					float s = center_s + aa_sample_pattern[sample * 2] * output_pixel_size;
					float t = center_t + aa_sample_pattern[sample * 2 + 1] * output_pixel_size;

					float scx = (2.0f * x / opt_size) - 1.0f;
					float scy = 1.0f - (2.0f * y / opt_size);

					float theta = scx * PI;
					float phi = scy * (PI / 2.0f);

					float vx = std::cos(phi) * std::cos(theta);
					float vy = std::sin(phi);
					float vz = std::cos(phi) * std::sin(theta);

					Cubemap::CubeFace cube_face;
					float tex_s, tex_t;
					input_cubemap.computeTexCoords(vx, vy, vz, cube_face, tex_s, tex_t);
					u32 sample_color = input_cubemap.sampleFace(cube_face, tex_s, tex_t);

					u8 r, g, b;
					splitColor(sample_color, r, g, b);
					sample_r += r;
					sample_g += g;
					sample_b += b;
				}

				sample_r /= num_aa_samples;
				sample_g /= num_aa_samples;
				sample_b /= num_aa_samples;

				out_data[y * opt_size + x] = makeColor(sample_r, sample_g, sample_b);
			}
		}
	}

	string_path panoramaPath;
	FS.update_path(panoramaPath, "$screenshots$", file_name ? file_name : "360_panorama.jpg");

	// Dance Maniac: Added saving to jpg format
	convert_tga_to_jpg(panoramaPath, out_data.data(), output_size_x, opt_size, 4, 95);

	// Delete cubemap screenshots
	for (i = 0; i < image_paths.size(); ++i)
	{
		if (FS.path_exist("$screenshots$"))
			FS.file_delete(image_paths[i].c_str());
	}
}
