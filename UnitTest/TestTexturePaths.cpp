#include "pch.h"

#include <chrono>
#include <fstream>
#include <objbase.h>
#include "Core/IO/PathEncoding.h"
#include "Rendering/Mesh/StaticMeshLoader.h"
#include "Rendering/GpuResourceManager.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "DirectXTK.lib")

namespace
{
	class TexturePaths : public testing::Test
	{
	protected:
		std::filesystem::path root;
		bool ownsRoot = false;

		void SetUp() override
		{
			root = std::filesystem::current_path() / ("TexturePathTest_" +
				std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
			ownsRoot = std::filesystem::create_directory(root);
			ASSERT_TRUE(ownsRoot);
		}

		void TearDown() override
		{
			if (ownsRoot)
			{
				std::error_code error;
				std::filesystem::remove_all(root, error);
			}
		}
	};

	TEST_F(TexturePaths, PreservesUtf8AndLegacyWindowsPaths)
	{
		const std::filesystem::path relative(L"\ud14d\uc2a4\ucc98/\uc774\ubbf8\uc9c0.png");
		const auto file = root / relative;
		std::filesystem::create_directories(file.parent_path());
		std::ofstream(file) << "texture";
		const auto utf8 = PathEncoding::ToUtf8(file);
		EXPECT_EQ(PathEncoding::FromUtf8(utf8), file);
		EXPECT_TRUE(std::filesystem::exists(PathEncoding::FromExternal(utf8)));
		// The Windows code page on Korean systems can also represent these names.
		BOOL substituted = FALSE;
		char legacy[1024]{};
		const int count = WideCharToMultiByte(CP_ACP, 0, file.c_str(), -1,
			legacy, sizeof(legacy), nullptr, GetACP() == CP_UTF8 ? nullptr : &substituted);
		if (count > 0 && !substituted)
		{
			EXPECT_EQ(PathEncoding::FromExternal(legacy), file);
		}
	}

	TEST_F(TexturePaths, ImportsAndReloadsKoreanTexturePathsFromBinary)
	{
		const auto folder = root / L"\ubaa8\ub378";
		std::filesystem::create_directories(folder);
		const auto obj = folder / L"\uba54\uc2dc.obj";
		const auto mtl = folder / L"\uc7ac\uc9c8.mtl";
		const auto texture = folder / L"\ud14d\uc2a4\ucc98/\uc774\ubbf8\uc9c0.png";
		std::filesystem::create_directories(texture.parent_path());
		std::ofstream(texture) << "texture";
		std::ofstream(obj) << "mtllib " << PathEncoding::ToUtf8(mtl.filename())
			<< "\nv 0 0 0\nv 1 0 0\nv 0 1 0\nusemtl material\nf 1 2 3\n";
		std::ofstream(mtl) << "newmtl material\nmap_Kd "
			<< PathEncoding::ToUtf8(texture.lexically_relative(folder))
			<< "\nmap_bump " << PathEncoding::ToUtf8(texture.lexically_relative(folder))
			<< "\nmap_Ks " << PathEncoding::ToUtf8(texture.lexically_relative(folder)) << '\n';
		FStaticMeshCookedData first;
		const FString source(PathEncoding::ToUtf8(obj).c_str());
		ASSERT_TRUE(StaticMeshLoader::Load(source, first));
		ASSERT_EQ(first.materialSlots.Num(), 1);
		const auto binary = std::filesystem::path(obj).replace_extension(".bin");
		ASSERT_TRUE(std::filesystem::exists(binary));
		// Remove the source geometry while preserving its timestamp: success now requires the cache.
		const auto sourceTime = std::filesystem::last_write_time(obj);
		std::ofstream(obj, std::ios::trunc) << "# cache-only test\n";
		std::filesystem::last_write_time(obj, sourceTime);
		FStaticMeshCookedData cached;
		ASSERT_TRUE(StaticMeshLoader::Load(source, cached));
		ASSERT_EQ(cached.materialSlots.Num(), 1);
		const auto& material = cached.materialSlots[0].DefaultMaterial;
		for (const auto name : { material.DiffuseTexture, material.NormalTexture, material.SpecularTexture })
		{
			EXPECT_EQ(PathEncoding::FromUtf8(name.ToString().CStr()), texture);
		}
	}

	TEST_F(TexturePaths, LoadsGpuTexturesAndRetriesAfterMissingFile)
	{
		const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		ASSERT_TRUE(SUCCEEDED(comResult));
		struct ComScope { ~ComScope() { CoUninitialize(); } } comScope;
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		ASSERT_TRUE(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP,
			nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, nullptr)));
		FAssetManager assets;
		FGpuResourceManager resources;
		resources.Initialize(assets, *device.Get());
		const auto folder = root / L"\ud14d\uc2a4\ucc98";
		std::filesystem::create_directories(folder);
		const auto png = folder / L"\uc774\ubbf8\uc9c0.png";
		const FName name(PathEncoding::ToUtf8(png).c_str());
		const auto count = resources.GetTextureCount();
		EXPECT_EQ(resources.FindTextureOrAdd(name), nullptr);
		EXPECT_EQ(resources.GetTextureCount(), count);
		std::filesystem::copy_file("Assets/Textures/RayKu_source/RayEye.png", png);
		ASSERT_NE(resources.FindTextureOrAdd(name), nullptr);
		EXPECT_EQ(resources.GetTextureCount(), count + 1);
		const auto dds = folder / L"\uc774\ubbf8\uc9c0.dds";
		std::filesystem::copy_file("Assets/Textures/CubeTextureSample.dds", dds);
		ASSERT_NE(resources.FindTextureOrAdd(FName(PathEncoding::ToUtf8(dds).c_str())), nullptr);
		resources.CreateUnicodeFontTexture(name);
		ASSERT_NE(resources.FindTextureOrAdd(name), nullptr);
	}
}
