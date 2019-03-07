#pragma once
#include "basedevice.h"
#include "BaseItem.h"
#include "YOPCItem.h"
class YOPCDevice : public CBaseDevice
{
public:
	YOPCDevice(void);
	virtual ~YOPCDevice(void);
	virtual void OnUpdate()=0;
	virtual bool SetDeviceItemValue(CBaseItem* pAppItem)=0;
	YOPCItem* GetItemById(DWORD dwItemID)const;
	YOPCItem* GetItemByName(const CString strName)const;
	int GetItemsCount()const{ return (int)(y_ItemArray.GetCount());}
	const CMapStringToOb* GetItemArray()const{ return &y_ItemArray;}
	bool ShowItemBrowseDlg(CObArray* pDevItemArray);
	virtual void HandleData(char* pBuf,int nLen) = 0;
	virtual void BeginUpdateThread() = 0;
	virtual void EndUpdateThread() = 0;
protected:
	CMapStringToOb y_ItemArray;
};