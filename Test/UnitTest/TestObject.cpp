#include "pch.h"

#include "Object.h"

namespace UObjectTest
{

	class UMockObject : public UObject
	{
		REFLECT_CLASS(UMockObject, UObject);
	};

	class UMockObjectChildA : public UMockObject
	{
		REFLECT_CLASS(UMockObjectChildA, UMockObject);
	};

	class UMockObjectChildB : public UMockObject
	{
		REFLECT_CLASS(UMockObjectChildB, UMockObject);
	};

	TEST(TestUObject, WhenCreatingInstance_ReturnsCorrectType)
	{
		UObject* obj = new UMockObject();
		FClassInfo* classInfo = obj->GetRuntimeClass();

		EXPECT_EQ(classInfo->Name, "UMockObject");
		EXPECT_EQ(classInfo->SuperClass->Name, "UObject");

		delete obj;
	}

	TEST(TestGetClass, WhenFunctionCalled_ReturnsCorrectClassInfo)
	{
		FClassInfo* uMockObjectInfo = UMockObject::GetClass();
		FClassInfo* uObjectInfo = UObject::GetClass();

		EXPECT_EQ(uMockObjectInfo->Name, "UMockObject");
		EXPECT_EQ(uMockObjectInfo->SuperClass, uObjectInfo);
		EXPECT_EQ(uObjectInfo->Name, "UObject");
		EXPECT_EQ(uObjectInfo->SuperClass, nullptr);
	}

	TEST(TestIsA, WhenInputIsItself_ReturnsTrue)
	{
		UObject* obj = new UMockObject();
		UObject* objChildA = new UMockObjectChildA();
		UObject* objChildB = new UMockObjectChildB();

		EXPECT_TRUE(obj->IsA(UMockObject::GetClass()));
		EXPECT_TRUE(objChildA->IsA(UMockObjectChildA::GetClass()));
		EXPECT_TRUE(objChildB->IsA(UMockObjectChildB::GetClass()));

		delete obj, objChildA, objChildB;
	}

	TEST(TestIsA, WhenInputIsParentClass_ReturnsTrue)
	{
		UObject* objChildA = new UMockObjectChildA();
		UObject* objChildB = new UMockObjectChildB();

		EXPECT_TRUE(objChildA->IsA(UMockObject::GetClass()));
		EXPECT_TRUE(objChildB->IsA(UMockObject::GetClass()));
		EXPECT_TRUE(objChildA->IsA(UObject::GetClass()));
		EXPECT_TRUE(objChildB->IsA(UObject::GetClass()));

		delete objChildA, objChildB;
	}

	TEST(TestIsA, WhenInputIsUnrelatedClass_ReturnsFalse)
	{
		UObject* obj = new UMockObject();
		UObject* objChildA = new UMockObjectChildA();
		UObject* objChildB = new UMockObjectChildB();
		EXPECT_FALSE(obj->IsA(UMockObjectChildA::GetClass()));
		EXPECT_FALSE(obj->IsA(UMockObjectChildB::GetClass()));
		EXPECT_FALSE(objChildA->IsA(UMockObjectChildB::GetClass()));
		EXPECT_FALSE(objChildB->IsA(UMockObjectChildA::GetClass()));
		delete obj, objChildA, objChildB;
	}
}
