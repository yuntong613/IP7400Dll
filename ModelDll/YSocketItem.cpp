#include "StdAfx.h"
#include "YSocketItem.h"
#include "common.h"

YSocketItem::YSocketItem(DWORD dwItemId,LPCTSTR sItemName,LPCTSTR sItemDesc)
{
	m_bReadOnly = false;
	CString strDevName;
	strDevName.LoadString(IDS_DEVICENAME);
	SetDeviceName(strDevName);
	m_dwItemID = dwItemId;
	m_strItemName = sItemName;
	m_strItemDescription = sItemDesc;
}

YSocketItem::~YSocketItem(void)
{
}

YBoolItem::YBoolItem(DWORD dwItemId,LPCSTR sItemName,LPCSTR sItemDesc)
:YSocketItem(dwItemId,sItemName,sItemDesc)
{
	SetVarType(YSocketItem::vtBool);
	SetVarValue(CString(_T("FALSE")));
}
YBoolItem::~YBoolItem()
{
}
bool YBoolItem::OnUpdate(CString strVal)
{
	if(m_vItemValue.vt != VT_BOOL) return false;
	HWND hWnd;
	SetVarValue(strVal);
	GET_EXE_HWND(hWnd);
	::SendMessage(hWnd,CW_UPDATEITEMVALUE,DIC_ITEM_UPDATE,(LPARAM)this);
	return true;
}

bool YBoolItem::GetBoolValue()
{
	if(m_vItemValue.vt != VT_BOOL) return 0;
	bool bRet = m_vItemValue.bVal ? true : false;
	return bRet;
}

YShortItem::YShortItem(DWORD dwItemId,LPCSTR sItemName,LPCSTR sItemDesc)
:YSocketItem(dwItemId,sItemName,sItemDesc)
{
	SetVarType(YSocketItem::vtShort);
	SetVarValue(CString(_T("0")));
}

YShortItem::~YShortItem()
{
}

bool YShortItem::OnUpdate(CString strVal)
{
	if(m_vItemValue.vt != VT_I2) return false;
	HWND hWnd;
	SetVarValue(strVal);
	GET_EXE_HWND(hWnd);
	::SendMessage(hWnd,CW_UPDATEITEMVALUE,DIC_ITEM_UPDATE,(LPARAM)this);
	return true;
}

YFloatItem::YFloatItem(DWORD dwItemId,LPCSTR sItemName,LPCSTR sItemDesc)
:YSocketItem(dwItemId,sItemName,sItemDesc)
{
	SetVarType(YSocketItem::vtFloat);
	SetVarValue(CString(_T("0.0")));
}

YFloatItem::~YFloatItem()
{
}

bool YFloatItem::OnUpdate(CString strVal)
{
	if(m_vItemValue.vt != VT_R4) return false;
	HWND hWnd;
	SetVarValue(strVal);
	GET_EXE_HWND(hWnd);
	::SendMessage(hWnd,CW_UPDATEITEMVALUE,DIC_ITEM_UPDATE,(LPARAM)this);
	return true;
}

float YFloatItem::GetFloatValue()
{
	if(m_vItemValue.vt != VT_R4) return 0;
	float fVal = m_vItemValue.fltVal;
	return fVal;
}

YStringItem::YStringItem(DWORD dwItemId,LPCSTR sItemName,LPCSTR sItemDesc)
:YSocketItem(dwItemId,sItemName,sItemDesc)
{
	SetVarType(YSocketItem::vtString);
	SetVarValue(CString(_T("null")));
}

YStringItem::~YStringItem()
{
}

bool YStringItem::OnUpdate(CString strVal)
{
	if(m_vItemValue.vt != VT_BSTR) return false;
	CString strOldVal;
	strOldVal = GetStringValue();
	if(strOldVal == strVal)return false;
	HWND hWnd;
	SetVarValue(strVal);
	GET_EXE_HWND(hWnd);
	::SendMessage(hWnd,CW_UPDATEITEMVALUE,DIC_ITEM_UPDATE,(LPARAM)this);
	return true;
}

CString YStringItem::GetStringValue()
{
	if(m_vItemValue.vt != VT_BSTR) return false;
	CString strVal;
	FormatVariant(strVal);
	return strVal;
}
