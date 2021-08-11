// MMtimerTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <Windows.h>

constexpr auto WAIT_MY_HANDLE_ERROR = -2;


class CMyEvent
{
	HANDLE m_hHandle;
public:
	CMyEvent()
	{
		m_hHandle = 0;
	}

	virtual ~CMyEvent()
	{
		::CloseHandle(m_hHandle);
	}

	virtual BOOL Create(WCHAR* name)
	{
		BOOL rV = FALSE;
		m_hHandle = ::CreateEventW(NULL, FALSE, FALSE, name);
		if (m_hHandle) rV = TRUE;
		return rV;
	}

	bool Post()
	{
		//----------------------------------
		//	Post
		//		This function is used to
		//	the object to the signaled state.
		//
		// return value
		//	TRUE if succesfull
		//	FALSE on fail
		//--------------------------------------

		if (m_hHandle) {
			if (::SetEvent(m_hHandle))
				return TRUE;
		}
		return FALSE;
	}
	int Pend(DWORD timeout = INFINITE)
	{
		//-----------------------------------
		// Pend
		//	This function will wait for an
		//	Event to be signaled.
		// Parameter:
		//	timeout---Time, in milliseconds
		//	that the method will wait for
		//	the object to be signalled.
		//	The default is Infinate
		//
		//	return value:
		//		WAIT_OBJECT_0....Success
		//		WAIT_TIMEOUT.....Timeout
		//		WAIT_FALED.......ERROR
		//		WAIT_MY_HANDLE_ERRPR...ERROR, bad handle
		//------------------------------------

		int rV = WAIT_MY_HANDLE_ERROR;	//object not created error
		if (m_hHandle)
			rV = ::WaitForSingleObject(m_hHandle, timeout);
		return rV;
	}
};



class CMMtimer
{
	UINT m_TimerID;
	UINT m_Resolution;
	static void CALLBACK TimerCallback(
		UINT TimerID,
		UINT message_DontUse,
		DWORD UserData,
		DWORD DontUse1,
		DWORD DontUse2
	);
public:
	CMMtimer() {
		m_TimerID = 0;
		m_Resolution = 0;
	};
	virtual ~CMMtimer() {
		timeKillEvent(m_TimerID);
		EndPeriod(m_Resolution);
	}
	MMRESULT BeginPeriod(DWORD timerRes) {
		return timeBeginPeriod(timerRes);
	}
	MMRESULT EndPeriod(DWORD timerRes) {
		return timeEndPeriod(timerRes);
	}
	virtual MMRESULT Create(
		UINT delay,
		UINT resolution,
		UINT TimerType = TIME_PERIODIC
	)
	{
		MMRESULT mmResult;

		mmResult = BeginPeriod(resolution);
		if (TIMERR_NOCANDO == mmResult)
			printf("Could not begin Period Bad Resolution(?) %d\n", resolution);
		else
		{
			mmResult = timeSetEvent(
				delay,
				resolution,
				CMMtimer::TimerCallback,
				DWORD_PTR(this),
				TimerType
			);
			if (0 < mmResult)
				m_TimerID = mmResult;
			else
				EndPeriod(resolution);
		}
		return mmResult;
	}
	DWORD GetTime() { return timeGetTime();	}
	MMRESULT GetSystemTime(MMTIME* SysTime, UINT cbmmt) {
		return timeGetSystemTime(SysTime, cbmmt);
	}
	MMRESULT GetDevCaps(TIMECAPS* timeCaps, UINT cbtc) {
		return timeGetDevCaps(timeCaps, cbtc);
	}
	virtual void DoCallback(
		UINT TimerID
	) {}
};


void CALLBACK CMMtimer::TimerCallback(
	UINT TimerID,
	UINT message_DontUse,
	DWORD UserData,
	DWORD DontUse1,
	DWORD DontUse2
)
{
	CMMtimer* pMMtmr = (CMMtimer*)UserData;
	pMMtmr->DoCallback(TimerID);
}

class CMyMMtimer :public CMMtimer
{
	CMyEvent m_Wait;
	UINT Count;
public:
	CMyMMtimer() {
		m_Wait.Create(NULL);
		Count = 0;
	}
	virtual ~CMyMMtimer() {}
	virtual void DoCallback(
		UINT TimerID
	);
	void Wait() {
		m_Wait.Pend();
	}
	void ResetCount() { Count = 10000; }
};

void CMyMMtimer::DoCallback(UINT id)
{
	//---------------------
	// Simple Callback
	//---------------------
	if (--Count == 0)
		m_Wait.Post();
}

int main()
{
	DWORD start, end;
	CMyMMtimer Timer;

	Timer.ResetCount();
	Timer.Create(1, 1);
	printf("Start\n");
	//--------------------------
	// Reset Timer Count to zero
	//--------------------------
	Timer.ResetCount();
	//--------------------------
	// Get Start time in mS
	//--------------------------
	start = Timer.GetTime();
	//--------------------------
	// Wait for count to get to
	// 10000 mS (10 seconds)
	//--------------------------
	Timer.Wait();
	//--------------------------
	// Calculate elapsed time
	// and display
	//--------------------------
	end = Timer.GetTime();
	printf("Stop %d\n",end - start);
}

