#pragma once

#include "PopUnity.h"



__export Unity::ulong	PopAbc_Alloc(const char* Filename);
__export bool			PopAbc_Free(Unity::ulong Instance);
__export bool			PopAbc_PushRenderTexture(Unity::ulong Instance,Unity::NativeTexturePtr TextureId,Unity::sint Width,Unity::sint Height,Unity::RenderTexturePixelFormat::Type PixelFormat);
__export bool			PopAbc_PushTexture2D(Unity::ulong Instance,Unity::NativeTexturePtr TextureId,Unity::sint Width,Unity::sint Height,Unity::Texture2DPixelFormat::Type PixelFormat);
__export Unity::sint	PopAbc_PopData(Unity::ulong Instance,char* Buffer,Unity::uint BufferSize);



namespace PopAbc
{
	class TInstance;
	typedef Unity::ulong	TInstanceRef;
	
	class TParser;
	class TParams;

	std::shared_ptr<TInstance>	Alloc(TParams Params,std::shared_ptr<Opengl::TContext> OpenglContext);
	std::shared_ptr<TInstance>	GetInstance(TInstanceRef Instance);
	bool						Free(TInstanceRef Instance);
};




class PopAbc::TParser
{
public:
};

class PopAbc::TParams
{
public:
};



class PopAbc::TInstance
{
public:
	TInstance()=delete;
	TInstance(const TInstance& Copy)=delete;
public:
	explicit TInstance(const TInstanceRef& Ref,TParams Params,std::shared_ptr<Opengl::TContext> OpenglContext);
	
	TInstanceRef	GetRef() const		{	return mRef;	}

	void			PushTexture(Opengl::TTexture Texture);
	void			PopData(std::stringstream& Data);
	
public:
	std::shared_ptr<Opengl::TContext>	mOpenglContext;
	std::shared_ptr<TParser>			mParser;

private:
	TInstanceRef	mRef;
};



