// SmartReader.cpp: implementation of the CSmartReader class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SmartReader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define DRIVE_HEAD_REG	0xA0

CSmartReader::CSmartReader()
{
	InitAll();
	FillAttribGenericDetails();
}

CSmartReader::~CSmartReader()
{
	CloseAll();
}

VOID CSmartReader::InitAll()
{
	m_ucDrivesWithInfo=m_ucDrives=0;
	m_oSmartInfo.clear();
}

VOID CSmartReader::CloseAll()
{
	InitAll();
}

BOOL CSmartReader::ReadSMARTValuesForAllDrives()
{
  int iDriveIndex = 0;

  CloseAll();

  while (ReadSMARTInfo(iDriveIndex++))
    m_ucDrivesWithInfo++;

  m_ucDrives = m_ucDrivesWithInfo;

	if (m_ucDrives > 0)
		return TRUE;
	else
		return FALSE;
}

BOOL CSmartReader::ReadSMARTInfo(BYTE ucDriveIndex)
{
	HANDLE hDevice=NULL;
	TCHAR szT1[MAX_PATH]={0};
	BOOL bRet=FALSE;
	DWORD dwRet=0;
	
	_sntprintf(szT1, MAX_PATH, _T("\\\\.\\PHYSICALDRIVE%d"), ucDriveIndex);
	hDevice = CreateFile(szT1, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);
	if(hDevice!=INVALID_HANDLE_VALUE)
	{
		bRet=DeviceIoControl(hDevice,SMART_GET_VERSION,NULL,0,&m_stDrivesInfo[ucDriveIndex].m_stGVIP,sizeof(GETVERSIONINPARAMS),&dwRet,NULL);
		if(bRet)
		{			
			if((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_SMART_CMD)==CAP_SMART_CMD)
			{
				if(IsSmartEnabled(hDevice,ucDriveIndex))
				{
					bRet=CollectDriveInfo(hDevice,ucDriveIndex);
					bRet=ReadSMARTAttributes(hDevice,ucDriveIndex);
				}
			}
		}
		CloseHandle(hDevice);
	}
	return bRet;
}

BOOL CSmartReader::IsSmartEnabled(HANDLE hDevice,UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP={0};
	SENDCMDOUTPARAMS stCOP={0};
	DWORD dwRet=0;
	BOOL bRet=FALSE;

	stCIP.cBufferSize=0;
	stCIP.bDriveNumber =ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg=ENABLE_SMART;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;
	
	bRet=DeviceIoControl(hDevice,SMART_SEND_DRIVE_COMMAND,&stCIP,sizeof(stCIP),&stCOP,sizeof(stCOP),&dwRet,NULL);
	if(bRet)
	{

	}
	else
	{
		dwRet = GetLastError();
    _snprintf(m_stDrivesInfo[ucDriveIndex].m_csErrorString, sizeof(m_stDrivesInfo[ucDriveIndex].m_csErrorString)-1, "Error %d in reading SMART Enabled flag", dwRet);
	}
	return bRet;
}

BOOL CSmartReader::CollectDriveInfo(HANDLE hDevice,UCHAR ucDriveIndex)
{
	BOOL bRet=FALSE;
	SENDCMDINPARAMS stCIP={0};
	DWORD dwRet=0;
	#define OUT_BUFFER_SIZE IDENTIFY_BUFFER_SIZE+16
	char szOutput[OUT_BUFFER_SIZE]={0};

	stCIP.cBufferSize=IDENTIFY_BUFFER_SIZE;
	stCIP.bDriveNumber =ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg=0;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = 0;
	stCIP.irDriveRegs.bCylHighReg = 0;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = ID_CMD;

	bRet=DeviceIoControl(hDevice,SMART_RCV_DRIVE_DATA,&stCIP,sizeof(stCIP),szOutput,OUT_BUFFER_SIZE,&dwRet,NULL);
	if(bRet)
	{
		CopyMemory(&m_stDrivesInfo[ucDriveIndex].m_stInfo,szOutput+16,sizeof(ST_IDSECTOR));
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sModelNumber,39);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sSerialNumber,20);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sFirmwareRev,8);
	}
	else
		dwRet = GetLastError();
	return bRet;
}

VOID CSmartReader::ConvertString(PBYTE pString, DWORD cbData)
{
	char *szT1 = new char[cbData+2];
  int start, end;
  
	for (DWORD nC1 = 0; nC1 < cbData; nC1 += 2) {
		szT1[nC1] = pString[nC1+1];
		szT1[nC1+1] = pString[nC1];
	}

  memset(pString, 0, cbData);
  start = 1;
  end = 0;
  for (DWORD i = 0; i < cbData; i++) {
    char c = szT1[i];
    if (start && isspace(c)) {
      start = 0;
      continue;
    }
    if (isascii(c)) {
      *pString = c;
      pString++;
    }
  }
  
  pString--;
  for (int i = cbData-1; 0 < i; i--) {
    if (isspace(*pString)) {
      *pString = NULL;
      pString--;
    } else {
      break;
    }
  }
  
  delete [] szT1;
}

VOID CSmartReader::FillAttribGenericDetails()
{
#if 0
	TCHAR szINIFileName[MAX_PATH]={0},szKeyName[MAX_PATH]={0},szValue[1024]={0};
	int nC1,nSmartAttribs;
	ST_SMART_DETAILS stSD;

	m_oSMARTDetails.clear();
//	if(IsDebuggerPresent()==FALSE)
	{
		GetModuleFileName(NULL,szINIFileName,MAX_PATH);
		szINIFileName[lstrlen(szINIFileName)-3]=0;
		lstrcat(szINIFileName,_T("ini"));
	}
//	else
//		wsprintf(szINIFileName,"D:\\Saneesh\\Projects\\Helpers\\SMART\\Smart.ini");
	nSmartAttribs=GetPrivateProfileInt(_T("General"),_T("Max Attributes"),0,szINIFileName);
	for(nC1=0;nC1<nSmartAttribs;++nC1)
	{
		wsprintf(szKeyName,_T("Attrib%d"),nC1);
		stSD.m_ucAttribId=GetPrivateProfileInt(szKeyName,_T("Id"),0,szINIFileName);
		stSD.m_bCritical=GetPrivateProfileInt(szKeyName,_T("Critical"),0,szINIFileName);
		GetPrivateProfileString(szKeyName,_T("Name"),_T(""),szValue,1024,szINIFileName);
		strncpy(stSD.m_csAttribName, szValue, MAX_PATH);
		GetPrivateProfileString(szKeyName,T("Details"),_T(""),szValue,1024,szINIFileName);
		strncpy(stSD.m_csAttribDetails, szValue, MAX_PATH);
		m_oSMARTDetails.insert(SMARTDETAILSMAP::value_type(stSD.m_ucAttribId,stSD));
	}
#endif
}

ST_SMART_DETAILS *CSmartReader::GetSMARTDetails(BYTE ucAttribIndex)
{
	SMARTDETAILSMAP::iterator pIt;
	ST_SMART_DETAILS *pRet=NULL;

	pIt=m_oSMARTDetails.find(ucAttribIndex);
	if(pIt!=m_oSMARTDetails.end())
		pRet=&pIt->second;

	return pRet;
}

ST_SMART_INFO *CSmartReader::GetSMARTValue(BYTE ucDriveIndex,BYTE ucAttribIndex)
{
	SMARTINFOMAP::iterator pIt;
	ST_SMART_INFO *pRet=NULL;

	pIt=m_oSmartInfo.find(MAKELONG(ucAttribIndex,ucDriveIndex));
	if(pIt!=m_oSmartInfo.end())
		pRet=(ST_SMART_INFO *)pIt->second;
	return pRet;
}

BOOL CSmartReader::ReadSMARTAttributes(HANDLE hDevice,UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP={0};
	DWORD dwRet=0;
	BOOL bRet=FALSE;
	BYTE	szAttributes[sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
	UCHAR ucT1;
	PBYTE pT1,pT3;PDWORD pT2;
	ST_SMART_INFO *pSmartValues;

	stCIP.cBufferSize=READ_ATTRIBUTE_BUFFER_SIZE;
	stCIP.bDriveNumber =ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg=READ_ATTRIBUTES;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;
	bRet=DeviceIoControl(hDevice,SMART_RCV_DRIVE_DATA,&stCIP,sizeof(stCIP),szAttributes,sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,&dwRet,NULL);
	if(bRet)
	{
		m_stDrivesInfo[ucDriveIndex].m_ucSmartValues=0;
		m_stDrivesInfo[ucDriveIndex].m_ucDriveIndex=ucDriveIndex;
		pT1=(PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		for(ucT1=0;ucT1<30;++ucT1)
		{
			pT3=&pT1[2+ucT1*12];
			pT2=(PDWORD)&pT3[INDEX_ATTRIB_RAW];
			pT3[INDEX_ATTRIB_RAW+2]=pT3[INDEX_ATTRIB_RAW+3]=pT3[INDEX_ATTRIB_RAW+4]=pT3[INDEX_ATTRIB_RAW+5]=pT3[INDEX_ATTRIB_RAW+6]=0;
			if(pT3[INDEX_ATTRIB_INDEX]!=0)
			{
				pSmartValues=&m_stDrivesInfo[ucDriveIndex].m_stSmartInfo[m_stDrivesInfo[ucDriveIndex].m_ucSmartValues];
				pSmartValues->m_ucAttribIndex=pT3[INDEX_ATTRIB_INDEX];
				pSmartValues->m_ucValue=pT3[INDEX_ATTRIB_VALUE];
				pSmartValues->m_ucWorst=pT3[INDEX_ATTRIB_WORST];
				pSmartValues->m_dwAttribValue=pT2[0];
				pSmartValues->m_dwThreshold=MAXDWORD;
				m_oSmartInfo[MAKELONG(pSmartValues->m_ucAttribIndex,ucDriveIndex)]=pSmartValues;
				m_stDrivesInfo[ucDriveIndex].m_ucSmartValues++;
			}
		}
	}
	else
		dwRet=GetLastError();

	stCIP.irDriveRegs.bFeaturesReg=READ_THRESHOLDS;
	stCIP.cBufferSize=READ_THRESHOLD_BUFFER_SIZE; // Is same as attrib size
	bRet=DeviceIoControl(hDevice,SMART_RCV_DRIVE_DATA,&stCIP,sizeof(stCIP),szAttributes,sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,&dwRet,NULL);
	if(bRet)
	{
		pT1=(PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		for(ucT1=0;ucT1<30;++ucT1)
		{
			pT2=(PDWORD)&pT1[2+ucT1*12+5];
			pT3=&pT1[2+ucT1*12];
			pT3[INDEX_ATTRIB_RAW+2]=pT3[INDEX_ATTRIB_RAW+3]=pT3[INDEX_ATTRIB_RAW+4]=pT3[INDEX_ATTRIB_RAW+5]=pT3[INDEX_ATTRIB_RAW+6]=0;
			if(pT3[0]!=0)
			{
				pSmartValues=GetSMARTValue(ucDriveIndex,pT3[0]);
				if(pSmartValues)
					pSmartValues->m_dwThreshold=pT2[0];
			}
		}
	}
	return bRet;
}

ST_DRIVE_INFO *CSmartReader::GetDriveInfo(BYTE ucDriveIndex)
{
	return &m_stDrivesInfo[ucDriveIndex];
}
/*
		switch(pT3[0])
			{
				case SMART_ATTRIB_RAW_READ_ERROR_RATE:// Raw Read Error Rate
						TRACE("\n Raw Read Error Rate: %d",pT2[0]);
					break;

				case SMART_ATTRIB_SPIN_UP_TIME:// Spin up Time
						TRACE("\n Spin up Time:%d",pT2[0]);
					break;

				case SMART_ATTRIB_START_STOP_COUNT:// Start stop count
						TRACE("\n Start stop count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_START_REALLOCATION_SECTOR_COUNT:// Reallocation Sector Count
						TRACE("\n Reallocation Sector Count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_SEEK_ERROR_RATE:// Seek Error Rate
						TRACE("\n Seek Error Rate:%d",pT2[0]);
					break;

				case SMART_ATTRIB_POWER_ON_HOURS_COUNT:// Power On hours count
						TRACE("\n Power On hours count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_SPIN_RETRY_COUNT:// Spin retry count
						TRACE("\n Spin retry count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_DEVICE_POWER_CYCLE_COUNT:// Device Power cycle count
						TRACE("\n Device Power cycle count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_POWER_OFF_RETRACT_COUNT:// Power-off Retract Count
						TRACE("\n Power-off Retract Count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_LOAD_UNLOAD_CYCLE_COUNT:// Load/Unload cycle Count
						TRACE("\n Load/Unload cycle Count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_TEMPERATURE: // Temperature
						TRACE("\n Temperature: %d C",pT2[0]);
					break;

				case SMART_ATTRIB_ECC_ON_THE_FLY_COUNT:// ECC on the fly count
						TRACE("\n ECC on the fly count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_REALLOCATION_EVENT_COUNT:// Reallocation Event Count
						TRACE("\n Reallocation Event Count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_CURRENT_PENDING_SECTOR_COUNT:// Current Pending Sector count
						TRACE("\n Current Pending Sector count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_UNCORRECTABLE_SECTOR_COUNT:// Uncorrectable sector count
						TRACE("\n Uncorrectable sector count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_ULTRA_DMA_CRC_ERROR_COUNT:// Ultra DMA CRC Error count
						TRACE("\n Ultra DMA CRC Error count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_WRITE_ERROR_RATE://Write Error rate
						TRACE("\n Write Error rate: %d",pT2[0]);
					break;

				case SMART_ATTRIB_TA_COUNTER_INCREASED:// TA Counter increased
						TRACE("\n TA Counter increased: %d",pT2[0]);
					break;

				default:
						if(pT3[0]!=0)
						{
							TRACE("\n ID %d Value %d",pT3[0],pT2[0]);
						}
						else
							m_stDrivesInfo[ucDriveIndex].m_ucSmartValues--;
					break;
			}

*/

CSmartReader2::CSmartReader2()
{
  m_LastUpdateTime.QuadPart = 0;
}

CSmartReader2::~CSmartReader2()
{

}

// Only update the SMART info every minute
void CSmartReader2::UpdateSMART() 
{
  ULARGE_INTEGER curTime;
  GetSystemTimeAsFileTime((FILETIME *)&curTime);
  ULONGLONG diffTime = (curTime.QuadPart - m_LastUpdateTime.QuadPart);
  if (diffTime > (10000000*60)) {
    ReadSMARTValuesForAllDrives();
    m_LastUpdateTime.QuadPart = curTime.QuadPart;
  }
}
CSmartReader2 g_SmartReader;
JCCritSec g_SmartReaderCritSec;
