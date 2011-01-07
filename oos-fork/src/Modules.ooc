import Bochs, Panic, memory/MM, ELF, devices/[CPU, Ports], devices/cpu/IRQ

Module: cover {
	start, end: UInt32
	name: Char*
	exports: UInt32*
}

KSymbol: cover {
	name: Char*
	value: UInt32
	next: KSymbol*
}

ModuleInfo: cover {
	Magic: UInt32 	//!< Identifying magic value (See ::MODULE_MAGIC)
	Version: UInt32	//!< Module Version in Major.Minor 8.8 form
	Name: UInt32 	//!< Module Name/Identifier
	Exports: UInt32
	Init: UInt32
}

Modules: cover {
	symbols := static null as KSymbol*

	MM_MODULE_GRANUALITY := static 0x10000 as SizeT // 32 KiB
	MM_MODULE_MIN := static 0xD0000000 as SizeT // lowest module address
	MM_MODULE_MAX := static (0xF0000000 - MM_MODULE_GRANUALITY) as SizeT // highest module address

	mods: static UInt32*

	number: static SizeT
	idx := static 0 as SizeT

	setModules: static func (num: SizeT) {
		number = num
		mods = gc_malloc(UInt32 size * number) as UInt32*
	}

	add: static func (mod: Module*) {
		mods[idx] = mod as UInt32
		idx += 1
	}

	setup: static func {
		// export symbols
		exportSymbol("bochsDebug", (Bochs debug&) as UInt32)
		exportSymbol("irqHandlerInstall", (IRQ handlerInstall&) as UInt32)
		exportSymbol("portsInByte", (Ports inByte&) as UInt32)
		exportSymbol("portsOutByte", (Ports outByte&) as UInt32)
		exportSymbol("gc_malloc", (gc_malloc&) as UInt32)
		exportSymbol("free", (free&) as UInt32)
		exportSymbol("memcpy", (memcpy&) as UInt32)
		exportSymbol("enableInterrupts", (CPU enableInterrupts&) as UInt32)
		exportSymbol("disableInterrupts", (CPU disableInterrupts&) as UInt32)

		for (i in 0..idx) {
			mod := (mods[i] as Module*)@
			
			Bochs debug("idx: %d, moduleStart: %08x, moduleEnd: %08x" format(i, mod start, mod end))
			ehdr := (mod start as Elf32_Ehdr*)@
			if (ehdr Ident1 != 0x464C457F) {

				Bochs debug("Error: could not load module %d, ident was 0x%08x" format(ehdr Ident1))
				continue
			}

			Bochs debug("program header count: %d, entry point: 0x%08x" format(ehdr phentcount, ehdr entrypoint))
			phtab := (mod start + ehdr phoff) as Elf32_Phdr*
		
			pages := 0
			base := -1 as UInt32
			dynamicTab := 0 as Elf32_Dyn*

			// find a valid base
			for (j in 0..ehdr phentcount) {
				if (phtab[j] Type == HeaderType PT_LOAD) {
					pages += ((phtab[j] VAddr & 0xFFF) + phtab[j] MemSize + 0xFFF) >> 12
					if (phtab[j] VAddr < base) base = phtab[j] VAddr
				}

				if (phtab[j] Type == HeaderType PT_DYNAMIC) {
					dynamicTab = phtab[j] VAddr as Elf32_Dyn*
				}
			}

			// relocate base to be inside the modules memory region
			baseDiff := 0
			oldBase := base
			if (base < MM_MODULE_MIN || base > MM_MODULE_MAX || base + (pages<<12) > MM_MODULE_MAX) {
				base = 0
			}	

			physical := gc_malloc(UInt32 size * pages) as UInt32*
			virtual := gc_malloc(UInt32 size * pages) as UInt32*
			sizes := gc_malloc(UInt32 size * pages) as UInt32*

			pageIdx := 0

			for (j in 0..ehdr phentcount) {
				if (phtab[j] Type == HeaderType PT_LOAD) {
					Bochs debug("phtab[%i] = {VAddr:0x%x, MemSize:0x%x}" format(j, phtab[j] VAddr, phtab[j] MemSize))

					// determine how much space the last page will need
					lastSize := 0
                			if( (phtab[j] FileSize & 0xFFF) < 0x1000 - (phtab[j] VAddr & 0xFFF) )
                        			lastSize = phtab[j] FileSize
                			else
                        			lastSize = (phtab[j] FileSize & 0xFFF) + (phtab[j] VAddr & 0xFFF)
                			lastSize &= 0xFFF

                			// count = number of pages needed to store this program header
                			count := ((phtab[j] VAddr & 0xFFF) + phtab[j] FileSize + 0xFFF) >> 12
                			for (k in 0..count) {
						virtual[pageIdx+k] = phtab[j] VAddr + (k<<12) // virtual address for this page
						physical[pageIdx+k] = phtab[j] Offset + (k<<12)   // Store the offset (file offset i guess) in the physical address

						if(k != 0) { // why is this needed for all but the first page?
						        physical[pageIdx+k] -= virtual[pageIdx+k] & 0xFFF // subtract non-aligned part from offset
						        virtual[pageIdx+k] &= ~0xFFF // make sure virtual address is aligned
						}

						if (k == count-1)
						        sizes[pageIdx+k] = lastSize       // Byte count in page
						else if (k == 0)
						        sizes[pageIdx+k] = 4096 - (phtab[j] VAddr & 0xFFF)
						else
						        sizes[pageIdx+k] = 4096
					}

					// pages needed for memory
					kk := count
					count = (((phtab[j] VAddr & 0xFFF) + phtab[j] MemSize + 0xFFF) >> 12)
					// FIXME: i think memsize pages need to be adjusted here if difference with filesize is less than one page.. we'll see
/*
					if (phtab[j] MemSize != phtab[j] FileSize && count == kk)
	count += 1
*/

					for (k in kk..count) {
						physical[pageIdx+k] = 0xffffffff
						virtual[pageIdx+k] = phtab[j] VAddr + (k<<12)

						if (k != 0)
							virtual[pageIdx+k] &= ~0xFFF // for all but first, align virtual address

						// determine size for last
						if (k == count-1 && (phtab[j] MemSize & 0xFFF))
						        sizes[pageIdx+k] = phtab[j] MemSize & 0xFFF // Byte count in page
						else
						        sizes[pageIdx+k] = 4096
					}

					pageIdx += count
				}
			}


			// check if any page's space is already taken
			if (base != 0) {
				for(k in 0..pages) {
					// test if taken
					if (MM isMapped(virtual[k] & ~0xFFF)) {
						base = 0
						Bochs debug("Address 0x%x is taken" format(virtual[k] & ~0xFFF))
						break
					}
				}
			}			

			// check if the executable has no base or it is not free
			if (base == 0) {
				// If so, give it a base
				base = MM_MODULE_MIN
				while (base < MM_MODULE_MAX) {
					// note: they need to be contiguous free pages
					k := 0
					while (k < pages) {
						addr := virtual[k] & ~0xFFF

						// adjust address to reflect new base
						addr -= oldBase
						addr += base

						if (MM isMapped(addr)) break // address taken, skip it

						k += 1
					}

					// If space was found, break
					if(k == pages) break
					// Else increment pointer and try again
					base += MM_MODULE_GRANUALITY
				}
			}

			for (k in 0..pages) {
				addr := virtual[k] & ~0xFFF // get previous page aligned to 0x1000

				// adjust address to reflect new base
				addr -= oldBase
				addr += base

				// map addr
				MM mapAddress(addr)

				if (physical[k] == 0xffffffff) { // memory page
					// set all bytes in vaddr + (virtual&0xfff) to vaddr+0x1000 to 0
					memset((addr + (virtual[k] & 0xfff)) as Pointer, 0, 0x1000 - (virtual[k] & 0xfff))
					Bochs debug("zeroing page at 0x%08x" format(addr))
				} else { // load program header page
					// read from modstart + physical to modstart + physical + size, put at vaddr + (virtual&0xfff) where vaddr is the address in the modules memory region
					memcpy((addr + (virtual[k] & 0xfff)) as Pointer, (mod start + physical[k]) as Pointer, sizes[k])
					Bochs debug("copying program header into page at 0x%08x" format(addr))
				}
			}

			baseDiff = base - oldBase
			Bochs debug("old base: 0x%08x new base: 0x%08x base diff: 0x%08x" format(oldBase, base, baseDiff))

			if (dynamicTab as UInt32 != 0) {
				dynamicTab = (dynamicTab as UInt32 + baseDiff) as Elf32_Dyn*
				ELF relocate(base, baseDiff, dynamicTab)
			} else {
				Bochs debug("no dynamic program header was found")
			}

			infoAddr := 0 as UInt32
			if (ELF getSymbol(base, "_Module_Info" as Char*, infoAddr&)) {
				info := infoAddr as ModuleInfo*

				exportCount := (infoAddr - info@ Exports) / UInt32 size
				Bochs debug("initializing module: %s, magic: 0x%08x version: %d.%d init: 0x%08x, export count: %d" format(info@ Name, info@ Magic, info@ Version >> 16, info@ Version & 0xffff, info@ Init, exportCount))
				(mods[i] as Module*)@ name = info@ Name as Char*
				(mods[i] as Module*)@ exports = info@ Exports as UInt32*

				fn: Func ()
				c := fn& as Closure*
				c@ thunk = info@ Init as Pointer
				fn()
			} else {
				Bochs warn("could not find _Module_Info section in module")
			}

			free(physical)
			free(virtual)
			free(sizes)
		}
	}

	exportSymbol: static func(name: String, value: UInt32) {
		Bochs debug("exporting symbol '%s' at address 0x%08x" format(name, value))

		len := name length()
		symbol := gc_malloc(3 * UInt32 size) as KSymbol*
		symbol@ name = gc_malloc(len) as Char*
		for (i in 0..len) {
			symbol@ name[i] = name[i] 			
		}
		symbol@ name[len] = 0

		symbol@ value = value	
		symbol@ next = symbols

		symbols = symbol
	}

	getExport: static func(module: String, exportIdx: UInt32) -> UInt32 {
		for (i in 0..idx) {
			if ((mods[i] as Module*)@ name == module)
				return (mods[i] as Module*)@ exports[exportIdx]
		}

		Bochs warn ("could not find module '%s'" format(module))
		return 0
	}

	getSymbol: static func(name: Char*, value: UInt32*) -> Bool {
		// Scan Kernel
		sym := symbols
		while (sym as UInt32 != 0) {
			if (name == sym@ name) {
				value@ = sym@ value
				return true
			}

			sym = sym@ next
		}
	
		// FIXME: scan Loaded libraries
		/*for(pKBin = glLoadedKernelLibs;
			pKBin;
			pKBin = pKBin->Next )
		{
			if( Binary_FindSymbol(pKBin->Base, Name, Value) ) {
				return true
			}
		}*/
	
		Bochs warn("Unable to find symbol '%s'" format(name))
		return false
	}

}
