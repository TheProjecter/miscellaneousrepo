import devices/[CPU, FDD], memory/MM, Bochs, Modules, Console, Exports

Kernel: cover {
    start: static Pointer = kernelStart&
    end:   static Pointer = kernelEnd&

    setup: static func() {
        MM setup()
        CPU setup()
        CPU enableInterrupts()
	Modules setup()
	Exports setup()

	FDD setup()
    }
}

// from the linker.ld linker script
kernelStart: extern proto Int
kernelEnd:   extern proto Int
