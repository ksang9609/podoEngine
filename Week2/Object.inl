template<typename TObject, typename... Args>
	requires std::derived_from<TObject, UObject>
static TObject* FObjectFactory::ConstructObject(Args&& ...args)
{
	static_assert(requires(TObject * obj)
	{
		obj->Initialize(std::forward<Args>(args)...);

	}, "TObject must have an Initialize method that accepts the provided arguments.");

	FClassInfo* classInfo = TObject::GetClass();
	if (!classInfo || !classInfo->Constructor)
	{
		return nullptr;
	}

	TObject* instance = static_cast<TObject*>(classInfo->CreateInstance());
	if (instance)
	{
		instance->Initialize(std::forward<Args>(args)...);
	}
	return instance;
}

template<typename TObject>
bool UObject::IsA() const
{
	return IsA(TObject::GetClass());
}
