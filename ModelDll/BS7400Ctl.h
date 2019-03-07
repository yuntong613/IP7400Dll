
#ifdef BS7400CTL_EXPORTS
#define BS7400CTL_API extern "C" __declspec(dllexport)
#else
#define BS7400CTL_API extern "C" __declspec(dllimport)
#endif

// This class is exported from the BS7400Ctl.dll
typedef void (CALLBACK* TRANDATAPROC)(LPVOID, LPSTR sRcvData, int iDataLen);

BS7400CTL_API void SetLnkIntval(void* pObject, int iInterval);
BS7400CTL_API void* New_Object();
BS7400CTL_API void Delete_Object(void* pObject);
BS7400CTL_API void ArrangeRcvAddress(void* pObject, LPSTR pRcvAddress, LPSTR pCtlAddress);
BS7400CTL_API int Execute(void* pObject, LPSTR sIPAdress, LPSTR sCommand, LPSTR sPara, int iPanelType = 0);
BS7400CTL_API void CloseReciever(void* pObject);
BS7400CTL_API DWORD OpenReceiver(void* pObject, TRANDATAPROC pTranFunc, LPVOID pPara);
BS7400CTL_API BOOL CanControlPanel(void* pObject);
BS7400CTL_API BOOL CanReceiveEvent(void* pObject);
BS7400CTL_API BOOL SetPanelControlCodes(void* pObject, LPSTR sPanelAddress, LPSTR sAgencyCode = NULL, LPSTR sPasscode = NULL);

