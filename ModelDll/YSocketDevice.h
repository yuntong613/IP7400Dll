#pragma once
#include "YOPCDevice.h"
#include "Log.h"

#include "resource.h"

class YSocketDevice :
	public YOPCDevice
{
public:
	enum{ DEVICENAME = IDS_DEVICENAME};
	YSocketDevice(LPCSTR pszAppPath);
	virtual ~YSocketDevice(void);
	virtual bool SetDeviceItemValue(CBaseItem* pAppItem);
	virtual void OnUpdate();
	virtual void HandleData(char* pBuf,int nLen);
	virtual void Serialize(CArchive& ar);
	BOOL InitConfigFile(CString strFile);
	void Load(CArchive& ar);
	void LoadItems(CArchive& ar);
	int QueryOnce();
	virtual void BeginUpdateThread();
	virtual void EndUpdateThread();
	void GetMessageText(LPSTR strMsg, CStringArray& arrMsg);
	int GetMessgeInfoCount(LPSTR strMsg, int nDataLen);
public:
	void OutPutLog(CString strMsg);
	void HandlePointStatus(CString strIPParam, CString PointBlockNum, CString strStatus);
	void HandlePartitionStatus(CString strIPParam, CString strStatus);
	void HandleEvent(CString strIPParam, CString strEventCode, CString strPartitionNo, CString strPointNo);
	void HanleArmEvent(CString strIPParam, CString strParttionNo, CString strArmed);

protected:
	void* pObject;
	long y_lUpdateTimer;
	CString m_strLocalIP;
	CString m_strRecvPort;
	CString m_strCtrlPort;
	CStringArray m_arrMasterInfo;

	int m_nUseLog;
	CLog m_Log;
};
