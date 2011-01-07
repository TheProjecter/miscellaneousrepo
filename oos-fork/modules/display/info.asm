bits 32

global _Module_Info
global _Module_Name
global _Module_Exports

extern displaySetup
extern print
extern println
extern setColor
extern setColorFn
extern setFgColor
extern setFgColorFn
extern setBgColor
extern setBgColorFn
extern clearScreen
extern printChar
extern printString

section .text

	_Module_Name:
		db 'Display',0

	_Module_Exports:
		dd print
		dd println
		dd setColor
		dd setColorFn
		dd setFgColor
		dd setFgColorFn
		dd setBgColor
		dd setBgColorFn
		dd clearScreen
		dd printChar
		dd printString

	_Module_Info:
		dd 0x12345678 ; Magic
		dd 0x00000001 ; Version
		dd _Module_Name ; Module name
		dd _Module_Exports ; Addresses of exported symbols
		dd displaySetup ; Init function pointer
