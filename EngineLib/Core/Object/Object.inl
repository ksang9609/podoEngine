#define CONCAT_IMPL(a, b) a##b    // 매크로 확장
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define DECLARE_OBJECT(ClassName, ParentName)                                    \
public:                                                                          \
    static FClassInfo ClassInfo;                                                 \
    static const FClassInfo* GetClass()                                          \
    {                                                                            \
        return &ClassInfo;                                                       \
    }             

#define IMPLEMENT_CLASS(className, superClassName)                               \
[[maybe_unused]] static constexpr int CONCAT(RTTI_Tag_, className) = 0;          \
FClassInfo className::ClassInfo(                                                 \
    #className,                                                                  \
    superClassName::GetClass(),                                                  \
    []() -> UObject*                                                             \
    {                                                                            \
        return new className();                                                  \
    }                                                                            \
);

//test
#define IMPLEMENT_CLASS_WITH_PROPERTIES(className, superClassName)               \
[[maybe_unused]] static constexpr int CONCAT(RTTI_Tag_, className) = 0;          \
FClassInfo className::ClassInfo(                                                 \
    #className,                                                                  \
    superClassName::GetClass(),                                                  \
    []() -> UObject*                                                             \
    {                                                                            \
        return new className();                                                  \
    },                                                                           \
    className::GetDeclaredProperties()                                           \
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
const TObject* UObject::Cast() const
{
	if (IsA<TObject>())
	{
		return static_cast<const TObject*>(this);
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
