#pragma once

#include <objbase.h>
#include <MsXml2.h>

namespace NXMLSettings
{
	IXMLDOMDocument	*DomFromCOM();
	void	WriteStandardSetting(IXMLDOMDocument *pXMLDom,IXMLDOMElement *pGrandparentNode,
		const TCHAR *szElementName,const TCHAR *szAttributeName,const TCHAR *szAttributeValue);
	VARIANT VariantString(const WCHAR *str);
	void	AddWhiteSpaceToNode(IXMLDOMDocument* pDom,BSTR bstrWs,IXMLDOMNode *pNode);
	void	AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent);
	void	AddAttributeToNode(IXMLDOMDocument *pXMLDom,IXMLDOMElement *pParentNode,
		const WCHAR *wszAttributeName,const WCHAR *wszAttributeValue);
	void	AddStringListToNode(IXMLDOMDocument *pXMLDom,IXMLDOMElement *pParentNode,
		const TCHAR *szBaseKeyName,const std::list<std::wstring> &strList);
	void	CreateElementNode(IXMLDOMDocument *pXMLDom,IXMLDOMElement **pParentNode,
		IXMLDOMElement *pGrandparentNode,const WCHAR *szElementName,const WCHAR *szAttributeName);
	const TCHAR	*EncodeBoolValue(BOOL value);
	BOOL	DecodeBoolValue(const TCHAR *value);
	WCHAR	*EncodeIntValue(int iValue);
	int		DecodeIntValue(WCHAR *wszValue);
	COLORREF	ReadXMLColorData(IXMLDOMNode *pNode);
	Gdiplus::Color	ReadXMLColorData2(IXMLDOMNode *pNode);
	HFONT	ReadXMLFontData(IXMLDOMNode *pNode);

	void	SafeBSTRRelease(BSTR bstr);
}