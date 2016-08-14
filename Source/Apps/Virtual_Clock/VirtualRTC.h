#ifndef __VirtualRTC_H
#define	__VirtualRTC_H

//RTCStructTypedef
//RTCUpdatedFlag is a flag which indicates if the RTC is updated,
//usually used in the case which needs to refresh the time on the 
//display terminal 
typedef struct {
	unsigned char Day;
	unsigned char Hour;
	unsigned char Min;
	unsigned char Sec;
  unsigned char SecDiv10;
	unsigned char RTCUpdatedFlag;
}RTCStructTypedef;

extern volatile RTCStructTypedef RTCCurrent;

extern volatile unsigned long SecondNum;

//see .c for details								
void SetVirtualRTC(RTCStructTypedef targetRTC);

//see .c for details								
void VirtualRTC_DeInit(void);

//see .c for details
void GenerateVirtualRTCString(char string[]);

//see .c for details
void VirtualRTC_Init(void);

//see .c for details
void VirtualRTC_Pause(void);

//see .c for details
void VirtualRTC_Resume(void);

#endif /* __VirtualRTC_H */
