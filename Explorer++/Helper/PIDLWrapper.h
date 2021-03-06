#pragma once

#include "UniqueHandle.h"
#include <ShlObj.h>

struct PIDLTraits
{
	typedef LPITEMIDLIST pointer;

	static LPITEMIDLIST invalid()
	{
		return nullptr;
	}

	static void close(LPITEMIDLIST value)
	{
		CoTaskMemFree(value);
	}
};

typedef unique_handle<PIDLTraits> PIDLPointer;