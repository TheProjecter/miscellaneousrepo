Registers: cover {
  gs, fs, es, ds: UInt // segments
  edi, esi, ebp, esp, ebx, edx, ecx, eax: UInt // pushed by pusha
  interruptNumber, errorCode: UInt
  eip, cs, eflags, useresp, ss: UInt // pushed by the processor automatically
}

portsOutByte: extern proto static func (port: UInt16, val: UInt8)
portsInByte: extern proto static func (port: UInt16) -> UInt8
enableInterrupts: extern proto static func
disableInterrupts: extern proto static func

irqHandlerInstall: extern proto static func (irq: Int, handler: Func (Registers*))

_irqHandlerInstall: static func (irq: Int, handler: Func (Registers*)) {
	irqHandlerInstall(irq, handler)
}

// own class must be loaded during initialization
_load: extern proto static func

RTC: cover {
	bcd: static Bool
	ticks := static 0 as UInt64

	setup: unmangled(rtcSetup) static func {
		_load()
		// install RTC interrupt handler
		_irqHandlerInstall(8, |regs|
			// if periodic interrupt, increment ticks by 1
			portsOutByte(0x70, 0x0C)
			type := portsInByte(0x71) // read to keep timer active
			if (type & 0x40) {
				ticks += 1
			}
		)

		disableInterrupts() // any interrupts will not allow us to program the RTC
		// check whether format is binary or BCD (need to convert to decimal)
		portsOutByte(0x70, 0x0B)
		status := portsInByte(0x71)
		bcd = !(status & 0x04)

		// set rate to 3 (so frequency will be 8192 Hz = 8192 ticks per second)
		portsOutByte(0x70, 0x0A)
		prev := portsInByte(0x71) // important, register A must be saved
		// as it contains more than just the rate
		portsOutByte(0x71, (prev & 0xF0) | 3)

		status |= 0x02 // 24 hours clock
		status |= 0x40 // register for periodic interrupt
		portsOutByte(0x70, 0x0B)
		portsOutByte(0x71, status)

		portsOutByte(0x70, 0x0C)
		portsInByte(0x71)
		enableInterrupts()
	}

	dechex: static func(dec: UInt) -> UInt {
		if (!bcd) return dec

		num := 0
		pow := 1

		while (dec > 0) {
			num += (dec & 0xf) * pow
			pow *= 10
			dec = dec >> 4
		}	

		return num
	}

	hexdec: static func(dec: UInt) -> UInt {
		if (!bcd) return dec

		num := 0
		pow := 1

		while (dec > 0) {
			num += (dec % 10) * pow
			pow = pow << 4
			dec = dec / 10
		}	

		return num
	}

	getDate: unmangled(rtcGetDate) static func() -> UInt {
		disableInterrupts()
		portsOutByte(0x70, 0x07)
		day := portsInByte(0x71) // 5 bits
		portsOutByte(0x70, 0x08)
		mon := portsInByte(0x71) // 4 bits
		portsOutByte(0x70, 0x09)
		year := portsInByte(0x71) // rest
		enableInterrupts()

		return (((2000 + dechex(year)) << 9) | (dechex(mon) << 5) | dechex(day))
	}

	getTime: unmangled(rtcGetTime) static func() -> UInt {
		disableInterrupts()
		portsOutByte(0x70, 0x00)
		s := portsInByte(0x71) // 6 bits
		portsOutByte(0x70, 0x02)
		m := portsInByte(0x71) // 6 bits
		portsOutByte(0x70, 0x04)
		h := portsInByte(0x71) // 5 bits
		enableInterrupts()

		return ((dechex(h) << 12) | (dechex(m) << 6) | dechex(s))
	}

	setDate: unmangled(rtcSetDate) static func(d, m, y: UInt) {
		disableInterrupts()
		portsOutByte(0x70, 0x07)
		portsOutByte(0x71, hexdec(d))
		portsOutByte(0x70, 0x08)
		portsOutByte(0x71, hexdec(m))
		portsOutByte(0x70, 0x09)
		y = y < 2000 ? 0 : y - 2000
		portsOutByte(0x71, hexdec(y))
		enableInterrupts()
	}

	setTime: unmangled(rtcSetTime) static func(h, m, s: UInt) {
		disableInterrupts()
		portsOutByte(0x70, 0x00)
		portsOutByte(0x71, hexdec(s))
		portsOutByte(0x70, 0x02)
		portsOutByte(0x71, hexdec(m))
		portsOutByte(0x70, 0x04)
		portsOutByte(0x71, hexdec(h)) // NOTE: in qemu this works properly
		// in bochs, it seemed to stay 1 hour behind what I set it to
		enableInterrupts()
	}

	isLeap?: static func(year: UInt) -> Bool {
		return !(year & 3) && (year % 100 || !(year % 400))
	}

	getTicks: unmangled(rtcGetTicks) static func() -> UInt {
		return ticks
	}

	getUnixTime: unmangled(rtcUnixTime) static func() -> UInt {
		disableInterrupts()
		portsOutByte(0x70, 0x00)
		sec := portsInByte(0x71)
		portsOutByte(0x70, 0x02)
		min := portsInByte(0x71)
		portsOutByte(0x70, 0x04)
		hour := portsInByte(0x71)
		portsOutByte(0x70, 0x07)
		day := portsInByte(0x71)
		portsOutByte(0x70, 0x08)
		mon := portsInByte(0x71)
		portsOutByte(0x70, 0x09)
		year := portsInByte(0x71) as UInt
		enableInterrupts()

		sec = dechex(sec)
		min = dechex(min)
		hour = dechex(hour)
		day = dechex(day)
		mon = dechex(mon)
		year = dechex(year) + 2000

		// year will always be > 1970 (>=2000 even)
		// NOTE: this doesn't consider DST (then again, UTC doesn't have DST)
		days := -1
		leap := isLeap?(year)
		for (i in 1..mon + 1) {
			if (i == mon)
				days += day
			else {
				if (i == 1) {
					days += 31
				} else if (i == 2) {
					days += leap ? 29 : 28
				} else if (i == 3) {
					days += 31
				} else if (i == 4) {
					days += 30
				} else if (i == 5) {
					days += 31
				} else if (i == 6) {
					days += 30
				} else if (i == 7) {
					days += 31
				} else if (i == 8) {
					days += 31
				} else if (i == 9) {
					days += 30
				} else if (i == 10) {
					days += 31
				} else if (i == 11) {
					days += 30
				} else if (i == 12) {
					days += 31
				}
			}
		}
	
		for (y in 1970..year) {			
			days += isLeap?(y) ? 366 : 365
		}

		return days * 86400 + hour * 3600 + min * 60 + sec
	}
}

