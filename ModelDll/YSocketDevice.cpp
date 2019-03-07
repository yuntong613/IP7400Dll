#include "StdAfx.h"
#include "common.h"
#include ".\YSocketDevice.h"
#include "OPCIniFile.h"
#include "ItemBrowseDlg.h"
#include "YSocketItem.h"
#include "ModelDll.h"
#include "IniFile.h"
#include "BS7400Ctl.h"

extern CModelDllApp theApp;

YSocketDevice::YSocketDevice(LPCSTR pszAppPath)
{
	pObject = NULL;
	m_nUseLog = 0;

	CString strConfigFile(pszAppPath);
	strConfigFile += _T("\\BoschIP7400.ini");
	if(!InitConfigFile(strConfigFile))
	{
		AfxMessageBox("打开文件BoschIP7400.ini失败");
		return;
	}

	CString strFileName(pszAppPath);
	strFileName += _T("\\BoschIP7400.ini");
	COPCIniFile iniFile;
	if (!iniFile.Open(strFileName,CFile::modeRead|CFile::typeText))
	{
		AfxMessageBox("打开文件BoschIP7400.ini失败");
		return;
	}
	CArchive ar(&iniFile,CArchive::load);
	Serialize(ar);
}

YSocketDevice::~YSocketDevice(void)
{
	POSITION pos = y_ItemArray.GetStartPosition();
	YSocketItem* pItem = NULL;
	CString strName;
	while(pos){
		y_ItemArray.GetNextAssoc(pos,strName,(CObject*&)pItem);
		if(pItem)
		{
			delete pItem;
			pItem = NULL;
		}
	}
	y_ItemArray.RemoveAll();
}

void YSocketDevice::Serialize(CArchive& ar)
{
	if (ar.IsStoring()){
		//		Store(ar);
	}else{
		Load(ar);
	}
}

BOOL YSocketDevice::InitConfigFile(CString strFile)
{
	if(!PathFileExists(strFile))
	{
		return FALSE;
	}

	CIniFile iniFile(strFile);

	m_bUpdate = iniFile.GetBool("param","Update",FALSE);

	m_lRate = iniFile.GetUInt("param","UpdateRate",3000);

	y_lUpdateTimer = m_lRate/1000;

	m_nUseLog = iniFile.GetUInt("Log","Enable",0);

	m_strDeviceDescription = iniFile.GetString("param", "DevDesc");

	m_strLocalIP = iniFile.GetString("param", "LocalIP");

	m_strRecvPort = iniFile.GetString("param", "RecvPort");

	m_strCtrlPort = iniFile.GetString("param", "CtrlPort");

	return TRUE;
}

void YSocketDevice::Load(CArchive& ar)
{
	LoadItems(ar);
}

void YSocketDevice::LoadItems(CArchive& ar)
{
	COPCIniFile* pIniFile = static_cast<COPCIniFile*>(ar.GetFile());
	YOPCItem* pItem  = NULL;
	int nItems = 0;
	CString strTmp = ("EventItemList");
	int dwItemId = 0;
	CString strItemName,strItemDesc,strValue;

	if(pIniFile->ReadNoSeqSection(strTmp)){
		nItems = pIniFile->GetItemsCount(strTmp,"Item");
		for (int i=0;i<nItems && !pIniFile->Endof();i++ )
		{
			try{
				if (pIniFile->ReadIniItem("Item",strTmp))
				{
					if (!pIniFile->ExtractSubValue(strTmp,strValue,1))
						throw new CItemException(CItemException::invalidId,pIniFile->GetFileName());
					dwItemId = atoi(strValue);
					if(!pIniFile->ExtractSubValue(strTmp,strItemName,2))strItemName = _T("Unknown");
					if(!pIniFile->ExtractSubValue(strTmp,strItemDesc,3)) strItemDesc = _T("Unknown");
					pItem = new YShortItem(dwItemId,strItemName,strItemDesc);
					if(GetItemByName(pItem->GetName()))delete pItem;
					else y_ItemArray.SetAt(pItem->GetName(),(CObject*)pItem);
				}
			}
			catch(CItemException* e){
				if(pItem){
					delete pItem;
					pItem = NULL;
				}
				e->Delete();
			}
		}
	}
}

void YSocketDevice::OnUpdate()
{
	if(!m_bUpdate)return;
	y_lUpdateTimer--;
	if(y_lUpdateTimer>0)return;
	y_lUpdateTimer = m_lRate/1000;
/*	QueryOnce();*/
}

int YSocketDevice::QueryOnce()
{
	int nSysStatus = 0;
	int nRet = 0;
	CString IPParam;
	if (pObject != NULL && CanReceiveEvent(pObject))
	{
		nSysStatus = 1;
		for (int q = 0; q < m_arrMasterInfo.GetCount(); q++)
		{
			IPParam = m_arrMasterInfo.GetAt(q);
			nRet = Execute(pObject, IPParam.GetBuffer(IPParam.GetLength()), "QUERY_7400_STATUS", NULL);
			IPParam.ReleaseBuffer();
			if (nRet == -3)
			{
				OutPutLog("EXCUTE QUERY_7400_STATUS参数错误");
			}
			else if (nRet != 0)
			{
				OutPutLog("EXCUTE QUERY_7400_STATUS发送控制命令失败");
			}
			else {
				//	m_Log.Write("EXCUTE QUERY_7400_STATUS已成功发送控制命令");
			}

			CString strPointBlock;
			for (int i = 0; i <= 30; i++)
			{
				strPointBlock.Format("%d", i);
				nRet = Execute(pObject, IPParam.GetBuffer(IPParam.GetLength()), "QUERY_ ZONE_STATUS", strPointBlock.GetBuffer(strPointBlock.GetLength()));
				IPParam.ReleaseBuffer();
				strPointBlock.ReleaseBuffer();
				if (nRet == -3)
				{
					OutPutLog("EXCUTE QUERY_ ZONE_STATUS参数错误");
				}
				else if (nRet != 0)
				{
					OutPutLog("EXCUTE QUERY_ ZONE_STATUS发送控制命令失败");
				}
				else {
					//	m_Log.Write("EXCUTE QUERY_ ZONE_STATUS已成功发送控制命令");
				}
			}
		}
	}

	return 0;
}

void CALLBACK RecvData(LPVOID pObject, LPSTR sRcvData, int iDataLen)
{
	if (pObject != NULL && iDataLen > 0 && sRcvData != NULL)
	{
		YSocketDevice* pThis = (YSocketDevice*)pObject;
		pThis->HandleData(sRcvData, iDataLen);
	}
}

void YSocketDevice::BeginUpdateThread()
{
	pObject = New_Object();
	if (pObject)
	{
		SetLnkIntval(pObject, 10);
		CString strRecvParam, strCtrlParam;
		strRecvParam = m_strLocalIP + "|" + m_strRecvPort;
		strCtrlParam = m_strLocalIP + "|" + m_strCtrlPort;
		ArrangeRcvAddress(pObject, strRecvParam.GetBuffer(strRecvParam.GetLength()), strCtrlParam.GetBuffer(strCtrlParam.GetLength()));
		strRecvParam.ReleaseBuffer();
		strCtrlParam.ReleaseBuffer();
		DWORD dwResult = OpenReceiver(pObject, RecvData, (LPVOID)this);
		if (dwResult == 0)
		{
			OutPutLog("SDK成功设置");
		}
		else
		{
			OutPutLog("SDK设置发送接收参数失败");
		}
	}
}

void YSocketDevice::EndUpdateThread()
{
	if (pObject != NULL)
	{
		Delete_Object(pObject);
		pObject = NULL;
	}
}


void YSocketDevice::GetMessageText(LPSTR strMsg, CStringArray& arrMsg)
{
	CString strTempMsg = strMsg;
	CString strInfo;
	int nPos = 0;
	nPos = strTempMsg.Find(">");
	while (nPos > 0)
	{
		strInfo = strTempMsg.Mid(1, nPos - 1);
		strTempMsg = strTempMsg.Mid(nPos + 1, strTempMsg.GetLength() - 1);
		arrMsg.Add(strInfo);
		nPos = strTempMsg.Find(">");
	}
}

int YSocketDevice::GetMessgeInfoCount(LPSTR strMsg, int nDataLen)
{
	int nCount = 0;
	for (int i = 0; i < nDataLen; i++)
	{
		if (strMsg[i] == '<' || strMsg[i] == '>')
			nCount++;
	}
	if (nCount % 2 == 0)
		return nCount / 2;
	else
		return 0;
}

void YSocketDevice::HandleData(char* pBuf,int nLen)
{
	CString strMsg = CString(pBuf);
	OutPutLog(strMsg);
	if (GetMessgeInfoCount(pBuf, nLen))
	{
		CStringArray arrInfo;
		GetMessageText(pBuf, arrInfo);
		if (!arrInfo.IsEmpty())
		{
			int nTextCount = arrInfo.GetCount();
			if (nTextCount > 2)
			{
				CString strIPParam = arrInfo.GetAt(0);
				strIPParam.Replace("|", "-");
				CString EventCode = arrInfo.GetAt(2);
				if (EventCode == "ZONES_STATE")//防区块状态
				{
					if (nTextCount == 5)
						HandlePointStatus(strIPParam, arrInfo.GetAt(3), arrInfo.GetAt(4));
				}
				else if (EventCode == "PARTITION_STATE")//分区状态
				{
					if (nTextCount == 4)
						HandlePartitionStatus(strIPParam, arrInfo.GetAt(3));
				}
				else
				{
					if (nTextCount == 6)
						HandleEvent(strIPParam, arrInfo.GetAt(3), arrInfo.GetAt(4), arrInfo.GetAt(5));
				}
			}
		}
		arrInfo.RemoveAll();
	}
}

bool YSocketDevice::SetDeviceItemValue(CBaseItem* pAppItem)
{
	return false;
}

void YSocketDevice::OutPutLog(CString strMsg)
{
	if(m_nUseLog)
		m_Log.Write(strMsg);
}

void YSocketDevice::HandlePointStatus(CString strIPParam, CString PointBlockNum, CString strStatus)
{
	int nBlockNum = atoi(PointBlockNum);
	int nIndex = nBlockNum * 8;
	CString strItemName;
	YOPCItem* pItem = NULL;
	for (int i = 0; i < 8; i++)
	{
		strItemName.Format("%s-PointBlockStatus%d", strIPParam, nIndex + i + 1);
		pItem = GetItemByName(strItemName);
		if (pItem)
		{
			CString strValue;
			if (strStatus.GetAt(i) == 'F')
				strValue = "-1";
			else
				strValue = CString(strStatus.GetAt(i));
			pItem->OnUpdate(strValue);
		}
	}
}

void YSocketDevice::HandlePartitionStatus(CString strIPParam, CString strStatus)
{
	CString strItemName;
	YOPCItem* pItem = NULL;
	for (int i = 0; i < 8; i++)
	{
		strItemName.Format("%s-PartitionStatus%d", strIPParam, i + 1);
		pItem = GetItemByName(strItemName);
		if (pItem)
		{
			pItem->OnUpdate(CString(strStatus.GetAt(i)));
		}
	}
}

void YSocketDevice::HandleEvent(CString strIPParam, CString strEventCode, CString strPartitionNo, CString strPointNo)
{
	YOPCItem* pItem = NULL;
	int nEventCode = atoi(strEventCode);
	if (nEventCode == 1408 || nEventCode == 1456 || nEventCode == 1459 || nEventCode == 1697 || nEventCode == 1463 || nEventCode == 1441 || nEventCode == 1442 ||
		(3400 <= nEventCode && nEventCode <= 3404) || (3407 <= nEventCode && nEventCode <= 3409) || strEventCode == "340A" || strEventCode == "340B" || nEventCode == 3441 || nEventCode == 3442 ||
		nEventCode == 3450 || nEventCode == 3452 || nEventCode == 3451 || nEventCode == 3456 || nEventCode == 3450 || nEventCode == 3463 || nEventCode == 3697)//布防
	{
		HanleArmEvent(strIPParam, strPartitionNo, "0");
	}
	if ((1400 <= nEventCode && nEventCode <= 1404) || nEventCode == 1407 || nEventCode == 1409 || strEventCode == "140A" || strEventCode == "140B" || nEventCode == 1450 ||
		nEventCode == 1451 || nEventCode == 1452 || nEventCode == 1695 || nEventCode == 3695)//撤防
	{
		HanleArmEvent(strIPParam, strPartitionNo, "1");
	}

	OutPutLog(strIPParam + "===" + strEventCode + "===" + strPartitionNo + "===" + strPointNo);

	CString strItemName;
	strItemName.Format("%s-Part%02d-Point%03d", strIPParam, atoi(strPartitionNo), atoi(strPointNo));
	pItem = GetItemByName(strItemName);
	if (!pItem) {
		OutPutLog("Event pItem为NULL,Name: " + strItemName);
		return;
	}
	switch (atoi(strEventCode))
	{
	case 1120:
	case 1121:
	case 1133:
	case 1134:
	case 1137:
	case 1142:
	case 1144:
	case 1150:
	case 1130://窃盗
	{
		pItem->OnUpdate("1");
	}
	break;
	default:
		break;
	}

	if (nEventCode == 1465 || nEventCode == 3465 || (3100 <= nEventCode && nEventCode <= 3163) || nEventCode == 3641 || nEventCode == 3642 || nEventCode == 3654)
	{
		pItem->OnUpdate("0");
	}

}

void YSocketDevice::HanleArmEvent(CString strIPParam, CString strParttionNo, CString strArmed)
{
	CString strLog;
	OutPutLog("处理撤布防事件" + strIPParam + "-" + strParttionNo + "-" + strArmed);

	YOPCItem* pItem = NULL;
	int nPartNo = atoi(strParttionNo);
	CString strItemName;
	strItemName.Format("%s-PartitionStatus%d", strIPParam, nPartNo);
	pItem = GetItemByName(strItemName);
	if (pItem)
		pItem->OnUpdate(strArmed);
	if (pItem)
		strLog.Format("%s-%s", pItem->GetName(), strArmed);
	else
		strLog = "pItem为空";
	OutPutLog(strLog);
}