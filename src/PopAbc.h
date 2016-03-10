#pragma once

#include "PopUnity.h"
#include "TParser.h"


__export Unity::ulong	PopAbc_Alloc(const char* Filename);
__export bool			PopAbc_Free(Unity::ulong Instance);
__export bool			PopAbc_PushRenderTexture(Unity::ulong Instance,Unity::NativeTexturePtr TextureId,Unity::sint Width,Unity::sint Height,Unity::RenderTexturePixelFormat::Type PixelFormat);
__export bool			PopAbc_PushTexture2D(Unity::ulong Instance,Unity::NativeTexturePtr TextureId,Unity::sint Width,Unity::sint Height,Unity::Texture2DPixelFormat::Type PixelFormat);
__export Unity::sint	PopAbc_PopData(Unity::ulong Instance,char* Buffer,Unity::uint BufferSize);


__export const char*	PopAbc_GetMeta(Unity::ulong Instance);

//	generic release-locked-string func to dispose of memory once it's been used
__export void			PopAbc_ReleaseString(const char* String);


__export Unity::sint			PopAbc_GetVertexCount(Unity::ulong Instance,const char* NodeName);
__export Unity::sint			PopAbc_GetTriangleCount(Unity::ulong Instance,const char* NodeName);
__export const Unity::Float*	PopAbc_LockVertexes(Unity::ulong Instance,const char* NodeName);
__export const Unity::sint*		PopAbc_LockTriangles(Unity::ulong Instance,const char* NodeName);
__export void					PopAbc_UnlockVertexes(const Unity::Float* Vertexes);
__export void					PopAbc_UnlockTriangles(const Unity::sint* Triangles);
	


namespace PopAbc
{
	class TInstance;
	typedef Unity::ulong	TInstanceRef;
	
	std::shared_ptr<TInstance>	Alloc(TParserParams Params,std::shared_ptr<Opengl::TContext> OpenglContext);
	std::shared_ptr<TInstance>	GetInstance(TInstanceRef Instance);
	bool						Free(TInstanceRef Instance);
};




class PopAbc::TInstance
{
public:
	TInstance()=delete;
	TInstance(const TInstance& Copy)=delete;
public:
	explicit TInstance(const TInstanceRef& Ref,TParserParams Params,std::shared_ptr<Opengl::TContext> OpenglContext);
	
	TInstanceRef	GetRef() const		{	return mRef;	}

	void			GetMeta(std::stringstream& Meta);
	Geo::TNode&		GetNode(const std::string& NodeName);
	
public:
	std::shared_ptr<Opengl::TContext>	mOpenglContext;
	std::shared_ptr<TParser>			mParser;

private:
	TInstanceRef	mRef;
};



