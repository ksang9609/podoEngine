#pragma once

#include "Core/Core.h"
#include "Core/Math/Vector.h"
#include "Core/Container/TArray.h"
#include <filesystem>

struct FNormalVertex;
struct FStaticMesh;

struct FObjMaterialInfo
{
	FString Name;
	FVector AmbientColor = FVector(0.0f, 0.0f, 0.0f);   // Ka
	FVector DiffuseColor = FVector(1.f, 1.0f, 1.0f);   // Kd 
	FVector SpecularColor = FVector(0.0f, 0.0f, 0.0f);   // Ks
	float   SpecularExponent = 0.0f;                      // Ns
	float   Alpha = 1.0f;
	FString DiffuseTexturePath; // map_Kd
	FString NormalTexturePath;  // map_bump or norm
	FString SpecularPath;
};

// f v1/vt1/vn1
// FObjVertexIndex는 하나의 정점에 대한 위치, UV, 노멀들의 인덱스를 저장함.
struct FObjVertexIndex
{
	int32 PositionIndex = -1;
	int32 UVIndex = -1;
	int32 NormalIndex = -1;

	bool operator==(const FObjVertexIndex&) const = default;
};

/// FObjFace는 하나의 면을 나타냄 (n각형 : 정점 n개)
struct FObjFace
{
	TArray<FObjVertexIndex> VertexIndices; // Face의 각 Vertex에 대한 Position, UV, Normal Index

	int32 smoothingGroup = 0;
};

struct FObjFaceGroup
{
	int32 MaterialIndex = -1;
	int32 GroupIndex = -1;

	uint32 FirstFaceIndex = 0;
	uint32 FaceCount = 0;
};

struct FObjInfo
{
	TArray<FVector> Positions;
	TArray<FVector2> UVs;
	TArray<FVector> Normals;

	TArray<FObjFace> Faces;
	TArray<FObjMaterialInfo> Materials;
	TArray<FString> GroupNames;
	TArray<FObjFaceGroup> FaceGroups;
};


class FObjImporter
{
public:

	//obj 파일을 읽어서 FStaticMesh로 변환하는 함수
	static FStaticMesh* ParseAndConvert(const FString& fileName);

private:

	// Obj File을 읽어서 FStaticMesh로 변환하는 함수
	static FStaticMesh* loadStaticMeshFromBinary(const FString& fileName);

	// Obj File을 파싱해서 FObjInfo
	static bool parseObjFile(const FString& fileName, FObjInfo& outObjInfo);

	// Parsing Mtl File 에서 FObjMaterialInfo
	static bool parseMtlFile(const std::filesystem::path& filePath, TArray<FObjMaterialInfo>& outMaterials);

	//FObjInfo 에서 FStaticMesh로 변환
	static void convertObjToStaticMesh(const FObjInfo& objInfo, FStaticMesh& outStaticMesh);

	static FVector positionToUEBasis(const FVector& inVector)
	{
		return FVector(inVector.x, -inVector.y, inVector.z);
	}

	static FVector2 UVToUEBasis(const FVector2& inVector)
	{
		return FVector2(inVector.x, 1.0f - inVector.y);
	}
private:
	FObjInfo mObjInfo;
};
