bits 32

global _Module_Info
global _Module_Name
global _Module_Exports

extern keyboardSetup
extern keyboardRead

section .text

	_Module_Name:
		db 'Keyboard',0

	_Module_Exports:
		dd keyboardRead

	_Module_Info:
		dd 0x12345678 ; Magic
		dd 0x00000001 ; Version
		dd _Module_Name ; Module name
		dd _Module_Exports ; Addresses of exported symbols
		dd keyboardSetup ; Init function pointer
