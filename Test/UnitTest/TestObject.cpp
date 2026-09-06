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

	class UMockObjectParameter : public UObject
	{
		REFLECT_CLASS(UMockObjectParameter, UObject);

	public:
		void Initialize(int32 param1, int32 param2)
		{
			Param1 = param1;
			Param2 = param2;
		}

		int32 Param1;
		int32 Param2;
	};

	TEST(TestUObject, WhenCreatingInstance_ReturnsCorrectType)
	{
		UObject* obj = FObjectFactory::ConstructObject(UMockObject::GetClass());
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
		UObject* obj = FObjectFactory::ConstructObject(UMockObject::GetClass());
		UObject* objChildA = FObjectFactory::ConstructObject(UMockObjectChildA::GetClass());
		UObject* objChildB = FObjectFactory::ConstructObject(UMockObjectChildB::GetClass());

		EXPECT_TRUE(obj->IsA(UMockObject::GetClass()));
		EXPECT_TRUE(objChildA->IsA(UMockObjectChildA::GetClass()));
		EXPECT_TRUE(objChildB->IsA(UMockObjectChildB::GetClass()));

		delete obj;
		delete objChildA;
		delete objChildB;
	}

	TEST(TestIsA, WhenInputIsParentClass_ReturnsTrue)
	{
		UObject* objChildA = FObjectFactory::ConstructObject(UMockObjectChildA::GetClass());
		UObject* objChildB = FObjectFactory::ConstructObject(UMockObjectChildB::GetClass());

		EXPECT_TRUE(objChildA->IsA(UMockObject::GetClass()));
		EXPECT_TRUE(objChildB->IsA(UMockObject::GetClass()));
		EXPECT_TRUE(objChildA->IsA(UObject::GetClass()));
		EXPECT_TRUE(objChildB->IsA(UObject::GetClass()));

		delete objChildA;
		delete objChildB;
	}

	TEST(TestIsA, WhenInputIsUnrelatedClass_ReturnsFalse)
	{
		UObject* obj = FObjectFactory::ConstructObject(UMockObject::GetClass());
		UObject* objChildA = FObjectFactory::ConstructObject(UMockObjectChildA::GetClass());
		UObject* objChildB = FObjectFactory::ConstructObject(UMockObjectChildB::GetClass());

		EXPECT_FALSE(obj->IsA(UMockObjectChildA::GetClass()));
		EXPECT_FALSE(obj->IsA(UMockObjectChildB::GetClass()));
		EXPECT_FALSE(objChildA->IsA(UMockObjectChildB::GetClass()));
		EXPECT_FALSE(objChildB->IsA(UMockObjectChildA::GetClass()));

		delete obj;
		delete objChildA;
		delete objChildB;
	}

	TEST(TestConstructObject, WhenConstructing_ReturnsCorrectType)
	{
		UObject* obj = FObjectFactory::ConstructObject(UMockObject::GetClass());
		EXPECT_EQ(obj->GetRuntimeClass()->Name, UMockObject::GetClass()->Name);
		delete obj;
	}

	TEST(TestConstructObject, WhenConstructingT_ReturnsCorrectType)
	{
		UMockObject* obj = FObjectFactory::ConstructObject<UMockObject>();
		EXPECT_EQ(obj->GetRuntimeClass(), UMockObject::GetClass());
		delete obj;
	}

	TEST(TestConstructObject, WhenConstructingTWithParameters_ReturnsCorrectTypeAndParameters)
	{
		int32 param1 = 42;
		int32 param2 = 84;
		UMockObjectParameter* obj = FObjectFactory::ConstructObject<UMockObjectParameter>(param1, param2);

		EXPECT_EQ(obj->GetRuntimeClass(), UMockObjectParameter::GetClass());
		EXPECT_EQ(obj->Param1, param1);
		EXPECT_EQ(obj->Param2, param2);

		delete obj;
	}

}
