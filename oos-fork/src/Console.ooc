import Bochs, memory/MM, Exports

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

Console: cover {
    variables: static Pointer*
    index := static 0 as Int
    MAX_HISTORY_SIZE := static 5 as UInt
    history: static String*
    historyIdx := static 0 as Int
    historyCount := static 0 as Int

    run: static func {
	variables = gc_calloc(100, UInt size) as Pointer*
	history = gc_calloc(MAX_HISTORY_SIZE, UInt size) as String*
	for (i in 0..MAX_HISTORY_SIZE)
		history[i] = null
	
        buffer: Char[255]
        bufferIndex := 0
        ">> " print()

        while(true) {
            chr := Exports keyboardRead()
	    if (chr == 0xff) { // up key pressed
		// go back in history
		if (historyIdx > 0) {
			for (i in 0..bufferIndex) {
				"\b \b" print()
			}
			historyIdx -= 1

			history[historyIdx] print()
			len := history[historyIdx] length()
			for (i in 0..len) {
				buffer[i] = history[historyIdx][i]
			}
			bufferIndex = len
		}
	    } else if (chr == 0xfe) { // down key pressed
		// go forward in history
		if (historyIdx < historyCount) {
			for (i in 0..bufferIndex) {
				"\b \b" print()
			}
			historyIdx += 1

			if (historyIdx < historyCount) {
				history[historyIdx] print()
				len := history[historyIdx] length()
				for (i in 0..len) {
					buffer[i] = history[historyIdx][i]
				}
				bufferIndex = len
			} else bufferIndex = 0
		}
            } else if(chr == '\n') {
                '\n' print()
                cmd := String new(255)
		j := 0
                for(i in 0..bufferIndex) {		                   
			if (buffer[i] printable?()) {
				cmd[j] = buffer[i]
				j += 1
			}
		}
                cmd[j] = '\0'
		ind := cmd indexOf(' ')
                handleCommand(ind == -1 ? cmd : cmd substr(0, ind), ind != -1 ? cmd substr(ind + 1) : null)
                ">> " print()

		if (historyCount < MAX_HISTORY_SIZE) {
			historyIdx = historyCount
			if (history[historyIdx] == null) {
				history[historyIdx] = String new(255)
			}

			cmd copyTo(history[historyIdx])

			historyIdx += 1
			historyCount += 1
		} else {
			historyIdx = historyCount
			// shift everything one step back, add new
			for (i in 0..historyCount - 1) {
				history[i + 1] copyTo(history[i])
			}
			cmd copyTo(history[historyCount - 1])
		}
		free(cmd as Pointer)
                bufferIndex = 0
            } else if(chr == '\b') {
                if(bufferIndex > 0) {
                    bufferIndex -= 1
                    "\b \b" print()
                }
            } else if(bufferIndex < 255) {
		if (chr printable?()) {
                	chr print()
                	buffer[bufferIndex] = chr
                	bufferIndex += 1
		}
            } 
        }
    }

    handleCommand: static func (cmd: String, para: String) {
        match cmd {
            case "memory" =>
                "Total Memory: %6i kB" printfln(MM memorySize / 1024)
                "Used Memory:  %6i kB" printfln(MM usedMemory / 1024)
                "Free Memory:  %6i kB" printfln(MM freeMemory / 1024)
            case "alloc" =>
		variables[index] = String new(10) as Pointer
		memcpy(variables[index], "Dis string", 10)
                index += 1
            case "free" =>
		if (index > 0) {
			free(variables[index - 1])
                	index -= 1
		}
	    case "testmem" =>
		passed := true
		arr := gc_calloc(2000, UInt size) as UInt**
		// allocate 2000x1000
		for (i in 0..2000) {
	    		arr[i] = gc_calloc(1000, UInt size) as UInt*
			for (j in 0..1000) {
				arr[i][j] = i*2000 + j
			}
		}
			
		// free first half
		for (i in 0..1000) {
			free(arr[i])
		}

		// allocate first half again
		for (i in 0..1000) {
	    		arr[i] = gc_calloc(1000, UInt size) as UInt*
			for (j in 0..1000) {
				arr[i][j] = i*2000 + j
			}
		}

		// now test accuracy
		for (i in 0..2000) {
			for (j in 0..1000) {
				if (arr[i][j] != i*2000 + j) {
					passed = false
					break
				}
			}
		}

		// now free all
		for (i in 0..2000) {
			free(arr[i] as Pointer)
		}
		free(arr as Pointer)

		if (passed)
			"test passed" println()
		else
			"test failed" println()	

	    case "print" =>
		for (i in 0..index)
			variables[i] as String println()

	    case "date" =>
		if (para == null) {		
			date := Exports rtcGetDate()
			"%d-%02d-%02d" printfln((date >> 9), (date >> 5) & 0xf, date & 0x1f)
		} else {
			ind1 := para indexOf('-')
			ind2 := para indexOf('-', ind1 + 1)
			if (ind1 == -1 || ind2 == -1) "Syntax is YYYY-MM-DD" println()
			else {
				year := parseInt(para substr(0, ind1))
				mon := parseInt(para substr(ind1 + 1, ind2 - ind1 - 1))
				day := parseInt(para substr(ind2 + 1))
				Exports rtcSetDate(day, mon, year)
				"Date set to %d-%02d-%02d" printfln(year, mon, day)
			}
		}

	    case "time" =>
		if (para == null) {
			time := Exports rtcGetTime()
			"%02d:%02d:%02d" printfln((time >> 12), (time >> 6) & 0x3f, time & 0x3f)
		} else {
			ind1 := para indexOf(':')
			ind2 := para indexOf(':', ind1 + 1)
			if (ind1 == -1 || ind2 == -1) "Syntax is HH:MM:SS" println()
			else {
				h := parseInt(para substr(0, ind1))
				m := parseInt(para substr(ind1 + 1, ind2 - ind1 - 1))
				s := parseInt(para substr(ind2 + 1))
				Exports rtcSetTime(h, m, s)				
				"Time set to %02d:%02d:%02d" printfln(h, m, s)
			}
		}

	    case "unixtime" =>
		"%d" printfln(Exports rtcUnixTime())

	    case "ticks" =>
		"%ld (%ld ms)" printfln(Exports rtcGetTicks(), Exports rtcGetTicks() * 1000 / 8192)

        }
    }
}
