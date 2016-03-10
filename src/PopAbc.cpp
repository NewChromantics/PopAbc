#include "PopAbc.h"
#include "PopUnity.h"
#include "SoyAlembic.h"


template<typename TYPE,typename EXTERNALTYPE>
class TExternalLockManager
{
public:
	TExternalLockManager() :
		mHeap		( true, true, "TExternalLockManager" ),
		mElements	( mHeap )
	{
	}

	const EXTERNALTYPE*					Lock(const TYPE& Item);
	void								Unlock(const EXTERNALTYPE* Element);

public:
	prmem::Heap							mHeap;
	std::mutex							mElementsLock;
	Array<TYPE>							mElements;
};

template<typename EXTERNALTYPE,typename TYPE>
const EXTERNALTYPE* ExternalLockCast(const TYPE& Item)
{
	throw Soy::AssertException("Unhandled case");
}

template<>
const char* ExternalLockCast<char>(const std::string& Item)
{
	return Item.c_str();
}

template<>
const float* ExternalLockCast<float>(const std::shared_ptr<Array<vec3f>>& Item)
{
	return &Item->GetArray()->x;
}

template<>
const Unity::sint* ExternalLockCast<Unity::sint>(const std::shared_ptr<Array<Geo::TTriangle>>& Item)
{
	return &Item->GetArray()->x;
}

bool operator==(const std::shared_ptr<Array<vec3f>>& Array,const float* ArrayRef)
{
	return &Array->GetArray()->x == ArrayRef;
}

bool operator==(const std::shared_ptr<Array<Geo::TTriangle>>& Array,const Unity::sint* ArrayRef)
{
	return &Array->GetArray()->x == ArrayRef;
}

namespace PopAbc
{
	std::mutex gInstancesLock;
	std::vector<std::shared_ptr<PopAbc::TInstance> >	gInstances;

	std::shared_ptr<TExternalLockManager<std::string,char>>						gStringManager;
	std::shared_ptr<TExternalLockManager<std::shared_ptr<Array<vec3f>>,float>>					gVertexArrayManager;
	std::shared_ptr<TExternalLockManager<std::shared_ptr<Array<Geo::TTriangle>>,Unity::sint>>	gTriangleArrayManager;

	TExternalLockManager<std::string,char>&										GetStringManager();
	TExternalLockManager<std::shared_ptr<Array<vec3f>>,float>&					GetVertexArrayManager();
	TExternalLockManager<std::shared_ptr<Array<Geo::TTriangle>>,Unity::sint>&	GetTriangleArrayManager();
};



TExternalLockManager<std::string,char>& PopAbc::GetStringManager()
{
	if ( !gStringManager )
		gStringManager.reset( new TExternalLockManager<std::string,char>() );
	return *gStringManager;
}

TExternalLockManager<std::shared_ptr<Array<vec3f>>,float>& PopAbc::GetVertexArrayManager()
{
	if ( !gVertexArrayManager )
		gVertexArrayManager.reset( new TExternalLockManager<std::shared_ptr<Array<vec3f>>,float>() );
	return *gVertexArrayManager;
}

TExternalLockManager<std::shared_ptr<Array<Geo::TTriangle>>,Unity::sint>& PopAbc::GetTriangleArrayManager()
{
	if ( !gTriangleArrayManager )
		gTriangleArrayManager.reset( new TExternalLockManager<std::shared_ptr<Array<Geo::TTriangle>>,Unity::sint>() );
	return *gTriangleArrayManager;
}


template<typename TYPE,typename EXTERNALTYPE>
const EXTERNALTYPE* TExternalLockManager<TYPE,EXTERNALTYPE>::Lock(const TYPE& Item)
{
	std::lock_guard<std::mutex> Lock( mElementsLock );
	
	auto& NewItem = mElements.PushBack( Item );
	return ExternalLockCast<EXTERNALTYPE>(NewItem);
}

template<typename TYPE,typename EXTERNALTYPE>
void TExternalLockManager<TYPE,EXTERNALTYPE>::Unlock(const EXTERNALTYPE* Element)
{
	std::lock_guard<std::mutex> Lock( mElementsLock );

	auto Index = mElements.FindIndex( Element );
	if ( Index == -1 )
		return;

	mElements.RemoveBlock( Index, 1 );
}


__export void PopAbc_ReleaseString(const char* String)
{
	auto& StringManager = PopAbc::GetStringManager();
	StringManager.Unlock( String );
}


Geo::TNode& GetNode(Unity::ulong Instance,const char* NodeName)
{
	auto pInstance  = PopAbc::GetInstance( Instance );
	Soy::Assert( pInstance!=nullptr, "Instance not found");
	Soy::Assert( NodeName, "null node name");
	return pInstance->GetNode( NodeName );
}

__export Unity::sint		PopAbc_GetVertexCount(Unity::ulong Instance,const char* NodeName)
{
	try
	{
		auto& Node = GetNode( Instance, NodeName );
		return Node.mVertexPositions.GetSize();
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return -1;
	}
}

__export Unity::sint		PopAbc_GetTriangleCount(Unity::ulong Instance,const char* NodeName)
{
	try
	{
		auto& Node = GetNode( Instance, NodeName );
		return Node.mTriangles.GetSize();
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return -1;
	}
}

__export const Unity::Float*		PopAbc_LockVertexes(Unity::ulong Instance,const char* NodeName)
{
	try
	{
		auto& Node = GetNode( Instance, NodeName );
		auto& VertexPositions = Node.GetVertexPositionsPtr();
		if ( VertexPositions->IsEmpty() )
			return nullptr;
		return PopAbc::GetVertexArrayManager().Lock( VertexPositions );
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return nullptr;
	}
}

__export const Unity::sint*		PopAbc_LockTriangles(Unity::ulong Instance,const char* NodeName)
{
	try
	{
		auto& Node = GetNode( Instance, NodeName );
		auto& Triangles = Node.GetTrianglesPtr();
		if ( Triangles->IsEmpty() )
			return nullptr;
		return PopAbc::GetTriangleArrayManager().Lock( Triangles );
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return nullptr;
	}
}

__export void				PopAbc_UnlockVertexes(const Unity::Float* Vertexes)
{
	try
	{
		return PopAbc::GetVertexArrayManager().Unlock( Vertexes );
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return;
	}
}

__export void				PopAbc_UnlockTriangles(const Unity::sint* Triangles)
{
	try
	{
		return PopAbc::GetTriangleArrayManager().Unlock( Triangles );
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return;
	}
}
	



__export Unity::ulong	PopAbc_Alloc(const char* Filename)
{
	TParserParams Params;
	Params.mFilename = Filename;
	try
	{
		auto OpenglContext = Unity::GetOpenglContextPtr();
		
		auto Instance = PopAbc::Alloc( Params, OpenglContext );
		if ( !Instance )
			return 0;
		return Instance->GetRef();
	}
	catch ( std::exception& e )
	{
		std::Debug << "Failed to allocate movie: " << e.what() << std::endl;
		return 0;
	}
	catch (...)
	{
		std::Debug << "Failed to allocate movie: Unknown exception in " << __func__ << std::endl;
		return 0;
	}
	
	return 0;
}

__export bool	PopAbc_Free(Unity::ulong Instance)
{
	ofScopeTimerWarning Timer(__func__, Unity::mMinTimerMs );
	return PopAbc::Free( Instance );
}


std::shared_ptr<PopAbc::TInstance> PopAbc::Alloc(TParserParams Params,std::shared_ptr<Opengl::TContext> OpenglContext)
{
	gInstancesLock.lock();
	static TInstanceRef gInstanceRefCounter(1000);
	auto InstanceRef = gInstanceRefCounter++;
	gInstancesLock.unlock();

	if ( !OpenglContext )
		OpenglContext = Unity::GetOpenglContextPtr();
	
	std::shared_ptr<TInstance> pInstance( new TInstance(InstanceRef,Params,OpenglContext) );
	
	gInstancesLock.lock();
	gInstances.push_back( pInstance );
	gInstancesLock.unlock();

	return pInstance;
}

std::shared_ptr<PopAbc::TInstance> PopAbc::GetInstance(TInstanceRef Instance)
{
	for ( int i=0;	i<gInstances.size();	i++ )
	{
		auto& pInstance = gInstances[i];
		if ( pInstance->GetRef() == Instance )
			return pInstance;
	}
	return std::shared_ptr<PopAbc::TInstance>();
}

bool PopAbc::Free(TInstanceRef Instance)
{
	gInstancesLock.lock();
	for ( int i=0;	i<gInstances.size();	i++ )
	{
		auto& pInstance = gInstances[i];
		if ( pInstance->GetRef() != Instance )
			continue;
		
		if ( pInstance )
		{
			pInstance.reset();
		}
		gInstances.erase( gInstances.begin()+ i );
		gInstancesLock.unlock();
		return true;
	}
	gInstancesLock.unlock();
	return false;
}



PopAbc::TInstance::TInstance(const TInstanceRef& Ref,TParserParams Params,std::shared_ptr<Opengl::TContext> OpenglContext) :
	mRef			( Ref ),
	mOpenglContext	( OpenglContext )
{
	mParser.reset( new Alembic::TArchive( Params ) );
}

void PopAbc::TInstance::GetMeta(std::stringstream& Meta)
{
	Soy::Assert( mParser!=nullptr, "Parser expected");

	mParser->GetMeta( Meta );
}


Geo::TNode& PopAbc::TInstance::GetNode(const std::string& NodeName)
{
	Soy::Assert( mParser!=nullptr, "Parser expected");
	auto pNode = mParser->GetNode( NodeName );
	Soy::Assert( pNode!=nullptr, "Node not found");

	return *pNode;
}


__export const char*	PopAbc_GetMeta(Unity::ulong Instance)
{
	try
	{
		auto pInstance = PopAbc::GetInstance( Instance );
		if ( !pInstance )
			return nullptr;

		//	extract meta
		std::stringstream Meta;
		pInstance->GetMeta( Meta );

		auto& StringManager = PopAbc::GetStringManager();
		return StringManager.Lock( Meta.str() );
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
		return nullptr;
	}
	catch(...)
	{
		std::Debug << __func__ << " unknown exception" << std::endl;
		return nullptr;
	}
}

