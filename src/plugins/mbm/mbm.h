/////////////////////////////////////////////////////////////////////////////
//  File     : mbm.hpp
//  Date     : Octobre 13, 2003
//  Author   : Danny Couture
//      Mail : dcouture@popupsolutions.com
//  Version  : 1.0
//  Desc     : C++ wrapper for MotherBoard Monitor Shared Memory Access
//
/////////////////////////////////////////////////////////////////////////////
//
//  Date     : December 12, 2003
//  Author   : Maurizio Bonesi
//      Mail : maurizio.bonesi@it.datalogic.com
//  Version  : 1.1
//  Desc     : Added several methods for direct sensor access through mbm object.
//             Introduced several #defines for data structures length for an easier
//             class expansion or customization
/////////////////////////////////////////////////////////////////////////////
//
//  Date     : January 12, 2004
//  Author   : Robert Roessler
//      Mail : support@rftp.com
//  Version  : 1.2
//  Desc     : Replaced STL-style "strings" with plain C char arrays

#ifndef MBM_HPP
#define MBM_HPP

namespace mbm {

#define NOTFOUND_STRING         "not_found"

#define BT_ISA          "type_isa"
#define BT_SMBUS        "type_smbus"
#define BT_VIA686ABUS   "type_via686abus"
#define BT_DIRECTIO     "type_directio"
#define BT_UNKNOWN      "type_unknown"
typedef enum { btISA = 0, btSMBus, btVIA686ABus, btDirectIO } bus_t;

#define SMBT_INTEL  "type_intel"
#define SMBT_AMD    "type_amd"
#define SMBT_ALI    "type_ali"
#define SMBT_NFORCE "type_nforce"
#define SMBT_SIS    "type_sis"
#define SMBT_UNK    BT_UNKNOWN
typedef enum { smtSMBIntel = 0, smtSMBAMD, smtSMBALi, smtSMBNForce, smtSMBSIS } smb_t;

#define ST_UNKNOWN      BT_UNKNOWN
#define ST_TEMPERATURE  "type_temperature"
#define ST_VOLTAGE      "type_voltage"
#define ST_FAN          "type_fan"
#define ST_MHZ          "type_mhz"
#define ST_PERCENTAGE   "type_percentage"
typedef enum { stUnknown = 0, stTemperature, stVoltage, stFan, stMhz, stPercentage } sensor_t;


/* centralize #defines here for portability and upgradability */
#define SENSOR_INDEX_LENGTH 10
#define SENSOR_INFO_LENGTH  100

#define SENSOR_NAME_LEN     12
#define TIME_STRING_LENGTH  41
#define MBM_PATHNAME_LENGTH 256

#define SMBUSNAME_STRING_LENGTH 41

#define PADDING1_SIZE_BYTES 3   
#define PADDING2_SIZE_BYTES 4
#define PADDING3_SIZE_BYTES 8

#define VERSION_STRING_LENGTH       4
#define VERSION_STRING_SEPARATOR_CH '.'

template <typename T>
void flush(const T & val)
{
    FlushViewOfFile(&val, sizeof(val));
}

#pragma pack(push, 1)

class index
{
private:
    int           _type;                   // type of sensor
    int           _count;                  // number of sensor for that type

public:
    sensor_t type() const { return (sensor_t)_type; }
    int count()     const { return _count; }
};

class sensor
{
private:
    char           _type;                           // type of sensor
    char           _name[SENSOR_NAME_LEN];          // name of sensor
    char           _padding1[PADDING1_SIZE_BYTES];  // padding of 3 byte
    double         _current;                        // current value
    double         _low;                            // lowest readout
    double         _high;                           // highest readout
    long           _readout;                        // total number of readout
    char           _padding2[PADDING2_SIZE_BYTES];  // padding of 4 byte
    long double    _total;                          // total amout of all readouts
    char           _padding3[PADDING3_SIZE_BYTES];  // padding of 6 byte
    double         _alarm1;                         // temp & fan: low alarm; voltage: % off;
    double         _alarm2;                         // temp: high alarm

public:
    sensor_t type() const { return (sensor_t)_type; }
    void type(sensor_t type) { _type = (char)type; flush(_type); }

    LPCSTR name() const { return _name; }
    void name(LPCSTR name) { strncpy(_name, name, sizeof(_name)); flush(_name); }

    double current() const { return _current; }
    void current(double current) { _current = current; flush(_current); }

    double low() const { return _low; }
    void low(double low) { _low = low; flush(_low); }

    double high() const { return _high; }
    void high(double high) { _high = high; flush(_high); }

    long readout() const { return _readout; }
    void readout(long readout) { _readout = readout; flush(_readout); }

    double alarm1() const { return _alarm1; }
    void alarm1(double alarm1) { _alarm1 = alarm1; flush(_alarm1); }

    double alarm2() const { return _alarm2; }
    void alarm2(double alarm2) { _alarm2 = alarm2; flush(_alarm2); }
};

class info
{
private:
    short          _smbBase;                            // SMBus base address
    bus_t          _smbType;                            // SMBus/Isa bus used to access chip
    smb_t          _smbCode;                            // SMBus sub type, Intel, AMD or ALi
    char           _smbAddr;                            // Address of sensor chip on SMBus
    char           _smbName[SMBUSNAME_STRING_LENGTH];   // Nice name for SMBus
    short          _isaBase;                            // ISA base address of sensor chip on ISA
    int            _chipType;                           // Chip nr, connects with Chipinfo.ini
    char           _voltageSubType;                     // Subvoltage option selected

public:

    short          smbBase()        const { return _smbBase; }
    bus_t          smbType()        const { return _smbType; }
    smb_t          smbCode()        const { return _smbCode; }

    char           smbAddr()        const { return _smbAddr; }
    LPCSTR        smbName()        const { return _smbName; }
    short          isaBase()        const { return _isaBase; }
    int            chipType()       const { return _chipType; }
    char           voltageSubType() const { return _voltageSubType; }
};

#pragma pack(pop)

class mbm
{
private:
    #pragma pack(push, 1)
        struct data_t
        {
            double         _version;                    // version number (example: 51090)
            index          _index[SENSOR_INDEX_LENGTH]; // Sensor index
            sensor         _sensor[SENSOR_INFO_LENGTH]; // sensor info
            info           _info;                       // misc. info
            unsigned char  _start[TIME_STRING_LENGTH];  // start time
            unsigned char  _current[TIME_STRING_LENGTH];// current time
            unsigned char  _path[MBM_PATHNAME_LENGTH];  // MBM path
        };
    #pragma pack(pop)

    data_t *    data;
    HANDLE      mbmMemory;

    int         totSensors;
    char        v[16];

public:
    mbm()
        : data(0), mbmMemory(0), totSensors(0)
    {
		v[0] = '\0';
    }

    ~mbm()
    {
        close();
    }

    bool open()
    {
        char t[16];

        if (opened()) 
            return true;

        // try to open shared memory
        if (mbmMemory == 0)
            mbmMemory = OpenFileMappingA(FILE_MAP_WRITE, FALSE, "$M$B$M$5$S$D$");

        if (mbmMemory == 0)
            return false;

        data = (data_t *)MapViewOfFile(mbmMemory, FILE_MAP_WRITE, 0, 0, 0);
        for(int j=0; j<SENSOR_INDEX_LENGTH; j++) 
                    totSensors += (data->_index[j]).count();            

        //format version string
		_snprintf(t, 16, "%u", (unsigned)data->_version);
		_snprintf(v, 16, "%c%c%c%c%c%c%s",
			t[0], VERSION_STRING_SEPARATOR_CH,
			t[1], VERSION_STRING_SEPARATOR_CH,
			t[2], VERSION_STRING_SEPARATOR_CH,
			&t[3]);

        return opened();
    }

    bool opened() const { return data != 0; }

    void close()
    {
        if (data != 0)
        {
            UnmapViewOfFile(data);
            data = 0;
        }

        if (mbmMemory != 0)
        {
            CloseHandle(mbmMemory);
            mbmMemory = 0;
        }
    }

    double      version() const         { return data->_version; }
    LPCSTR     version_string() const  { return v; }
    LPCSTR     path()    const         { return (char*)data->_path; }

    index  *    index()         { return data->_index; }
    sensor *    sensor()        { return data->_sensor; }    
    const int   tot_sensors()   { return totSensors; }

    /* 
        Methods for accessing MBM properties and 
        retrieve a LPCTSTR out of them
    */
    LPCSTR bus_type(void) const { 
        switch((data->_info).smbType()) {
        case btISA:
            return BT_ISA;
            break;
        case btSMBus:
            return BT_SMBUS;
            break;
        case btVIA686ABus:
            return BT_VIA686ABUS;
            break;
        case btDirectIO:
            return BT_DIRECTIO;
            break;
        default:
            return BT_UNKNOWN;
            break;
        }
    }
    LPCSTR smb_name(void) const { return (data->_info).smbName(); }    
    LPCSTR smb_type(void) const { 
        switch((data->_info).smbCode()) {
        case smtSMBIntel:
            return SMBT_INTEL;
            break;
        case smtSMBAMD:
            return SMBT_AMD;
            break;
        case smtSMBALi:
            return SMBT_ALI;
            break;
        case smtSMBNForce:
            return SMBT_NFORCE;
            break;
        case smtSMBSIS:
            return SMBT_SIS;
            break;
        default:
            return SMBT_UNK;
            break;
        }
    }
    /*
        methods for retrieving a sensor property 
        directly from the mbm object instance just 
        by passing the sensor's string name
    */
    sensor_t type(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).type();
        }
        return stUnknown; 
    }
    LPCSTR sensor_type(LPCSTR n) const { 
        //retrieves a string with the sensor type 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                switch((data->_sensor[j]).type()) {
                case stTemperature:
                    return ST_TEMPERATURE;
                    break;
                case stVoltage:
                    return ST_VOLTAGE;
                    break;
                case stFan:
                    return ST_FAN;                    
                    break;
                case stMhz:
                    return ST_MHZ;                    
                    break;
                case stPercentage:
                    return ST_PERCENTAGE;                    
                    break;
                case stUnknown:
                default:
                    break;
                }
        }
        return ST_UNKNOWN;
    }
    double current(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).current();
        }
        return 0; 
    }
    double high(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).high();
        }
        return 0; 
    }
    double low(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).low();
        }
        return 0; 
    }
    long readout(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).readout();
        }
        return 0; 
    }
    double alarm1(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).alarm1();
        }
        return 0; 
    }
    double alarm2(LPCSTR n) const { 
        for(int j=0; j<totSensors; j++) {
            if( !strcmp((data->_sensor[j]).name(), n))
                return (data->_sensor[j]).alarm2();
        }
        return 0; 
    }
};

}   //namespace mbm

#endif  //MBM_HPP
