#include "ObjImporter.h"
#include "Core/IO/FileManager.h"
#include "Editor/Console.h"
#include "StaticMesh.h"
#include <filesystem>
#include <fstream>
#include <istream>

namespace
{
	bool ReadLine(std::ifstream& File, FString& OutLine)
	{
		OutLine.Reset();
		char Ch;

		while (File.get(Ch))
		{
			if (Ch == '\n')
			{
				return true;
			}

			if (Ch != '\r')
			{
				OutLine.AppendChar(Ch);
			}
		}

		// 마지막 줄에 개행이 없는 경우
		return OutLine.Len() > 0;
	}

	bool ReadToken(const char*& Cursor, FString& OutToken)
	{
		OutToken.Reset();

		while (*Cursor == ' ' || *Cursor == '\t')
		{
			++Cursor;
		}

		if (*Cursor == '\0' || *Cursor == '#')
		{
			return false;
		}

		while (*Cursor != '\0' &&
			*Cursor != ' ' &&
			*Cursor != '\t' &&
			*Cursor != '#')
		{
			OutToken.AppendChar(*Cursor++);
		}

		return true;
	}
}

FStaticMesh* FObjImporter::ParseAndConvert(const FString& fileName)
{
	//테스트 코드 시작
    FObjInfo objInfo;
	FStaticMesh staticMesh;
    const bool Success = parseObjFile(fileName, objInfo);

    UE_LOG(Log, Render, "OBJ parsing: %s",
        Success ? "success" : "failed");

	convertObjToStaticMesh(objInfo, staticMesh);

    return nullptr; // 지금은 파싱만 테스트

	//테스트 코드 끝
}

bool FObjImporter::parseObjFile(const FString& fileName, FObjInfo& outObjInfo)
{
	//Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	//Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;        //The variable we will use to store one char from file at a time
	std::wstring face;        //Holds the string containing our face vertices
	int vIndex = 0;            //Keep track of our vertex index count
	int triangleCount = 0;    //Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	// OBJ 파일이 Assets 폴더에 있다면 kDefaultAssetsPath
	// 실행 폴더 바로 아래에 있다면 kDefaultRootPath
	std::filesystem::path objFilePath = std::filesystem::path{kDefaultRootPath} / fileName.CStr();
	std::ifstream fileIn(objFilePath.c_str());    //Open file

	if (!fileIn.is_open())
	{
		return false;
	}

	FString line;
	FString currentMaterialName;
	uint32 faceCount = 0;

	while (ReadLine(fileIn, line))
	{
		const char* Cursor = line.CStr();
		FString Prefix;

		if(!ReadToken(Cursor, Prefix)){ continue; }

		if (Prefix == "v")
		{
			FString X, Y, Z;
			if(!ReadToken(Cursor, X) || !ReadToken(Cursor, Y) || !ReadToken(Cursor, Z))
			{
				continue;
			}
			//UE_LOG(Log, Render, "v %f %f %f", X.ToFloat(), Y.ToFloat(), Z.ToFloat());
			outObjInfo.Positions.Add(FVector(X.ToFloat(), Y.ToFloat(), Z.ToFloat()));
		}
		else if (Prefix == "vt")
		{
			FString U, V;
			if(!ReadToken(Cursor, U) || !ReadToken(Cursor, V))
			{
				continue;
			}
			//UE_LOG(Log, Render, "vt %f %f", U.ToFloat(), V.ToFloat());
			outObjInfo.UVs.Add(FVector2(U.ToFloat(), V.ToFloat()));
		}
		else if (Prefix == "vn")
		{
			FString X, Y, Z;
			if(!ReadToken(Cursor, X) || !ReadToken(Cursor, Y) || !ReadToken(Cursor, Z))
			{
				continue;
			}
			//UE_LOG(Log, Render, "vn %f %f %f", X.ToFloat(), Y.ToFloat(), Z.ToFloat());
			outObjInfo.Normals.Add(FVector(X.ToFloat(), Y.ToFloat(), Z.ToFloat()));
		}
		else if (Prefix == "mtllib")
		{
			FString MtlFileName;
			if(!ReadToken(Cursor, MtlFileName))
			{
				continue;
			}
			//UE_LOG(Log, Render, MtlFileName.CStr());
			const auto& mtlFilePath = objFilePath.replace_filename(MtlFileName.CStr());
			parseMtlFile(mtlFilePath, outObjInfo.Materials);
		}
		else if (Prefix == "usemtl")
		{
			FString CurrentMaterialNameStr;
			if(!ReadToken(Cursor, CurrentMaterialNameStr))
			{
				continue;
			}

			auto It = std::find_if(outObjInfo.Materials.begin(), outObjInfo.Materials.end(),
				[&](const FObjMaterialInfo& Info) {
					return Info.Name == CurrentMaterialNameStr;
				}
			);

			int32 MatIndex = -1;
			if (It != outObjInfo.Materials.end())
			{
				MatIndex = std::distance(outObjInfo.Materials.begin(), It);
			}

			UE_LOG(Log, Render, CurrentMaterialNameStr.CStr());

			//인덱스 찾음 
			FObjFaceGroup newGroup = {};
			newGroup.MaterialIndex = MatIndex;
			newGroup.FirstFaceIndex = faceCount;
			outObjInfo.FaceGroups.Add(newGroup);
		}
		else if (Prefix == "f")
		{
			FObjFace newFace;
			FString faceData;
			while (ReadToken(Cursor, faceData))
			{
				const FString Slash("/");
				size_t Pos = faceData.Find(Slash);
				size_t Normal = (Pos == -1) ? -1 : faceData.Find(Slash, Pos + 1);

				FObjVertexIndex VertexIndex;

				VertexIndex.PositionIndex = faceData.Left(Pos).ToInt() - 1;


				if (Pos != -1 && Normal == -1)
				{
					// v/vt
					VertexIndex.UVIndex = faceData.RightChop(Pos + 1).ToInt() - 1;
				}
				else if (Normal != -1)
				{
					// v/vt/vn: 두 '/' 사이에 UV가 있는 경우
					// v//vn이면 건너뛰고 UVIndex는 -1 유지
					if (Normal > Pos + 1)
					{
						VertexIndex.UVIndex = faceData.Mid(Pos + 1, Normal - Pos - 1).ToInt() - 1;
					}

					// v/vt/vn 또는 v//vn의 노멀
					VertexIndex.NormalIndex = faceData.RightChop(Normal + 1).ToInt() - 1;
				}

				newFace.VertexIndices.Add(VertexIndex);
			}

			//테스트 코드 시작
			/*UE_LOG(Log, Render, "Face %u: %d vertices",faceCount,static_cast<int>(newFace.VertexIndices.Num()));

			for (const FObjVertexIndex& Vertex : newFace.VertexIndices)
			{
				UE_LOG(Log, Render, "Position=%d UV=%d Normal=%d",
					Vertex.PositionIndex,
					Vertex.UVIndex,
					Vertex.NormalIndex);
			}*/
			//테스트 코드 끝

			outObjInfo.VertexIndices.Add(newFace);
			faceCount++;
		}
	}

	return true;
}

bool FObjImporter::parseMtlFile(const std::filesystem::path& filePath, TArray<FObjMaterialInfo>& outMaterials)
{
	std::ifstream fileIn(filePath.c_str());
	FString line;

	if (!fileIn.is_open())
	{
		return false;
	}

	while (ReadLine(fileIn, line))
	{
		const char* Cursor = line.CStr();
		FString Prefix;

		if (!ReadToken(Cursor, Prefix)) { continue; }

		if (Prefix == "newmtl")
		{
			FString MaterialName;
			if (!ReadToken(Cursor, MaterialName)) { continue; }

			FObjMaterialInfo newMaterial;
			newMaterial.Name = MaterialName;

			//UE_LOG(Log, Render, newMaterial.Name.CStr());

			outMaterials.Add(newMaterial);
		}
		else if (Prefix == "Ka" || Prefix == "Kd" || Prefix == "Ks")
		{
			FString R, G, B;
			if (!ReadToken(Cursor, R) || !ReadToken(Cursor, G) || !ReadToken(Cursor, B)) { continue; }
			FVector Color(R.ToFloat(), G.ToFloat(), B.ToFloat());
			if (outMaterials.Num() > 0)
			{
				FObjMaterialInfo& CurrentMaterial = outMaterials[outMaterials.Num() - 1];
				if (Prefix == "Ka")
				{
					CurrentMaterial.AmbientColor = Color;
				}
				else if (Prefix == "Kd")
				{
					CurrentMaterial.DiffuseColor = Color;
				}
				else if (Prefix == "Ks")
				{
					CurrentMaterial.SpecularColor = Color;
				}
			}
			//UE_LOG(Log, Render, "%s %f %f %f", Prefix.CStr(), Color.x, Color.y, Color.z);
		}
		else if (Prefix == "Ns")
		{
			FString SpecularExponentStr;
			if (!ReadToken(Cursor, SpecularExponentStr)) { continue; }
			if (outMaterials.Num() > 0)
			{
				FObjMaterialInfo& CurrentMaterial = outMaterials[outMaterials.Num() - 1];
				CurrentMaterial.SpecularExponent = SpecularExponentStr.ToFloat();
			}
			//UE_LOG(Log, Render, "Ns %f", SpecularExponentStr.ToFloat());
		}
		else if (Prefix == "d" || Prefix == "Tr")
		{
			FString AlphaStr;
			if (!ReadToken(Cursor, AlphaStr)) { continue; }
			if (outMaterials.Num() > 0)
			{
				FObjMaterialInfo& CurrentMaterial = outMaterials[outMaterials.Num() - 1];
				float AlphaValue = AlphaStr.ToFloat();
				if (Prefix == "Tr") // Tr은 투명도이므로 반전
				{
					AlphaValue = 1.0f - AlphaValue;
				}
				CurrentMaterial.Alpha = AlphaValue;
			}

			//UE_LOG(Log, Render, "d/Tr %f", AlphaStr.ToFloat());
		}
		else if (Prefix == "map_Kd")
		{
			FString DiffuseTexturePath;
			if (!ReadToken(Cursor, DiffuseTexturePath)) { continue; }
			if (outMaterials.Num() > 0)
			{
				FObjMaterialInfo& CurrentMaterial = outMaterials[outMaterials.Num() - 1];
				CurrentMaterial.DiffuseTexturePath = DiffuseTexturePath;
			}
			//UE_LOG(Log, Render, "map_Kd %s", DiffuseTexturePath.CStr());
		}
		else if (Prefix == "map_bump" || Prefix == "bump" || Prefix == "norm")
		{
			FString NormalTexturePath;
			if (!ReadToken(Cursor, NormalTexturePath)) { continue; }
			if (outMaterials.Num() > 0)
			{
				FObjMaterialInfo& CurrentMaterial = outMaterials[outMaterials.Num() - 1];
				CurrentMaterial.NormalTexturePath = NormalTexturePath;
			}
			//UE_LOG(Log, Render, "map_bump %s", NormalTexturePath.CStr());
		}
		else if (Prefix == "map_Ks")
		{
			FString SpecularTexturePath;
			if (!ReadToken(Cursor, SpecularTexturePath)) { continue; }
			if (outMaterials.Num() > 0)
			{
				FObjMaterialInfo& CurrentMaterial = outMaterials[outMaterials.Num() - 1];
				CurrentMaterial.SpecularPath = SpecularTexturePath;
			}
			//UE_LOG(Log, Render, "map_Ks %s", SpecularTexturePath.CStr());
		}
	}

	return true;
}

void FObjImporter::convertObjToStaticMesh(const FObjInfo& objInfo, FStaticMesh& outStaticMesh)
{

	for(const FObjFace& face : objInfo.VertexIndices)
	{
		for(const FObjVertexIndex& vertexIndex : face.VertexIndices)
		{
			outStaticMesh.Vertices.Add(FNormalVertex{
				objInfo.Positions[vertexIndex.PositionIndex],
				vertexIndex.NormalIndex >= 0 ? objInfo.Normals[vertexIndex.NormalIndex] : FVector(0, 0, 1),
				FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
				vertexIndex.UVIndex >= 0 ? objInfo.UVs[vertexIndex.UVIndex] : FVector2(0, 0)
				});
		}
	}
}


