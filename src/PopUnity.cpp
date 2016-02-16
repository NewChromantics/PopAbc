#include "PopUnity.h"




__export Unity::sint PopAbc_GetPluginEventId()
{
	return Unity::GetPluginEventId();
}

int Unity::GetPluginEventId()
{
	return mEventId;
}


