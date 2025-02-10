#ifndef __ASIOCOMMON_INCLUDED___
#define __ASIOCOMMON_INCLUDED___

//#include <Objbase.h>

#include "./asio/asiosys.h"
#include "./asio/asio.h"
#include "./asio/asiodrivers.h"

#include "../CaptureIP/CommonData.h"

#include	"Writer_MP3.h"

#include <iostream>
using namespace std;

//using namespace CommonData;
using namespace AudioDevice;

const unsigned int g_MaxNmbChannels = MAX_DEVICES;	// 8 

//////////////////////////////////////////////
//the structure includes information about any models ASIO driver
struct	ASIO_Driver_Info
{
	unsigned int order_nmb;
	CHAR	name_model[_MAX_PATH];	// name one of the model device getting ASIO driver
};

typedef list<ASIO_Driver_Info>						ListASIODriverInfo;
typedef list<ASIO_Driver_Info>::iterator	IterListASIODriverInfo;


class CAsioWriter
{
public:

	CAsioWriter(void);
	~CAsioWriter(void);

	//static DWORD /*WINAPI */Func_Asio_Thread(LPVOID lpParam);

	// create common thread for ASIO processing
	bool AsioThread_Launch(void);
	// release Asio thread using
	void AsioThread_Stop(void);

	static void bufferSwitch(long index, ASIOBool direct);
	//void (*bufferSwitch)(long index, ASIOBool direct);
	static void sampleRateDidChange(ASIOSampleRate rate);
	static long asioMessage(long selector, long value, void* message, double* opt);
	static ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long index, ASIOBool direct);

	bool IsAsioEnable(void) const { return enable_asio; }

	// get synchronize objects for output control(camera control)
	void SetBarrierCtrlCam(	void *pSynh_Barrier_StateCam_1_in,
													void *pSynh_Barrier_StateCam_2_in
												);

	// function for synchronization( setting and management of working barrier )
	void Sync_Function(void);

	// release all data ASIO
	void ReleaseAll(void);

	// initialization data 
	void Init(void);

	// set state needed
 	int SetState(	long state_in
 							);
public:
	/// synchronization data
	//double m_dTime_CmnTimerCAMs;	// count frames from common timer by CAMERAs Module
//	volatile LONGLONG m_lCntFrame_CmnTimerCAMs_init;
	volatile LONGLONG m_lCntFrame_CmnTimerCAMs;
//	volatile LONGLONG m_lCmnTimeWrt_CAMs;

//	volatile LONGLONG m_lCntFrame_AsioTimer_All;
//	volatile LONGLONG m_lCntFrame_AsioTimer_Wr;

	volatile LONG m_lLastState_CAMs;
	///

	volatile LONG m_flag_Enable_BarrierOut;	// the flag define enable classCSynh_Barrier on the outside

	int m_InBuffer_size;
	int *m_p_in_buffers[2][g_MaxNmbChannels];

	int m_sectors_number;
	int	m_sectors_counter;

	int		m_out_sector_size;
	int		m_out_buffer_size;
	int		*m_p_out_buffers[g_MaxNmbChannels];

	volatile LONGLONG		m_bytes_SzData_real;	// uncompressed data size [byte] // for statistic -> not needed
	//unsigned int		m_bytes_SzData_MP3;			// compressed data size [byte]

	unsigned int m_max_nmb_LinesIn_dev;		// maximum channels(lines) in by device using
	unsigned int m_max_nmb_LinesOut_dev;	// maximum channels(lines) out by device using
	// 
	//AsioDrivers* asioDrivers;

	// objects list(audio ports) recording
	ListWriterMP3	m_List_Writers_MP3;

	// synchronization object for the writers
	CSynh_CritSect	m_SynhCS;

	volatile LONG m_state_rec;		// state [T_DeviceState::STOP / T_DeviceState::WRITE]

	double	frames_lost_dbl;
	double tm_Wr_CAMs_ms; // time from CAMs need to write audio

	volatile LONG m_nmb_rec;	// order number of the record 
	char m_BasePathRecord[_MAX_PATH];

protected:

	// initialization ASIO driver
	int InitDriver(void);

	// assignment buffers for Asio
	int CreateBuffers(void);

	void ReleaseWriters(void);

	// read file initialization
	void ReadIniFile( void );

private:

	// number channels offset get from file INI
	int m_iOffsetChannels_Asio;

	/////////////////
	// list ASIO model device was gotten from ini file (see FILENAME_INI_DEVICE)
	ListASIODriverInfo	m_List_ASIODriverInfo_INI;

	// pointers for barrier state control(from camera (project CaptureIP))
	CSynh_Barrier *m_pSynh_Bar_StCam_1;		// sinh. object barrier the state camera
	CSynh_Barrier *m_pSynh_Bar_StCam_2;		// sinh. object barrier the state camera

	// flag ASIO enable
	bool enable_asio;

	// Thread parameters
// 	HANDLE		m_hAsioThread;
// 	DWORD			m_dwAsioThreadId;

//	volatile LONG m_FlagAsio_Thread;
};

#endif