portsOutByte: extern proto static func (port: UInt16, val: UInt8)
portsInByte: extern proto static func (port: UInt16) -> UInt8

// own class must be loaded during initialization
_load: extern proto static func

print: unmangled(print) func (str: String) {
    Display printString(str)
}

println: unmangled(println) func (str: String) {
    Display printString(str)
    Display printChar('\n')
}

Color: enum {
    black = 0
    blue
    green
    cyan
    red
    magenta
    brown
    lightGrey
    darkGrey
    lightBlue
    lightGreen
    lightCyan
    lightRed
    lightMagenta
    yellow
    white
}

Display: cover {
    VIDEO_MEMORY := static 0xb8000 as UInt16*

    INDEX_PORT := static 0x3d4
    DATA_PORT  := static 0x3d5

    CURSOR_LOW_PORT  := static 0xE
    CURSOR_HIGH_PORT := static 0xF

    CONSOLE_WIDTH  := static 80
    CONSOLE_HEIGHT := static 25

    color: static UInt16
    fg: static UInt16
    bg: static UInt16
    cursor_x: static Int
    cursor_y: static Int
    last_command: static Int

    setup: unmangled(displaySetup) static func {
	_load()

        // Default to light grey on black like the BIOS.
        setColor(Color lightGrey as UInt16, Color black as UInt16)
        clearScreen()
    }

    setColor: unmangled(setColor) static func (fg, bg: UInt16) {
        This fg = fg
        This bg = bg
        This color = (fg & 0xf) | bg << 4
    }

    setColor: unmangled(setColorFn) static func ~withFn (fg, bg: UInt16, fn: Func) {
        oldFg := This fg
        oldBg := This bg
        setColor(fg, bg)
        fn()
        setColor(oldFg, oldBg)
    }

    setFgColor: unmangled(setFgColor) static func (fg: UInt16) {
        setColor(fg, This bg)
    }

    setFgColor: unmangled(setFgColorFn) static func ~withFn (fg: UInt16, fn: Func) {
        oldFg := This fg
        setFgColor(fg)
        fn()
        setFgColor(oldFg)
    }

    setBgColor: unmangled(setBgColor) static func (bg: UInt) {
        setColor(This fg, bg)
    }

    setBgColor: unmangled(setBgColorFn) static func ~withFn (bg: UInt, fn: Func) {
        oldBg := This bg
        setBgColor(bg)
        fn()
        setBgColor(oldBg)
    }

    clearScreen: unmangled(clearScreen) static func {
        for(i in 0..(CONSOLE_HEIGHT * CONSOLE_WIDTH)) {
            VIDEO_MEMORY[i] = ' ' | color << 8
        }
        cursor_x = 0
        cursor_y = 0
        last_command = 0
        updateCursor()
    }

    updateCursor: static func {
        position := cursor_y * CONSOLE_WIDTH + cursor_x

        portsOutByte(INDEX_PORT, CURSOR_LOW_PORT)
        portsOutByte(DATA_PORT, position >> 8)

        portsOutByte(INDEX_PORT, CURSOR_HIGH_PORT)
        portsOutByte(DATA_PORT, position)
    }

    printChar: unmangled(printChar) static func (chr: Char) {
        // Handle a backspace, by moving the cursor back one space
        if(chr == '\b') {	 
	    i := cursor_y * CONSOLE_WIDTH + cursor_x
	    if (cursor_x > 0 || last_command != cursor_y) {
	        if (i > 0 && (VIDEO_MEMORY[i - 1] & 0xff) == '\t') // if previous character is a tab, go back 4 spaces		
                    cursor_x -= 4
		else
                    cursor_x -= 1
            }

            if (cursor_x < 0) {
                cursor_y -= 1
                cursor_x = CONSOLE_WIDTH + cursor_x
            }

            updateCursor()
            return
        }

        // Handles a 'Carriage Return', which simply brings the
        // cursor back to the margin
        else if(chr == '\r') {
            cursor_x = 0
        }

        // We handle our newlines the way DOS and the BIOS do: we
        // treat it as if a 'CR' was also there, so we bring the
        // cursor to the margin and we increment the 'y' value
        else if(chr == '\n') {
            cursor_x = 0
            cursor_y += 1
            last_command = cursor_y
        } else {
            // Handles a tab by incrementing the cursor's x by 4
            if(chr == '\t') {
		i := cursor_y * CONSOLE_WIDTH + cursor_x
		VIDEO_MEMORY[i + 3] = '\t'
                cursor_x += 4
            }

            // Any character greater than and including a space, is a
            // printable character. The equation for finding the index
            // in a linear chunk of memory can be represented by:
            // Index = [(y * width) + x]
            else if(chr >= ' ') {
                i := cursor_y * CONSOLE_WIDTH + cursor_x
                VIDEO_MEMORY[i] = chr | color << 8
                cursor_x += 1
            }

            // If the cursor has reached the edge of the screen's width, we
            // insert a new line in there
            if(cursor_x >= CONSOLE_WIDTH) {
                cursor_x = cursor_x - CONSOLE_WIDTH
                cursor_y += 1
            }

        }

        // If the cursor has gone below the bottom of the screen, we
        // scroll the screen
        if(cursor_y >= CONSOLE_HEIGHT) {
            cursor_y -= 1
            scroll()
        }


        updateCursor()
    }

    scroll: static func {
	// scroll down all previous rows
        for(row in 1..CONSOLE_HEIGHT) {
            for(col in 0..CONSOLE_WIDTH) {
                VIDEO_MEMORY[(row - 1) * CONSOLE_WIDTH + col] = VIDEO_MEMORY[row * CONSOLE_WIDTH + col]
            }
        }

	// insert a blank line at the end
        for(col in 0..CONSOLE_WIDTH) {
            VIDEO_MEMORY[(CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH + col] = ' ' | color << 8
        }
    }

    printString: unmangled(printString) static func (str: String) {
        for(i in 0..str length()) {
            printChar(str[i])
        }
    }
}
