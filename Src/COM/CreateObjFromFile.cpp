//*********************************************************************
//*
//*	File:  CreateObjFromFile.cpp
//*
//*	DESC: 
//* 
//*
//* 


#include "stdafx.h"

#include "CreateObjFromFile.h"




//* define the prototype for the class factory entry point in a COM module
typedef HRESULT (STDAPICALLTYPE* FUNC_DLLGETCLASSOBJECT) (REFCLSID ClsIF, REFIID iid, void** ppv);

HRESULT CreateObjFromFile(REFCLSID ClsID, IUnknown** ppUnk, TCHAR* pPath)
{
	//* Load the module (DLL/EXE) directly
	HMODULE hLib = NULL;
	
	hLib = LoadLibrary(pPath);
	if (hLib == NULL)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	FUNC_DLLGETCLASSOBJECT pFn = (FUNC_DLLGETCLASSOBJECT) GetProcAddress(hLib, "DllGetClassObject");
	if (pFn == NULL)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	//* Create a class factory
	IUnknownPtr pUnk;
	HRESULT hr = pFn(ClsID,  IID_IUnknown,  (void**) ((IUnknown**) &pUnk));
	if (SUCCEEDED(hr))
	{
		IClassFactoryPtr pCF = pUnk;
		if (pCF == NULL)
		{
			hr = E_NOINTERFACE;
		}
		else
		{
			// ask the class factory to create the object
			hr = pCF->CreateInstance(NULL, IID_IUnknown, (void**) ppUnk);
		}
	}

	return hr;
}
