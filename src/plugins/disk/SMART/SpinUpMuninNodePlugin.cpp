#include "StdAfx.h"
#include "SpinUpMuninNodePlugin.h"

SpinUpMuninNodePlugin::SpinUpMuninNodePlugin()
{

}

SpinUpMuninNodePlugin::~SpinUpMuninNodePlugin()
{

}

int SpinUpMuninNodePlugin::GetConfig(char* buffer, int len)
{
    int ret = 0;
    int index = 0;

    ret = _snprintf(buffer, len, "graph_title Spin-Up Time\n"
        "graph_args --base 1000 -l 0\n"
        "graph_vlabel Time in milliseconds (ms)\n"
        "graph_category S.M.A.R.T\n"
        "graph_info This graph shows Spin up time of the hard drives in the machine.\n");
    buffer += ret;
    len -= ret;

    JCAutoLockCritSec lock(&g_SmartReaderCritSec);
    g_SmartReader.UpdateSMART();
    for (index = 0; index < g_SmartReader.m_ucDrivesWithInfo; index++) {
        ret = _snprintf(buffer, len, "_spinup_%i_.label %s \n", index,
            (PCHAR)g_SmartReader.m_stDrivesInfo[index].m_stInfo.sModelNumber);
        len -= ret;
        buffer += ret;
    }

    strncat(buffer, ".\n", len);

    return 0;
}

int SpinUpMuninNodePlugin::GetValues(char* buffer, int len)
{
    int index = 0;
    int ret;

    JCAutoLockCritSec lock(&g_SmartReaderCritSec);
    g_SmartReader.UpdateSMART();
    for (index = 0; index < g_SmartReader.m_ucDrivesWithInfo; index++) {
        ST_DRIVE_INFO* pDriveInfo = g_SmartReader.GetDriveInfo(index);
        if (!pDriveInfo)
            continue;

        ST_SMART_INFO* pSmartInfo = g_SmartReader.GetSMARTValue(pDriveInfo->m_ucDriveIndex, SMART_ATTRIB_SPIN_UP_TIME);
        if (!pSmartInfo)
            continue;

        ret = _snprintf(buffer, len, "_spinup_%i_.value %i\n", index, pSmartInfo->m_dwAttribValue);
        len -= ret;
        buffer += ret;
    }

    strncat(buffer, ".\n", len);
    return 0;
}
