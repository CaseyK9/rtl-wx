
/*========================================================================
    
   rtl-wx.h
    
   This provides state machine structs for reading serial data coming from
   Oregon Scientific weather sensors.

   The extenal interface to this functionality can be found at the
   bottom of this file (the top bits are declarations for packet
   structures).

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF
   THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE AWARE OF THE POSSIBILITY OF SUCH DAMAGE.
       
========================================================================*/
#ifndef __RTL_WX_h
#define __RTL_WX_h
#include <time.h>
#include <pthread.h>

#define CONFIG_FILE_PATH   "rtl-wx.conf"
#define LOG_FILE_PATH      "rtl-wx.log"
#define DEFAULT_WORKING_DIR   "../www"

typedef enum _BOOL { FALSE = 0, TRUE } BOOL;

//-------------------------------------------------------------------------------------------------------------------------------
// Define main structure for storage of weather data
//-------------------------------------------------------------------------------------------------------------------------------

#define NUM_MINUTES_PER_SNAPSHOT             15 // This is the default.  It determines size of storage and some timeout checks

#define ENERGY_HISTORY_SAMPLES_PER_MINUTE     6
#define ENERGY_HISTORY_SAMPLES_PER_SNAPSHOT  (ENERGY_HISTORY_SAMPLES_PER_MINUTE * NUM_MINUTES_PER_SNAPSHOT)

#define OWL_ENERGY_HISTORY_SAMPLES_PER_MINUTE     6
#define OWL_ENERGY_HISTORY_SAMPLES_PER_SNAPSHOT  (OWL_ENERGY_HISTORY_SAMPLES_PER_MINUTE * NUM_MINUTES_PER_SNAPSHOT)

#define LARGEST_ENERGY_HISTORY_SAMPLES_PER_SNAPSHOT OWL_ENERGY_HISTORY_SAMPLES_PER_SNAPSHOT

#define WX_NUM_RECORDS_TO_STORE              96 // default is 1 day at 4 records per hour
#define WX_NUM_RAIN_RECORDS_TO_STORE        168 // default is 1 week at 1 per hour


// Timestamps - Timestamps are used in several places throughout the weather station monitoring program and 
// serve two distinct functions 1) They indicate whether a record has any data in it or is empty and 2) they
// indicate when the data for that record was gathered.
//
// Although it seems sufficient to timestamp all the fields received in one message from a sensor, 
// each data item is individually marked with a timestamp.
// THESE FIELD SPECIFIC TIMESTAMPS ARE REQUIRED BY THE MAX/MIN HISTORICAL DATA TRACKING CODE
//
typedef struct WX_timestamp
{
unsigned int   PktCnt;       // Value of global pkt count when timestamp was taken
time_t         timet;
} WX_Timestamp;


typedef struct WX_wind_gauge_data
{
WX_Timestamp Timestamp;
WX_Timestamp SpeedTimestamp;
WX_Timestamp AvgSpeedTimestamp;
BOOL     BatteryLow;
int      LockCode;
int      LockCodeMismatchCount;
int      noDataFor300Seconds;
int      noDataBetweenSnapshots;
int      Bearing;      //�
float    Speed;        //m/sec
float    AvgSpeed;     //m/sec
BOOL     ChillValid;   //not overrun and not invalid
int      WindChill;    //�C
} WX_WindGaugeData;

typedef struct WX_rain_gauge_gata
{
WX_Timestamp Timestamp;
WX_Timestamp RateTimestamp;
BOOL     BatteryLow;
int      LockCode;
int      LockCodeMismatchCount;
int      noDataFor300Seconds;
int      noDataBetweenSnapshots;
int      Rate;               //mm/hr
int      Total;              //mm
} WX_RainGaugeData;

typedef struct WX_outdoor_unit_data
{
WX_Timestamp Timestamp;
WX_Timestamp TempTimestamp;
WX_Timestamp RelHumTimestamp;
WX_Timestamp DewpointTimestamp;
BOOL     BatteryLow;
int      LockCode;
int      LockCodeMismatchCount;
int      noDataFor300Seconds; // No data received for >180 seconds
int      noDataBetweenSnapshots;
int      Channel;      //(don't think this is relevant for this unit)
float    Temp;         //�C
int      RelHum;       //%
float    Dewpoint;     //�C
} WX_OutdoorUnitData;

typedef struct WX_indoor_unit_data
{
WX_Timestamp Timestamp;
WX_Timestamp TempTimestamp;
WX_Timestamp RelHumTimestamp;
WX_Timestamp DewpointTimestamp;
WX_Timestamp PressureTimestamp;
BOOL     BatteryLow;
int      LockCode;
int      LockCodeMismatchCount;
int      noDataFor300Seconds;
int      noDataBetweenSnapshots;
float    Temp;           //�C
int      RelHum;         //%
float    Dewpoint;       //�C
int      Pressure;       //mbar
char     *ForecastStr;   //3 == rain, 6 == partly cloudy, 2 == cloudy, 12 == sunny
int      SeaLevelOffset; // mbar
} WX_IndoorUnitData;


// After each snapshot is taken, the  scheduler
// zeros  the energy sample array so it can detect missing samples when it does the next snapshot.
// Because the array is updated by the rtl433fm callback, a lock is needed
extern pthread_rwlock_t energy_sample_array_rw_lock;

typedef struct WX_energy_sensor_data
{
WX_Timestamp Timestamp;
int      LockCode;
int      LockCodeMismatchCount;
int      noDataFor300Seconds;
int      noDataBetweenSnapshots;
int      Watts;
int      WattsAvg; // Only updated when a snapshot is saved off by datastore.
int	 BurnerRuntimeSeconds; // Only updated when a snapshot is saved off by datastore.  USED IF   ENERGY SENSOR IS ATTACHED TO OIL OR GAS BURNER
int 	 WattsHistory[LARGEST_ENERGY_HISTORY_SAMPLES_PER_SNAPSHOT]; // Multithread issues - Get rw lock whenever writing this array
} WX_EnergySensorData;

typedef struct WX_extra_sensor_data
{
WX_Timestamp Timestamp;
WX_Timestamp TempTimestamp;
WX_Timestamp RelHumTimestamp;
WX_Timestamp DewpointTimestamp;
BOOL     BatteryLow;
int      LockCode;
int      LockCodeMismatchCount;
int      noDataFor300Seconds;
int      noDataBetweenSnapshots;
float    Temp;         //�C
int      RelHum;       //%
float    Dewpoint;     //�C
} WX_ExtraSensorData;

#define MAX_SENSOR_CHANNEL_INDEX 9
#define EXTRA_SENSOR_ARRAY_SIZE MAX_SENSOR_CHANNEL_INDEX+1

//Collection of latest data  received.
typedef struct WX_data
{
 WX_Timestamp currentTime;
 int BadPktCnt;
 int UnsupportedPktCnt;
 int noDataBetweenSnapshots;
 
 WX_WindGaugeData wg;
 WX_RainGaugeData rg;
 WX_OutdoorUnitData odu;
 WX_IndoorUnitData idu;
 WX_EnergySensorData energy;
 WX_EnergySensorData owl;
 WX_ExtraSensorData ext[EXTRA_SENSOR_ARRAY_SIZE]; // indexes 0..9 are used
} WX_Data;

//-------------------------------------------------------------------------------------------------------------------------------
//  Macro to control sending of status and debug messages to the remote client pipe and to the logfile
//-------------------------------------------------------------------------------------------------------------------------------

extern FILE *outputfd;  // Desc to send output to, could be stdout, or a pipe to client
extern FILE *logfd;     // Desc for program log file 

#define DPRINTF(...)  { \
char logPrintfStr[500]; time_t logprintftime = time(0); \
sprintf(logPrintfStr, __VA_ARGS__); \
if (logfd != NULL) { \
   char *logPrinTimeS = asctime(localtime(&logprintftime)); \
   if (strlen(logPrinTimeS) != 0) \
      logPrinTimeS[strlen(logPrinTimeS)-1] = 0; \
   fprintf(logfd,"%s %s",logPrinTimeS, logPrintfStr); \
   fflush(logfd); \
   } \
}

//-------------------------------------------------------------------------------------------------------------------------------
// Configuration (.conf) File Processor definitions
//-------------------------------------------------------------------------------------------------------------------------------

#define MAX_CONFIG_NAME_SIZE 500
#define MAX_CONFIG_LIST_SIZE 25

typedef struct _WX_CSVFile {
 char fname[MAX_CONFIG_NAME_SIZE];
 int  snapshotsBetweenUpdates;
} WX_CSVFile;
typedef struct _WX_TagFile {
 char inFile[MAX_CONFIG_NAME_SIZE];
 char outFile[MAX_CONFIG_NAME_SIZE];
} WX_TagFile;
typedef struct _WX_FtpFile {
 char filename[MAX_CONFIG_NAME_SIZE];
 char destpath[MAX_CONFIG_NAME_SIZE];
} WX_FtpFile;
typedef struct _WX_MailMessage {
 char recipients[MAX_CONFIG_NAME_SIZE];
 char subject[MAX_CONFIG_NAME_SIZE];
 char bodyFilename[MAX_CONFIG_NAME_SIZE];
} WX_MailMessage;

typedef struct _WX_ConfigSettings
{
 int sensorLockingEnabled;
 int altitudeInFeet;
 
 int fuelBurnerOnWattageThreshold;
 float fuelBurnerGallonsPerHour;

 int configFileReadFrequency;
 int dataSnapshotFrequency;
 int rainDataSnapshotFrequency;
 
 int realtimeCsvWriteFrequency;
 char realtimeCsvFile[MAX_CONFIG_NAME_SIZE];
 
 int numCsvFilesToUpdate;
 WX_CSVFile csvFiles[MAX_CONFIG_LIST_SIZE];
 
 int tagFileParseFrequency;
 int NumTagFilesToParse;
 WX_TagFile tagFiles[MAX_CONFIG_LIST_SIZE];

 int webcamSnapshotFrequency;

 int ftpUploadFrequency;      // Minutes
 char ftpServerHostname[MAX_CONFIG_NAME_SIZE];
 char ftpServerUsername[MAX_CONFIG_NAME_SIZE];
 char ftpServerPassword[MAX_CONFIG_NAME_SIZE];
 int  numFilesToFtp;
 WX_FtpFile ftpFiles[MAX_CONFIG_LIST_SIZE];
 
 int  mailSendFrequency;      // hours
 char mailServerHostname[MAX_CONFIG_NAME_SIZE];
 char mailServerUsername[MAX_CONFIG_NAME_SIZE];
 char mailServerPassword[MAX_CONFIG_NAME_SIZE];
 int numMailMsgsToSend; 
 WX_MailMessage mailMsgList[MAX_CONFIG_LIST_SIZE];

 char iduNameString[MAX_CONFIG_NAME_SIZE];
 char oduNameString[MAX_CONFIG_NAME_SIZE];
 char extNameStrings[EXTRA_SENSOR_ARRAY_SIZE][MAX_CONFIG_NAME_SIZE]; // sensor idx 0..9 used
 
} WX_ConfigSettings;

extern WX_ConfigSettings WxConfig;
extern int WX_processConfigSettingsFile(char *inFname, WX_ConfigSettings *cVarp);

//-------------------------------------------------------------------------------------------------------------------------------
// Util.c routines
//-------------------------------------------------------------------------------------------------------------------------------

extern void WX_DumpInfo(FILE *fd);
extern void WX_DumpMaxMinInfo(FILE *fd);
extern void WX_DumpSensorInfo(FILE *fd);
extern void WX_DumpEnergyHistoryInfo(FILE *fd, char *sensor_name, WX_EnergySensorData *energyp, int samples_per_minute);
extern void WX_DumpConfigInfo(FILE *fd);
extern BOOL isTimestampPresent(WX_Timestamp *ts);
extern int getWattsAvgAvg(int use_efergy_sensor, int numSnapshotsToAverage);
extern int getEnergyHistoryIndex(int minute, int second, int samples_per_minute);

//-------------------------------------------------------------------------------------------------------------------------------
// rtl-wx.c routines and data
//-------------------------------------------------------------------------------------------------------------------------------

//the global collection of latest weather station data
extern WX_Data wxData;
// Thread lock for changing data. 

extern time_t WX_programStartTime;
extern long int WX_totalBurnerRunSeconds;

//-------------------------------------------------------------------------------------------------------------------------------
// TagProc.c routines
//-------------------------------------------------------------------------------------------------------------------------------
extern void WX_ReplaceTagsInTextFile(char *inFname, char *outFname);

//-------------------------------------------------------------------------------------------------------------------------------
// DataStore.c definitions
//-------------------------------------------------------------------------------------------------------------------------------

extern void WX_InitHistoricalWeatherData(int numberOfRecordsToStore);
extern void WX_InitHistoricalRainData(int numberOfRainRecordsToStore);
extern void WX_InitHistoricalMaxMinData(void);
extern void WX_SaveWeatherDataRecord(WX_Data *weatherDatap, WX_ConfigSettings *cVarp, int minutesPerSnapshot);
extern void WX_SaveRainDataRecord(WX_Data *weatherDatap);
extern void WX_WriteSensorDataToCSVFile(char *csvFilename, WX_Data *weatherDatap, WX_ConfigSettings *cVarp, int numSamplesToInclude);
extern void WX_WriteRealTimeCSVFile();

extern WX_Data *WX_GetWeatherDataRecord(int howFarBackToGo);
extern unsigned int WX_GetRainDataRecord(int howFarBackToGo);
extern WX_Data *WX_GetMinDataRecord();
extern WX_Data *WX_GetMaxDataRecord();

//-------------------------------------------------------------------------------------------------------------------------------
// Scheduler  routines
//-------------------------------------------------------------------------------------------------------------------------------

// Init scheduler module that keep track of when to do ftp upload, file parsing, etc 
extern void WX_InitActionScheduler(WX_Data *weatherDatap, 
                                WX_ConfigSettings *configurationVarp);
extern void WX_DumpSchedulerInfo(FILE *fd);

// These scheduler actions are global because user can manually initiate them from outside of the Scheduler
extern void WX_DoScheduledActions(void);
extern void WX_DoConfigFileRead(void);
extern void WX_DoDataSnapshotSave(int minutesPerSnapshot);
extern void WX_DoRealTimeCsvFileWrite();
extern void WX_DoCsvFileUpdate(int index);
extern void WX_DoRainDataSnapshotSave(void);
extern void WX_DoWebcamSnapshot(void);
extern void WX_DoTagFileProcessing(void);
extern int  WX_DoFtpUpload(void);

#endif
