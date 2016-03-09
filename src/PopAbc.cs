using UnityEngine;
using System.Collections;					// required for Coroutines
using System.Runtime.InteropServices;		// required for DllImport
using System;								// requred for IntPtr
using System.Text;							//	required for string builder


public class PopAbc
{
#if UNITY_STANDALONE_OSX || UNITY_EDITOR_OSX
	private const string PluginName = "PopAbcOsx";
#elif UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
	private const string PluginName = "PopAbc";
#elif UNITY_ANDROID
	private const string PluginName = "PopAbc";
#elif UNITY_IOS
	//private const string PluginName = "PopAbcIos";
	private const string PluginName = "__Internal";
#endif

	private ulong	mInstance = 0;
	private static int	mPluginEventId = PopAbc_GetPluginEventId();

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void DebugLogDelegate(string str);
	private DebugLogDelegate	mDebugLogDelegate = null;
	
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void OpenglCallbackDelegate();

	public void AddDebugCallback(DebugLogDelegate Function)
	{
		if ( mDebugLogDelegate == null ) {
			mDebugLogDelegate = new DebugLogDelegate (Function);
		} else {
			mDebugLogDelegate += Function;
		}
	}
	
	public void RemoveDebugCallback(DebugLogDelegate Function)
	{
		if ( mDebugLogDelegate != null ) {
			mDebugLogDelegate -= Function;
		}
	}
	
	void DebugLog(string Message)
	{
		if ( mDebugLogDelegate != null )
			mDebugLogDelegate (Message);
	}

	public bool IsAllocated()
	{
		return mInstance != 0;
	}
	
	[DllImport (PluginName,CallingConvention=CallingConvention.Cdecl)]
	private static extern ulong		PopAbc_Alloc(String Filename);
	
	[DllImport (PluginName)]
	private static extern bool		PopAbc_Free(ulong Instance);

	[DllImport (PluginName)]
	private static extern int		PopAbc_GetPluginEventId();

	[DllImport (PluginName)]
	private static extern bool		FlushDebug([MarshalAs(UnmanagedType.FunctionPtr)]System.IntPtr FunctionPtr);

	[DllImport (PluginName)]
	private static extern bool		PopAbc_PushTexture2D(ulong Instance,System.IntPtr TextureId,int Width,int Height,TextureFormat textureFormat);

	[DllImport (PluginName)]
	private static extern bool		PopAbc_PushRenderTexture(ulong Instance,System.IntPtr TextureId,int Width,int Height,RenderTextureFormat textureFormat);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern int		PopAbc_PopData(ulong Instance, StringBuilder Buffer, uint BufferSize);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern string	PopAbc_GetMeta(ulong Instance);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern void		PopAbc_ReleaseString(string Str);

	public PopAbc(String Filename)
	{
		mInstance = PopAbc_Alloc (Filename);

		//	if this fails, capture the flush and throw an exception
		if (mInstance == 0) {
			string AllocError = "";
			FlushDebug (
				(string Error) => {
				AllocError += Error; }
			);
			if ( AllocError.Length == 0 )
				AllocError = "No error detected";
			throw new System.Exception("PopAbc failed: " + AllocError);
		}
	}
	
	~PopAbc()
	{
		//	gr: don't quite get the destruction order here, but need to remove the [external] delegates in destructor. 
		//	Assuming external delegate has been deleted, and this garbage collection (despite being explicitly called) 
		//	is still deffered until after parent object[monobehaviour] has been destroyed (and external function no longer exists)
		mDebugLogDelegate = null;
		PopAbc_Free (mInstance);
		FlushDebug ();
	}
	
	void FlushDebug()
	{
		FlushDebug (mDebugLogDelegate);
	}

	public static void FlushDebug(DebugLogDelegate Callback)
	{
		//	if we have no listeners, do fast flush
		bool HasListeners = (Callback != null) && (Callback.GetInvocationList ().Length > 0);
		if (HasListeners) {
			//	IOS (and aot-only platforms cannot get a function pointer. Find a workaround!
#if UNITY_IOS && !UNITY_EDITOR
			FlushDebug (System.IntPtr.Zero);
#else
			FlushDebug (Marshal.GetFunctionPointerForDelegate (Callback));
#endif
		} else {
			FlushDebug (System.IntPtr.Zero);
		}
	}

	public void UpdateTexture(Texture Target)
	{
		Update ();

		if (Target is RenderTexture) {
			RenderTexture Target_rt = Target as RenderTexture;
			PopAbc_PushRenderTexture (mInstance, Target_rt.GetNativeTexturePtr(), Target.width, Target.height, Target_rt.format );
		}
		if (Target is Texture2D) {
			Texture2D Target_2d = Target as Texture2D;
			PopAbc_PushTexture2D (mInstance, Target_2d.GetNativeTexturePtr(), Target.width, Target.height, Target_2d.format );
		}
		FlushDebug ();
	}

	private void Update()
	{
		GL.IssuePluginEvent (mPluginEventId);
		FlushDebug();
	}

	public string PopData()
	{
		//	gr: through trial and error, windows 7, unity 5.20f3 doesn't modify the buffer if the buffersize is over N(not 641 as first thought)
		StringBuilder StringBuffer = new StringBuilder(256);
		var OrigLength = PopAbc_PopData( mInstance, StringBuffer, (uint)StringBuffer.Capacity );

		//	out of sources (negative on error)
		if (OrigLength <= 0)
			return null;

		string Source = StringBuffer.ToString();

		//	warning if we clipped!
		if (OrigLength > StringBuffer.Capacity)
			Debug.LogWarning("Filename from PopMovie_EnumSource was clipped; " + StringBuffer.Capacity + "/" + OrigLength + "; " + Source);

		return Source;
	}

	static public string GetVersion()
	{
		return "GIT_REVISION";
	}


	public string GetMeta()
	{
		string MetaStringC = PopAbc_GetMeta(mInstance);
		if ( MetaStringC == null )
			return null;

		//	copy string
		string MetaString = MetaStringC;
		PopAbc_ReleaseString(MetaStringC);
		return MetaString;
	}

}

