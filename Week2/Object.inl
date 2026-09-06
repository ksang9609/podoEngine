
#define REFLECT_CLASS(className, superClassName)									\
public:																				\
	static const FClassInfo* GetClass()												\
	{																				\
		static FClassInfo classInstance = FClassInfo(								\
			#className,																\
			superClassName::GetClass(),												\
			[]() -> UObject* {														\
				UObject* instance = new className();								\
				instance->Initialize();												\
				return instance;													\
			}																		\
		);																			\
		return &classInstance;														\
	}																				\
private:

template<typename TObject, typename... Args>
	requires std::derived_from<TObject, UObject>
static TObject* FObjectFactory::ConstructObject(Args&& ...args)
{
	static_assert(requires(TObject * obj)
	{
		obj->Initialize(std::forward<Args>(args)...);

	}, "TObject must have an Initialize method that accepts the provided arguments.");

	const FClassInfo* classInfo = TObject::GetClass();
	if (!classInfo || !classInfo->Constructor)
	{
		return nullptr;
	}

	TObject* instance = static_cast<TObject*>(classInfo->CreateInstance());
	if (instance)
	{
		instance->mClassInfo = classInfo;
		instance->Initialize(std::forward<Args>(args)...);
	}
	return instance;
}

template<typename TObject>
bool UObject::IsA() const
{
	return IsA(TObject::GetClass());
}
