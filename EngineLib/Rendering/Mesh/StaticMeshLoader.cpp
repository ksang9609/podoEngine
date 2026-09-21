#include "StaticMeshLoader.h"

#include <filesystem>
#include <utility>

#include "Archive.h"
#include "Core/IO/FileManager.h"
#include "Core/IO/WindowsBinReader.h"
#include "Core/IO/WindowsBinWriter.h"
#include "Core/Name.h"
#include "Rendering/Mesh/ObjImporter.h"
#include "Rendering/Mesh/StaticMesh.h"

namespace
{
	bool AreMaterialLibrariesUsable(
		const std::filesystem::path& sourcePath,
		const std::filesystem::file_time_type& binaryWriteTime);

	constexpr uint32 StaticMeshMagic =
		static_cast<uint32>('P') |
		(static_cast<uint32>('M') << 8) |
		(static_cast<uint32>('S') << 16) |
		(static_cast<uint32>('H') << 24);

	constexpr uint32 StaticMeshBinaryVersion = 2;

	constexpr uint32 MaxVertices = 10'000'000;
	constexpr uint32 MaxIndices = 30'000'000;
	constexpr uint32 MaxSections = 1'000'000;
	constexpr uint32 MaxGroupNames = 1'000'000;
	constexpr uint32 MaxMaterialSlots = 65'536;

	std::filesystem::path ResolvePath(
		const FString& pathString)
	{
		std::filesystem::path path( pathString.CStr());

		if (path.is_relative())
		{
			path = std::filesystem::path{kDefaultRootPath} / path;
		}

		std::error_code error;

		auto normalized = std::filesystem::weakly_canonical( path, error);

		if (!error)
		{
			return normalized;
		}

		error.clear();

		normalized = std::filesystem::absolute( path, error);

		return error ? path.lexically_normal() : normalized.lexically_normal();
	}

	std::filesystem::path MakeBinaryPath( const std::filesystem::path& sourcePath)
	{
		std::filesystem::path binaryPath = sourcePath;

		binaryPath.replace_extension(".bin");
		return binaryPath;
	}

	bool IsBinaryUsable( const std::filesystem::path& sourcePath, const std::filesystem::path& binaryPath)
	{
		std::error_code error;

		const bool binaryExists = std::filesystem::is_regular_file( binaryPath, error);

		if (error || !binaryExists)
		{
			return false;
		}

		error.clear();

		const bool sourceExists = std::filesystem::is_regular_file( sourcePath, error);

		if (error)
		{
			return false;
		}

		// 배포 환경처럼 OBJ가 없고 바이너리만 있는 경우
		if (!sourceExists)
		{
			return true;
		}

		const auto sourceWriteTime = std::filesystem::last_write_time( sourcePath, error);

		if (error)
		{
			return false;
		}

		const auto binaryWriteTime = std::filesystem::last_write_time( binaryPath, error);

		if (error)
		{
			return false;
		}

		if (binaryWriteTime < sourceWriteTime)
		{
			return false;
		}

		return AreMaterialLibrariesUsable(sourcePath, binaryWriteTime);
	}


	// 텍스처 경로를 바이너리 파일에 저장할 때는 상대 경로로 변환하고, 바이너리에서 읽을 때는 다시 절대 경로로 복원하는 함수.
	// 다른 PC나 다른 폴더로 .bin과 텍스처를 함께 옮겨도 원래 PC의 절대 경로를 참조해서 텍스처 로딩이 실패할 수 있는 상황을 방지.
	void SerializeTexturePath(FArchive& archive, FName& texture, const std::filesystem::path& binaryDirectory)
	{
		uint8 hasValue =
			archive.IsSaving() && texture.ComparisonIndex >= 0 ? 1 : 0;

		archive << hasValue;

		if (archive.HasError() || hasValue > 1)
		{
			archive.SetError();
			return;
		}

		if (!hasValue)
		{
			if (archive.IsLoading())
			{
				texture = FName();
			}
			return;
		}

		FString storedPath;

		if (archive.IsSaving())
		{
			// 현재 importer가 넣어 주는 텍스처 경로는 절대 경로.
			const std::filesystem::path absolutePath(
				texture.ToString().CStr());

			const auto relativePath =
				absolutePath.lexically_relative(binaryDirectory);

			// 다른 드라이브 등에 있어 상대 경로를 만들 수 없는 경우.
			if (relativePath.empty() || relativePath.is_absolute())
			{
				archive.SetError();
				return;
			}

			storedPath = FString(relativePath.generic_string().c_str());
		}

		archive << storedPath;

		if (archive.IsLoading() && !archive.HasError())
		{
			const std::filesystem::path relativePath(storedPath.CStr());

			if (relativePath.empty() || relativePath.is_absolute())
			{
				archive.SetError();
				return;
			}

			const auto absolutePath =
				(binaryDirectory / relativePath).lexically_normal();

			texture = FName(FString(absolutePath.string().c_str()));
		}
	}

	void SerializeName( FArchive& archive, FName& name)
	{
		uint8 hasValue = archive.IsSaving() ? static_cast<uint8>(name.ComparisonIndex >= 0) : 0;

		archive << hasValue;

		if (archive.HasError() ||
			hasValue > 1)
		{
			archive.SetError();
			return;
		}

		FString stringValue;

		if (archive.IsSaving() && hasValue)
		{
			stringValue = name.ToString();
		}

		if (hasValue)
		{
			archive << stringValue;
		}

		if (archive.IsLoading() && !archive.HasError())
		{
			name = hasValue ? FName(stringValue) : FName();
		}
	}

	void SerializeVector( FArchive& archive, FVector& vector)
	{
		archive << vector.x;
		archive << vector.y;
		archive << vector.z;
	}

	void SerializeVector2(FArchive& archive, FVector2& vector)
	{
		archive << vector.x;
		archive << vector.y;
	}

	void SerializeColor(FArchive& archive, FLinearColor& color)
	{
		archive << color.R;
		archive << color.G;
		archive << color.B;
		archive << color.A;
	}

	void SerializeVertex(FArchive& archive, FNormalVertex& vertex)
	{
		SerializeVector(archive, vertex.pos);

		SerializeVector(archive, vertex.normal);

		SerializeColor(archive, vertex.color);

		SerializeVector2(archive, vertex.tex);
	}

	void SerializeSection(FArchive& archive, FStaticMeshSection& section)
	{
		archive << section.Name;
		archive << section.MaterialSlotIndex;
		archive << section.StartIndex;
		archive << section.IndexCount;
		archive << section.GroupIndex;
	}

	void SerializeMaterial(FArchive& archive, FMaterial& material)
	{
		SerializeVector(archive, material.AmbientColor);

		SerializeVector(archive, material.DiffuseColor);

		SerializeVector(archive, material.SpecularColor);

		archive << material.SpecularExponent;
		archive << material.Opacity;

		SerializeName(archive, material.DiffuseTexture);

		SerializeName(archive,material.NormalTexture);

		SerializeName(archive, material.SpecularTexture);
	}

	void SerializeMaterialSlot(FArchive& archive, FMaterialSlot& slot)
	{
		archive << slot.Name;

		SerializeMaterial(archive, slot.DefaultMaterial);
	}

	template<typename T, typename Serializer>
	bool SerializeArray(FArchive& archive, TArray<T>& values, uint32 maxCount, uint64 minElementBytes, Serializer serializeElement)
	{
		uint32 count = archive.IsSaving() ?
			static_cast<uint32>(values.Num())
			: 0;

		archive << count;

		if (archive.HasError() || count > maxCount)
		{
			archive.SetError();
			return false;
		}

		if (archive.IsLoading())
		{
			values.Reset(static_cast<int32>(count));

			for (uint32 i = 0; i < count; ++i)
			{
				values.Add(T{});
			}
		}

		for (uint32 i = 0;i < count; ++i)
		{
			serializeElement(archive, values[i]);

			if (archive.HasError())
			{
				return false;
			}
		}

		return true;
	}

	bool SerializeBakedData(FArchive& archive, FStaticMesh& mesh, TArray<FMaterialSlot>& materialSlots)
	{

		// pos 3 + normal 3 + color 4 + tex 2 = float 12개.
		// sizeof(FNormalVertex)는 패딩이 포함될 수 있으므로 사용하지 않음.
		constexpr uint64 SerializedVertexBytes = sizeof(float) * 12;

		if (!SerializeArray( archive, mesh.Vertices, MaxVertices, SerializedVertexBytes, SerializeVertex))
		{
			return false;
		}

		if (!SerializeArray(archive,mesh.Indices,MaxIndices, sizeof(uint32),
			[](FArchive& ar, uint32& index)
			{
				ar << index;
			}))
		{
			return false;
		}

		if (!SerializeArray(archive, mesh.Sections, MaxSections, sizeof(FStaticMeshSection), SerializeSection))
		{
			return false;
		}

		if (!SerializeArray(archive, mesh.GroupNames, MaxGroupNames, sizeof(FString),
			[](FArchive& ar, FString& name)
			{
				ar << name;
			}))
		{
			return false;
		}

		if (!SerializeArray(archive,materialSlots,MaxMaterialSlots, sizeof(FMaterialSlot), SerializeMaterialSlot))
		{
			return false;
		}

		return !archive.HasError();
	}

	bool ValidateBakedData(const FStaticMesh& mesh, const TArray<FMaterialSlot>& materialSlots)
	{
		if (mesh.Vertices.IsEmpty() ||
			mesh.Indices.IsEmpty() ||
			mesh.Indices.Num() % 3 != 0)
		{
			return false;
		}

		for (uint32 index : mesh.Indices)
		{
			if (index >= static_cast<uint32>(mesh.Vertices.Num()))
			{
				return false;
			}
		}

		for (const FStaticMeshSection& section :
			mesh.Sections)
		{
			if (section.StartIndex < 0 ||
				section.IndexCount <= 0 ||
				section.IndexCount % 3 != 0)
			{
				return false;
			}

			const uint64 sectionEnd =
				static_cast<uint64>(section.StartIndex) + static_cast<uint64>(section.IndexCount);

			if (sectionEnd > static_cast<uint64>(mesh.Indices.Num()))
			{
				return false;
			}

			if (section.MaterialSlotIndex < -1 ||
				section.MaterialSlotIndex >= static_cast<int32>(materialSlots.Num()))
			{
				return false;
			}

			if (section.GroupIndex < -1 ||
				section.GroupIndex >= static_cast<int32>(mesh.GroupNames.Num()))
			{
				return false;
			}
		}

		return true;
	}

	// OBJ 파일에서 참조하는 mtl 파일이 바이너리보다 최신이면, 바이너리를 다시 생성해야 함.
	bool AreMaterialLibrariesUsable(const std::filesystem::path& sourcePath, const std::filesystem::file_time_type& binaryWriteTime)
	{
		std::ifstream objFile(sourcePath);

		if (!objFile.is_open())
		{
			return false;
		}

		std::string line;

		while (std::getline(objFile, line))
		{
			std::istringstream tokens(line);
			std::string keyword;

			if (!(tokens >> keyword) || keyword != "mtllib")
			{
				continue;
			}

			std::string materialFile;
			if (!(tokens >> materialFile))
			{
				return false;
			}

			const auto materialPath = sourcePath.parent_path() / materialFile;

			std::error_code error;
			const auto materialWriteTime = std::filesystem::last_write_time(materialPath, error);

			if (error || materialWriteTime > binaryWriteTime)
			{
				return false;
			}
		}

		return !objFile.bad();
	}

	bool TryLoadBinary( const std::filesystem::path& sourcePath, FStaticMeshCookedData& outResult)
	{
		const std::filesystem::path binaryPath = MakeBinaryPath(sourcePath);

		if (!IsBinaryUsable( sourcePath, binaryPath))
		{
			return false;
		}

		FWindowsBinReader reader(binaryPath);

		if (reader.HasError())
		{
			return false;
		}

		uint32 magic = 0;
		uint32 version = 0;

		reader << magic;
		reader << version;

		if (reader.HasError() ||
			magic != StaticMeshMagic ||
			version != StaticMeshBinaryVersion)
		{
			return false;
		}

		auto mesh = std::make_unique<FStaticMesh>();

		TArray<FMaterialSlot> materialSlots;

		if (!SerializeBakedData(reader, *mesh, materialSlots))
		{
			return false;
		}

		if (!ValidateBakedData( *mesh, materialSlots))
		{
			return false;
		}

		outResult.meshData =std::move(mesh);

		outResult.materialSlots = std::move(materialSlots);

		return true;
	}

	bool SaveBinary(const std::filesystem::path& sourcePath, FStaticMesh& mesh, TArray<FMaterialSlot>& materialSlots)
	{
		const std::filesystem::path binaryPath = MakeBinaryPath(sourcePath);

		FWindowsBinWriter writer(binaryPath);

		if (writer.HasError())
		{
			return false;
		}

		uint32 magic = StaticMeshMagic;
		uint32 version = StaticMeshBinaryVersion;

		writer << magic;
		writer << version;

		if (!SerializeBakedData(writer,mesh, materialSlots))
		{
			return false;
		}

		writer.Flush();
		return !writer.HasError();
	}
}

bool StaticMeshLoader::Load( const FString& sourcePath, FStaticMeshCookedData& outResult)
{
	outResult.meshData.reset();
	outResult.materialSlots.Reset(0);

	const std::filesystem::path resolvedPath =  ResolvePath(sourcePath);

	if (TryLoadBinary(resolvedPath, outResult))
	{
		outResult.meshData->PathFileName = FName(sourcePath);

		return true;
	}

	FObjImportResult objImportResult;

	if (!FObjImporter::ParseAndConvert(sourcePath,objImportResult) || !objImportResult.meshData)
	{
		return false;
	}

	// Bake 저장 실패가 현재 메시 로딩 실패를 뜻하지는 않는다.
	SaveBinary(resolvedPath, *objImportResult.meshData, objImportResult.materialSlots);

	objImportResult.meshData->PathFileName =
		FName(sourcePath);

	outResult.meshData =
		std::move(objImportResult.meshData);

	outResult.materialSlots =
		std::move(objImportResult.materialSlots);

	return true;
}
