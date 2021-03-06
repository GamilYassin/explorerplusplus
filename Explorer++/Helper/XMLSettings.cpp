/******************************************************************
 *
 * Project: Helper
 * File: XMLSettings.cpp
 * License: GPL - See LICENSE in the top level directory
 *
 * XML helper functions. Intended to be used when
 * reading from/writing to an XML file.
 *
 * Written by David Erceg
 * www.explorerplusplus.com
 *
 *****************************************************************/

#include "stdafx.h"
#include "XMLSettings.h"
#include "Helper.h"
#include "Macros.h"


static const TCHAR BOOL_YES[] = _T("yes");
static const TCHAR BOOL_NO[] = _T("no");

/* Helper function to create a DOM instance. */
IXMLDOMDocument *NXMLSettings::DomFromCOM()
{
	HRESULT					hr;
	IXMLDOMDocument	*pxmldoc = NULL;

	hr = CoCreateInstance(__uuidof(DOMDocument30),NULL,CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument),reinterpret_cast<LPVOID *>(&pxmldoc));

	if(SUCCEEDED(hr))
	{
		pxmldoc->put_async(VARIANT_FALSE);
		pxmldoc->put_validateOnParse(VARIANT_FALSE);
		pxmldoc->put_resolveExternals(VARIANT_FALSE);
		pxmldoc->put_preserveWhiteSpace(VARIANT_TRUE);
	}

	return pxmldoc;
}

void NXMLSettings::WriteStandardSetting(IXMLDOMDocument *pXMLDom,
	IXMLDOMElement *pGrandparentNode, const TCHAR *szElementName,
	const TCHAR *szAttributeName, const TCHAR *szAttributeValue)
{
	IXMLDOMElement		*pParentNode = NULL;
	IXMLDOMAttribute	*pa = NULL;
	IXMLDOMAttribute	*pa1 = NULL;
	BSTR						bstr = NULL;
	BSTR						bstr_wsntt = SysAllocString(L"\n\t\t");
	VARIANT						var;

	bstr = SysAllocString(szElementName);
	pXMLDom->createElement(bstr,&pParentNode);
	SysFreeString(bstr);
	bstr = NULL;

	NXMLSettings::AddWhiteSpaceToNode(pXMLDom,bstr_wsntt,pParentNode);

	/* This will form an attribute of the form:
	name="AttributeName" */
	bstr = SysAllocString(L"name");

	var = NXMLSettings::VariantString(szAttributeName);

	pXMLDom->createAttribute(bstr,&pa);
	pa->put_value(var);
	pParentNode->setAttributeNode(pa,&pa1);
	SysFreeString(bstr);
	bstr = NULL;

	if (pa1)
	{
		pa1->Release();
		pa1 = NULL;
	}

	pa->Release();
	pa = NULL;
	VariantClear(&var);

	bstr = SysAllocString(szAttributeValue);
	pParentNode->put_text(bstr);
	SysFreeString(bstr);
	bstr = NULL;

	SysFreeString(bstr_wsntt);
	bstr_wsntt = NULL;

	NXMLSettings::AppendChildToParent(pParentNode,pGrandparentNode);

	pParentNode->Release();
	pParentNode = NULL;
}

VARIANT NXMLSettings::VariantString(const WCHAR *str)
{
	VARIANT	var;

	VariantInit(&var);
	V_BSTR(&var) = SysAllocString(str);
	V_VT(&var) = VT_BSTR;

	return var;
}

/* Helper function to append a whitespace text node to a
specified element. */
void NXMLSettings::AddWhiteSpaceToNode(IXMLDOMDocument* pDom,
	BSTR bstrWs, IXMLDOMNode *pNode)
{
	IXMLDOMText	*pws = NULL;
	IXMLDOMNode	*pBuf = NULL;

	HRESULT hr = pDom->createTextNode(bstrWs,&pws);

	if(FAILED(hr))
	{
		goto clean;
	}

	hr = pNode->appendChild(pws,&pBuf);

	if(FAILED(hr))
	{
		goto clean;
	}

clean:
	SafeRelease(&pws);
	SafeRelease(&pBuf);
}

/* Helper function to append a child to a parent node. */
void NXMLSettings::AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent)
{
	IXMLDOMNode	*pNode = NULL;
	pParent->appendChild(pChild, &pNode);
	SafeRelease(&pNode);
}

void NXMLSettings::AddAttributeToNode(IXMLDOMDocument *pXMLDom,
IXMLDOMElement *pParentNode,const WCHAR *wszAttributeName,
const WCHAR *wszAttributeValue)
{
	IXMLDOMAttribute	*pa = NULL;
	IXMLDOMAttribute	*pa1 = NULL;
	BSTR						bstr = NULL;
	VARIANT						var;

	bstr = SysAllocString(wszAttributeName);

	var = VariantString(wszAttributeValue);

	pXMLDom->createAttribute(bstr,&pa);
	pa->put_value(var);
	pParentNode->setAttributeNode(pa,&pa1);

	SysFreeString(bstr);
	bstr = NULL;

	if(pa1)
	{
		pa1->Release();
		pa1 = NULL;
	}

	pa->Release();
	pa = NULL;

	VariantClear(&var);
}

void NXMLSettings::AddStringListToNode(IXMLDOMDocument *pXMLDom,
IXMLDOMElement *pParentNode,const TCHAR *szBaseKeyName,
	const std::list<std::wstring> &strList)
{
	TCHAR szNode[64];
	int i = 0;

	for(const auto &str : strList)
	{
		StringCchPrintf(szNode,SIZEOF_ARRAY(szNode),_T("%s%d"),
			szBaseKeyName,i++);
		NXMLSettings::AddAttributeToNode(pXMLDom,pParentNode,
			szNode,str.c_str());
	}
}

void NXMLSettings::CreateElementNode(IXMLDOMDocument *pXMLDom,
	IXMLDOMElement **pParentNode,
	IXMLDOMElement *pGrandparentNode, const WCHAR *szElementName,
	const WCHAR *szAttributeName)
{
	BSTR bstrElement = NULL;
	BSTR bstrName = NULL;
	VARIANT var;
	bool varInitialized = false;
	IXMLDOMAttribute *pa = NULL;
	IXMLDOMAttribute *pa1 = NULL;
	HRESULT hr;

	bstrElement = SysAllocString(szElementName);

	if(bstrElement == NULL)
	{
		goto clean;
	}

	hr = pXMLDom->createElement(bstrElement, pParentNode);

	if(FAILED(hr))
	{
		goto clean;
	}

	bstrName = SysAllocString(L"name");

	if(bstrName == NULL)
	{
		goto clean;
	}

	var = VariantString(szAttributeName);
	varInitialized = true;

	hr = pXMLDom->createAttribute(bstrName, &pa);

	if(FAILED(hr))
	{
		goto clean;
	}

	hr = pa->put_value(var);

	if(FAILED(hr))
	{
		goto clean;
	}

	hr = (*pParentNode)->setAttributeNode(pa, &pa1);

	if(FAILED(hr))
	{
		goto clean;
	}

	AppendChildToParent(*pParentNode, pGrandparentNode);

clean:
	SafeBSTRRelease(bstrElement);
	SafeBSTRRelease(bstrName);
	if(varInitialized) { VariantClear(&var); }
	SafeRelease(&pa);
	SafeRelease(&pa1);
}

const TCHAR	*NXMLSettings::EncodeBoolValue(BOOL value)
{
	if(value)
	{
		return BOOL_YES;
	}

	return BOOL_NO;
}

BOOL NXMLSettings::DecodeBoolValue(const TCHAR *value)
{
	if(lstrcmp(value, BOOL_YES) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

WCHAR *NXMLSettings::EncodeIntValue(int iValue)
{
	static WCHAR wszDest[64];

	_itow_s(iValue,wszDest,
		SIZEOF_ARRAY(wszDest),10);

	return wszDest;
}

int NXMLSettings::DecodeIntValue(WCHAR *wszValue)
{
	return _wtoi(wszValue);
}

COLORREF NXMLSettings::ReadXMLColorData(IXMLDOMNode *pNode)
{
	IXMLDOMNode			*pChildNode = NULL;
	IXMLDOMNamedNodeMap	*am = NULL;
	BSTR						bstrName;
	BSTR						bstrValue;
	long						lChildNodes;
	BYTE						r = 0;
	BYTE						g = 0;
	BYTE						b = 0;
	long						i = 0;

	pNode->get_attributes(&am);

	am->get_length(&lChildNodes);

	/* RGB data requires three attributes (R,G,B). */
	/*if(lChildNodes != 3)*/

	/* Attribute name should be one of: r,g,b
	Attribute value should be a value between
	0x00 and 0xFF.
	Although color values have a bound, it does
	not need to be checked for, as each color
	value is a byte, and can only hold values
	between 0x00 and 0xFF. */
	for(i = 1;i < lChildNodes;i++)
	{
		am->get_item(i,&pChildNode);

		/* Element name. */
		pChildNode->get_nodeName(&bstrName);

		/* Element value. */
		pChildNode->get_text(&bstrValue);

		if(lstrcmp(bstrName,L"r") == 0)
			r = (BYTE)NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"g") == 0)
			g = (BYTE)NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"b") == 0)
			b = (BYTE)NXMLSettings::DecodeIntValue(bstrValue);
	}

	return RGB(r,g,b);
}

Gdiplus::Color NXMLSettings::ReadXMLColorData2(IXMLDOMNode *pNode)
{
	IXMLDOMNode			*pChildNode = NULL;
	IXMLDOMNamedNodeMap	*am = NULL;
	Gdiplus::Color				color;
	BSTR						bstrName;
	BSTR						bstrValue;
	long						lChildNodes;
	BYTE						r = 0;
	BYTE						g = 0;
	BYTE						b = 0;
	long						i = 0;

	pNode->get_attributes(&am);

	am->get_length(&lChildNodes);

	/* RGB data requires three attributes (R,G,B). */
	/*if(lChildNodes != 3)*/

	/* Attribute name should be one of: r,g,b
	Attribute value should be a value between 0x00 and 0xFF. */
	for(i = 1;i < lChildNodes;i++)
	{
		am->get_item(i,&pChildNode);

		/* Element name. */
		pChildNode->get_nodeName(&bstrName);

		/* Element value. */
		pChildNode->get_text(&bstrValue);

		if(lstrcmp(bstrName,L"r") == 0)
			r = (BYTE)NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"g") == 0)
			g = (BYTE)NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"b") == 0)
			b = (BYTE)NXMLSettings::DecodeIntValue(bstrValue);
	}

	color = Gdiplus::Color(r,g,b);

	return color;
}

HFONT NXMLSettings::ReadXMLFontData(IXMLDOMNode *pNode)
{
	IXMLDOMNode			*pChildNode = NULL;
	IXMLDOMNamedNodeMap	*am = NULL;
	LOGFONT						FontInfo;
	BSTR						bstrName;
	BSTR						bstrValue;
	long						lChildNodes;
	long						i = 0;

	pNode->get_attributes(&am);

	am->get_length(&lChildNodes);

	for(i = 1;i < lChildNodes;i++)
	{
		am->get_item(i,&pChildNode);

		pChildNode->get_nodeName(&bstrName);
		pChildNode->get_text(&bstrValue);

		if(lstrcmp(bstrName,L"Height") == 0)
			FontInfo.lfHeight = NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"Width") == 0)
			FontInfo.lfWidth = NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"Weight") == 0)
			FontInfo.lfWeight = NXMLSettings::DecodeIntValue(bstrValue);
		else if(lstrcmp(bstrName,L"Italic") == 0)
			FontInfo.lfItalic = (BYTE)NXMLSettings::DecodeBoolValue(bstrValue);
		else if(lstrcmp(bstrName,L"Underline") == 0)
			FontInfo.lfUnderline = (BYTE)NXMLSettings::DecodeBoolValue(bstrValue);
		else if(lstrcmp(bstrName,L"Strikeout") == 0)
			FontInfo.lfStrikeOut = (BYTE)NXMLSettings::DecodeBoolValue(bstrValue);
		else if(lstrcmp(bstrName,L"Font") == 0)
			StringCchCopy(FontInfo.lfFaceName,SIZEOF_ARRAY(FontInfo.lfFaceName),bstrValue);
	}

	FontInfo.lfWeight			= FW_MEDIUM;
	FontInfo.lfCharSet			= DEFAULT_CHARSET;
	FontInfo.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	FontInfo.lfEscapement		= 0;
	FontInfo.lfOrientation		= 0;
	FontInfo.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	FontInfo.lfPitchAndFamily	= FIXED_PITCH|FF_MODERN;
	FontInfo.lfQuality			= PROOF_QUALITY;

	return CreateFontIndirect(&FontInfo);
}

void NXMLSettings::SafeBSTRRelease(BSTR bstr)
{
	if(bstr != NULL)
	{
		SysFreeString(bstr);
	}
}