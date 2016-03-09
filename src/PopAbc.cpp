#include "PopAbc.h"
#include "PopUnity.h"
#include "SoyAlembic.h"


class TStringManager;


namespace PopAbc
{
	std::mutex gInstancesLock;
	std::vector<std::shared_ptr<PopAbc::TInstance> >	gInstances;

	std::shared_ptr<TStringManager>		gStringManager;
	TStringManager&						GetStringManager();
};



class TStringManager
{
public:
	TStringManager() :
		mHeap		( true, true, "StringManager" ),
		mStrings	( mHeap )
	{
	}

	const char*							LockString(const std::string& String);
	void								UnlockString(const char* String);

public:
	prmem::Heap							mHeap;
	std::mutex							mStringsLock;
	Array<std::string>					mStrings;
};



TStringManager& PopAbc::GetStringManager()
{
	if ( !gStringManager )
		gStringManager.reset( new TStringManager );
	return *gStringManager;
}

const char* TStringManager::LockString(const std::string& String)
{
	std::lock_guard<std::mutex> Lock( mStringsLock );
	
	auto& NewString = mStrings.PushBack( String );
	return NewString.c_str();
}

void TStringManager::UnlockString(const char* String)
{
	std::lock_guard<std::mutex> Lock( mStringsLock );

	auto Index = mStrings.FindIndex( String );
	if ( Index == -1 )
		return;

	mStrings.RemoveBlock( Index, 1 );
}


__export void PopAbc_ReleaseString(const char* String)
{
	auto& StringManager = PopAbc::GetStringManager();
	StringManager.UnlockString( String );
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



__export bool	PopAbc_PushRenderTexture(Unity::ulong Instance,Unity::NativeTexturePtr TextureId,Unity::sint Width,Unity::sint Height,Unity::RenderTexturePixelFormat::Type PixelFormat)
{
	auto pInstance = PopAbc::GetInstance( Instance );
	if ( !pInstance )
		return false;

	try
	{
		//	assuming type atm... maybe we can extract it via opengl?
		SoyPixelsMeta Meta( Width, Height, Unity::GetPixelFormat( PixelFormat ) );
		GLenum Type = GL_TEXTURE_2D;
		Opengl::TTexture Texture( TextureId, Meta, Type );
		pInstance->PushTexture( Texture );
		return true;
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " failed: " << e.what() << std::endl;
		return false;
	}
}


__export bool	PopAbc_PushTexture2D(Unity::ulong Instance,Unity::NativeTexturePtr TextureId,Unity::sint Width,Unity::sint Height,Unity::Texture2DPixelFormat::Type PixelFormat)
{
	auto pInstance = PopAbc::GetInstance( Instance );
	if ( !pInstance )
		return false;
	
	try
	{
		//	assuming type atm... maybe we can extract it via opengl?
		SoyPixelsMeta Meta( Width, Height, Unity::GetPixelFormat( PixelFormat ) );
		GLenum Type = GL_TEXTURE_2D;
		Opengl::TTexture Texture( TextureId, Meta, Type );
		pInstance->PushTexture( Texture );
		return true;
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " failed: " << e.what() << std::endl;
		return false;
	}
}

__export Unity::sint		PopAbc_PopData(Unity::ulong Instance,char* Buffer,Unity::uint BufferSize)
{
	auto pInstance = PopAbc::GetInstance( Instance );
	if ( !pInstance )
		return -1;

	try
	{
		std::stringstream Data;
		pInstance->PopData( Data );
		
		Soy::StringToBuffer( Data.str(), Buffer, BufferSize );
		
		return size_cast<Unity::sint>( Data.str().length() );
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " failed: " << e.what() << std::endl;
		return -1;
	}
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

void PopAbc::TInstance::PushTexture(Opengl::TTexture Texture)
{
	throw Soy::AssertException("todo");
}


void PopAbc::TInstance::PopData(std::stringstream& Data)
{
	throw Soy::AssertException("todo");
}

void PopAbc::TInstance::GetMeta(std::stringstream& Meta)
{
	Meta << "I am the meta data";
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
		return StringManager.LockString( Meta.str() );
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

