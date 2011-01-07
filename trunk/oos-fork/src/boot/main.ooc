import Kernel, Exports, Multiboot, memory/MM, Panic, Bochs, Console

kmain: func (mb: MultibootInfo*, magic: UInt32) {
    multiboot = mb@
    Kernel setup()

    "Welcome to " print()
    Exports setFgColorFn(Color lightGreen as UInt16, ||
        "oos-fork" print()
    )
    " booted by " print()
    Exports setFgColorFn(Color lightBlue as UInt16, ||
        multiboot bootLoaderName as String print()
    )
    ".\n" println()

    "Available commands are: memory, alloc, free, testmem, print, date, time, unixtime, ticks\n" println()

    "Total Memory: %6i kB" printfln(MM memorySize / 1024)
    "Used Memory:  %6i kB" printfln(MM usedMemory / 1024)
    "Free Memory:  %6i kB" printfln(MM freeMemory / 1024)
    '\n' print()

    Console run()
}
