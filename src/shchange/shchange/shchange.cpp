// shchange.cpp : Copyright (c) Troy Simpson 2017
// http://www.ebswift.com
//
// This program sends a shell change notification so that after an install the
// context menu will show up

#include "stdafx.h"
#include <ShlObj.h>

int main()
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return 0;
}

