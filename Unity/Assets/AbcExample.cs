using UnityEngine;
using System.Collections;


[ExecuteInEditMode]
public class AbcExample : MonoBehaviour {

	public UnityEngine.UI.Text	mErrorText;
	public UnityEngine.UI.Text	mFilenameText;

	PopAbc			mAbc;


	void OnError(string Error)
	{
		Debug.LogError( Error );

		if ( mErrorText == null )
			return;

		mErrorText.text += Error + "\n";
	}

	public void LoadAbc()
	{
		Debug.Log("Load abc("+mFilenameText.text+")");
		LoadAbc (mFilenameText.text);

	}

	public void LoadAbc(string Filename)
	{
		mAbc = new PopAbc( Filename );

		string Meta = mAbc.GetMeta();
		Debug.Log ("Meta is " + Meta);
	}

	void Update()
	{
		PopAbc.FlushDebug (Debug.Log);
	}

}
