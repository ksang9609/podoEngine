//
//#define REFLECT_CLASS(className, superClassName)									\
//public:																				\
//	static const FClassInfo* GetClass()												\
//	{																				\
//		static FClassInfo classInstance = FClassInfo(								\
//			#className,																\
//			superClassName::GetClass(),												\
//			[]() -> UObject* {														\
//				UObject* instance = new className();								\
//				return instance;													\
//			}																		\
//		);																			\
//		return &classInstance;														\
//	}																				\
//private:

#define DECLARE_OBJECT(ClassName, ParentName) \
public: \
    static FClassInfo ClassInfo; \
    static const FClassInfo* GetClass() \
    { \
        return &ClassInfo; \
    }\
private:

#define IMPLEMENT_CLASS(className, superClassName) \
FClassInfo className::ClassInfo( \
    #className, \
    superClassName::GetClass(), \
    []() -> UObject* \
    { \
        return new className(); \
    } \
);

template<typename TObject>
	requires std::derived_from<TObject, UObject>
bool UObject::IsA() const
{
	return IsA(TObject::GetClass());
}

template<typename TObject>
	requires std::derived_from<TObject, UObject>
TObject* UObject::Cast()
{
	if (IsA<TObject>())
	{
		return static_cast<TObject*>(this);
	}
	return nullptr;
}

template<typename TObject>
	requires std::derived_from<TObject, UObject>
TObject* UObject::GetObjectByUUID(int32 uuid)
{
	UObject* object = GetObjectByUUID(uuid);
	if (object && object->IsA<TObject>())
	{
		return static_cast<TObject*>(object);
	}
	return nullptr;
}

template<typename TObject>
	requires std::derived_from<TObject, UObject>
TObject* UObject::GetObjectByInternalIndex(uint32 internalIndex)
{
	UObject* object = GetObjectByInternalIndex(internalIndex);
	if (object && object->IsA<TObject>())
	{
		return static_cast<TObject*>(object);
	}
	return nullptr;
}
