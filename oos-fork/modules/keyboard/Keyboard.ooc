Registers: cover {
  gs, fs, es, ds: UInt // segments
  edi, esi, ebp, esp, ebx, edx, ecx, eax: UInt // pushed by pusha
  interruptNumber, errorCode: UInt
  eip, cs, eflags, useresp, ss: UInt // pushed by the processor automatically
}

Scancode: cover {
    ESC        := static 0x01
    ENTER      := static 0x1c
    LCTRL      := static 0x1d
    LSHIFT     := static 0x2a
    RSHIFT     := static 0x36
    LALT       := static 0x38
    CAPSLOCK   := static 0x3a
    NUMLOCK    := static 0x45
    SCROLLLOCK := static 0x46
}

portsOutByte: extern proto static func (port: UInt16, val: UInt8)
portsInByte: extern proto static func (port: UInt16) -> UInt8
irqHandlerInstall: extern proto static func (irq: Int, handler: Func (Registers*))

// own class must be loaded during initialization
_load: extern proto static func

// FIXME: rock compiler has a bug where Closures aren't treated properly when they are passed as arguments to externs
_irqHandlerInstall: static func (irq: Int, handler: Func (Registers*)) {
	irqHandlerInstall(irq, handler)
}

Keyboard: cover {
    lowercase: static Char[128] = [
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
        '9', '0', '-', '=', 0x08, /* Backspace */
        '\t', /* Tab */
        'q', 'w', 'e', 'r', /* 19 */
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
        0, /* 29 - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
        '\'', '`', 0, /* Left shift */
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
        'm', ',', '.', '/', 0, /* Right shift */
        '*',
        0, /* Alt */
        ' ', /* Space bar */
        0, /* Caps lock */
        0, /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0xff, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0xfe, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0 /* All other keys are undefined */
    ]
    
    uppercase: static Char[128] = [
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
        '(', ')', '_', '+', 0x08, /* Backspace */
        '\t', /* Tab */
        'Q', 'W', 'E', 'R', /* 19 */
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
        0, /* 29 - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
        '"', '~', 0, /* Left shift */
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', /* 49 */
        'M', '<', '>', '?', 0, /* Right shift */
        '*',
        0, /* Alt */
        ' ', /* Space bar */
        0, /* Caps lock */
        0, /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0 /* All other keys are undefined */
    ]

    ESCAPE_CODE := static const 0xE0

    // true if this key is being held down
    shift     := static false
    alt       := static false
    ctrl      := static false

    // true if this key is enabled
    capslock   := static false
    numlock    := static false
    scrolllock := static false

    // true if the previous scancode was an escape code
    escaped := static false

    // buffer to store the scancodes
    buffer: static UInt8[1024]
    bufferIndex := static 0

    flushBuffer: static func {
        while(portsInByte(0x64) bitSet?(0)) {
            portsInByte(0x60)
        }
    }
    
    updateLights: static func {
        status: UInt8 = 0

        if(scrolllock)
            status |= 1
        if(numlock)
            status |= 2
        if(capslock)
            status |= 4

        // Wait for the keyboard to process our previous input if the
        // input buffer is full.
        while(portsInByte(0x64) bitSet?(1)) {}
        portsOutByte(0x60, 0xED)
        while(portsInByte(0x64) bitSet?(1)) {}
        portsOutByte(0x60, status)
    }

    read: unmangled(keyboardRead) static func -> Char {
        // Wait for keyboard input. The keyboard interrupt handler
        // will increment bufferIndex.
        while(bufferIndex == 0){}

        bufferIndex -= 1
        scancode := buffer[bufferIndex]

        if((shift && !capslock) || (capslock && !shift))
            uppercase[scancode] as Char
        else
            lowercase[scancode] as Char
    }
    
    setup: unmangled(keyboardSetup) static func {
	_load()

        numlock = true
        
        updateLights()
        flushBuffer()
        
        // The keyboard interrupt handler.
        _irqHandlerInstall(1, |regs|
            scancode := portsInByte(0x60)

            if(scancode == ESCAPE_CODE) {
                escaped = true
            } else if(scancode bitSet?(7)) {
                // This scancode defines that a key was just released (key-up).
                match(scancode bitClear(7)) {
                    // Shift key release
                    case Scancode LSHIFT =>
                        shift = false

                    case Scancode RSHIFT =>
                        shift = false
                }
            } else {
                match scancode {
                    // Shift key press
                    case Scancode LSHIFT =>
                        shift = true

                    case Scancode RSHIFT =>
                        shift = true

                    case Scancode CAPSLOCK =>
                        capslock = !capslock
                        This updateLights()

                    case Scancode NUMLOCK =>
                        numlock = !numlock
                        This updateLights()

                    case Scancode SCROLLLOCK =>
                        scrolllock = !scrolllock
                        This updateLights()
                
                    // Any other scan code
                    case =>
                        buffer[bufferIndex] = scancode
                        bufferIndex += 1
                }
            }
        )
    }
}

