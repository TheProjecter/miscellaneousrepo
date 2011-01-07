import Modules

Exports: cover {
    // core module exports
    // Keyboard
    keyboardRead: static Func() -> Char
    // Display
    print: static Func(String)
    println: static Func(String)
    setColor: static Func(UInt16, UInt16)
    setColorFn: static Func(UInt16, UInt16, Func ())
    setFgColor: static Func(UInt16)
    setFgColorFn: static Func(UInt16, Func ())
    setBgColor: static Func(UInt16)
    setBgColorFn: static Func(UInt16, Func ())
    clearScreen: static Func()
    printChar: static Func (Char)
    printString: static Func(String)
    // RTC
    rtcGetDate: static Func() -> UInt
    rtcGetTime: static Func() -> UInt
    rtcSetDate: static Func(UInt, UInt, UInt) 
    rtcSetTime: static Func(UInt, UInt, UInt)
    rtcUnixTime: static Func() -> UInt
    rtcGetTicks: static Func() -> UInt

    lastExport := static null as String
    exportIdx := static 0 as UInt32

    addExport: static func (module: String, fn: Pointer) {
	if (module != lastExport) {
		lastExport = module
		exportIdx = 0
	} else {
		exportIdx += 1
	}		

	addr := Modules getExport(module, exportIdx)
	c := fn as Closure*
	c@ thunk = addr as Pointer
    }

    setup: static func {
	// configure exported functions
        // Keyboard
	addExport("Keyboard", keyboardRead& as Pointer)
	// Display
	addExport("Display", print& as Pointer)
	addExport("Display", println& as Pointer)
	addExport("Display", setColor& as Pointer)
	addExport("Display", setColorFn& as Pointer)
	addExport("Display", setFgColor& as Pointer)
	addExport("Display", setFgColorFn& as Pointer)
	addExport("Display", setBgColor& as Pointer)
	addExport("Display", setBgColorFn& as Pointer)
	addExport("Display", clearScreen& as Pointer)
	addExport("Display", printChar& as Pointer)
	addExport("Display", printString& as Pointer)
	// RTC
	addExport("RTC", rtcGetDate& as Pointer)
	addExport("RTC", rtcGetTime& as Pointer)
	addExport("RTC", rtcSetDate& as Pointer)
	addExport("RTC", rtcSetTime& as Pointer)
	addExport("RTC", rtcUnixTime& as Pointer)
	addExport("RTC", rtcGetTicks& as Pointer)
    }
}
