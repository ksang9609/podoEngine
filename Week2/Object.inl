
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

template<typename TObject>
bool UObject::IsA() const
{
	return IsA(TObject::GetClass());
}
