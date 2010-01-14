// SmartReader.h: interface for the CSmartReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMARTREADER_H__494F15B9_0FFA_4BB4_BDD0_2D4C5129E530__INCLUDED_)
#define AFX_SMARTREADER_H__494F15B9_0FFA_4BB4_BDD0_2D4C5129E530__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../core/TString.h"
#include "../../core/JCThread.h"

typedef std::map<DWORD,LPVOID> SMARTINFOMAP;


#define SMART_ATTRIB_RAW_READ_ERROR_RATE					1
#define SMART_ATTRIB_THROUGHPUT_PERFORMANCE					2
#define SMART_ATTRIB_SPIN_UP_TIME							3
#define SMART_ATTRIB_START_STOP_COUNT						4
#define SMART_ATTRIB_START_REALLOCATION_SECTOR_COUNT		5
#define SMART_ATTRIB_SEEK_ERROR_RATE						7
#define SMART_ATTRIB_POWER_ON_HOURS_COUNT					9
#define SMART_ATTRIB_SPIN_RETRY_COUNT						10
#define SMART_ATTRIB_RECALIBRATION_RETRIES					11
#define SMART_ATTRIB_DEVICE_POWER_CYCLE_COUNT				12
#define SMART_ATTRIB_SOFT_READ_ERROR_RATE					13
#define SMART_ATTRIB_LOAD_UNLOAD_CYCLE_COUNT				193
#define SMART_ATTRIB_TEMPERATURE							194
#define SMART_ATTRIB_ECC_ON_THE_FLY_COUNT					195
#define SMART_ATTRIB_REALLOCATION_EVENT_COUNT				196
#define SMART_ATTRIB_CURRENT_PENDING_SECTOR_COUNT			197
#define SMART_ATTRIB_UNCORRECTABLE_SECTOR_COUNT				198
#define SMART_ATTRIB_ULTRA_DMA_CRC_ERROR_COUNT				199
#define SMART_ATTRIB_WRITE_ERROR_RATE						200
#define SMART_ATTRIB_TA_COUNTER_INCREASED					202
#define SMART_ATTRIB_GSENSE_ERROR_RATE						221
#define SMART_ATTRIB_POWER_OFF_RETRACT_COUNT				228
#define MAX_ATTRIBUTES	256

#define INDEX_ATTRIB_INDEX									0
#define INDEX_ATTRIB_UNKNOWN1								1
#define INDEX_ATTRIB_UNKNOWN2								2
#define INDEX_ATTRIB_VALUE									3
#define INDEX_ATTRIB_WORST									4
#define INDEX_ATTRIB_RAW									5
typedef struct 
{
	WORD wGenConfig;
	WORD wNumCyls;
	WORD wReserved;
	WORD wNumHeads;
	WORD wBytesPerTrack;
	WORD wBytesPerSector;
	WORD wSectorsPerTrack;
	WORD wVendorUnique[3];
	BYTE sSerialNumber[20];
	WORD wBufferType;
	WORD wBufferSize;
	WORD wECCSize;
	BYTE sFirmwareRev[8];
	BYTE sModelNumber[39];
	WORD wMoreVendorUnique;
	WORD wDoubleWordIO;
	WORD wCapabilities;
	WORD wReserved1;
	WORD wPIOTiming;
	WORD wDMATiming;
	WORD wBS;
	WORD wNumCurrentCyls;
	WORD wNumCurrentHeads;
	WORD wNumCurrentSectorsPerTrack;
	WORD ulCurrentSectorCapacity;
	WORD wMultSectorStuff;
	DWORD ulTotalAddressableSectors;
	WORD wSingleWordDMA;
	WORD wMultiWordDMA;
	BYTE bReserved[127];
}ST_IDSECTOR;

typedef struct
{
	BYTE  bDriverError;
	BYTE  bIDEStatus;
	BYTE  bReserved[2];
	DWORD dwReserved[2];
} ST_DRIVERSTAT;

typedef struct
{
	DWORD      cBufferSize;
	ST_DRIVERSTAT DriverStatus;
	BYTE       bBuffer[1];
} ST_ATAOUTPARAM;

typedef struct
{
	BYTE m_ucAttribIndex;
	DWORD m_dwAttribValue;
	BYTE m_ucValue;
	BYTE m_ucWorst;
	DWORD m_dwThreshold;
}ST_SMART_INFO;

typedef struct
{
	GETVERSIONINPARAMS m_stGVIP;
	ST_IDSECTOR m_stInfo;
	ST_SMART_INFO m_stSmartInfo[256];
	BYTE m_ucSmartValues;
	BYTE m_ucDriveIndex;
  char m_csErrorString[MAX_PATH];
}ST_DRIVE_INFO;

typedef struct
{
	BOOL m_bCritical;
	BYTE m_ucAttribId;
	char m_csAttribName[MAX_PATH];
	char m_csAttribDetails[MAX_PATH];
}ST_SMART_DETAILS;

typedef std::map<BYTE,ST_SMART_DETAILS> SMARTDETAILSMAP;

class CSmartReader  
{
public:
	CSmartReader();
	virtual ~CSmartReader();

	BOOL ReadSMARTValuesForAllDrives();
	ST_SMART_DETAILS *GetSMARTDetails(BYTE ucAttribIndex);
	ST_SMART_INFO *GetSMARTValue(BYTE ucDriveIndex,BYTE ucAttribIndex);
	ST_DRIVE_INFO *GetDriveInfo(BYTE ucDriveIndex);

	BYTE m_ucDrivesWithInfo; // Number of drives with information read
	BYTE m_ucDrives;// Fixed HDD's
	ST_DRIVE_INFO m_stDrivesInfo[32];


private:
	VOID InitAll();
	VOID CloseAll();
	VOID FillAttribGenericDetails();
	VOID ConvertString(PBYTE pString,DWORD cbData);

	BOOL ReadSMARTInfo(BYTE ucDriveIndex);
	BOOL IsSmartEnabled(HANDLE hDevice,UCHAR ucDriveIndex);
	BOOL CollectDriveInfo(HANDLE hDevice,UCHAR ucDriveIndex);
	BOOL ReadSMARTAttributes(HANDLE hDevice,UCHAR ucDriveIndex);

	ST_SMART_DETAILS m_stSmartDetails;
	SMARTINFOMAP m_oSmartInfo;
	SMARTDETAILSMAP m_oSMARTDetails;
};

class CSmartReader2 : public CSmartReader {
  ULARGE_INTEGER m_LastUpdateTime;
public:
  CSmartReader2();
  virtual ~CSmartReader2();

  // Only update the SMART info every minute
  void UpdateSMART();
};

extern CSmartReader2 g_SmartReader;
extern JCCritSec g_SmartReaderCritSec;

#endif // !defined(AFX_SMARTREADER_H__494F15B9_0FFA_4BB4_BDD0_2D4C5129E530__INCLUDED_)
