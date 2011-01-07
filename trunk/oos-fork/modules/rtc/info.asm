bits 32

global _Module_Info
global _Module_Name
global _Module_Exports

extern rtcSetup
extern rtcGetDate
extern rtcGetTime
extern rtcSetDate
extern rtcSetTime
extern rtcUnixTime
extern rtcGetTicks

section .text

	_Module_Name:
		db 'RTC',0

	_Module_Exports:
		dd rtcGetDate
		dd rtcGetTime
		dd rtcSetDate
		dd rtcSetTime
		dd rtcUnixTime
		dd rtcGetTicks

	_Module_Info:
		dd 0x12345678 ; Magic
		dd 0x00000001 ; Version
		dd _Module_Name ; Module name
		dd _Module_Exports ; Addresses of exported symbols
		dd rtcSetup ; Init function pointer
