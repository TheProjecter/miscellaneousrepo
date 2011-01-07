import Kernel, Multiboot, Panic, Bochs, Modules
import structs/Bitmap

include c_types

PageTableEntry: cover from _PageTableEntry {
        present,
	readwrite,
	user,
	writethrough,
	cachedisable,
	accessed,
	dirty,
	attributeindex,
	global,
	reserved,
	address: extern UInt32
}

PageDirEntry: cover from _PageDirEntry {
        present,
	readwrite,
	user,
	writethrough,
	cachedisable,
	accessed,
	available,
	pagesize,
	global,
	reserved,
	address: extern UInt32
}

HeapHead: cover {
	size: UInt
	magic: UInt
}

HeapFoot: cover {
	head: HeapHead*
	magic: UInt
}

MM: cover {
    MAGIC_FOOT := static 0x01231527 as UInt
    MAGIC_FREE := static 0x01241527 as UInt
    MAGIC_USED := static 0x01251527 as UInt

    heapStart := static Pointer
    heapEnd := static Pointer
    lastHead := static null as HeapHead*
  
    HEAP_START_ADDR := static 0xE0000000 as UInt
    HEAP_MAX_ADDR := static 0xF0000000 as UInt
    HEAP_INIT_SIZE := static 0x8000 as UInt
    HEAP_BLOCK_SIZE := static 0x4 as UInt
    HEAP_STRUCT_SIZE := static 0x8 as UInt

    // Once set to true, all memory will be allocated using page tables
    setupDone := static false

    /// Memory region covered by one page. (4kB)
    PAGE_SIZE := static 4096 as UInt

    PAGE_TABLE_ADDR := static 0xFC000000 as UInt // note: this assumes that when a new directory is created, all of its page tables are allocated at the same time (in consecutive pages); thus 0xFC000000 is pt[0], 0xFC000004 is pt[1], 0xFC0003FC is pt[1023] - 1024th page table from 1st page directory, and 0xFC001000 is pt[1024] - 1st page table from 2nd page directory
    PAGE_DIR_ADDR := static 0xFC3F0000 as UInt // 3F0 is the index of the page dir mapping all tables

    pd := static PAGE_DIR_ADDR as PageDirEntry*
    pt := static PAGE_TABLE_ADDR as PageTableEntry*

    pageCount := static 0 as UInt

    /// Amount of memory (in bytes) in the computer.
    memorySize: static SizeT

    /// Total amount of memory in use.
    usedMemory: static SizeT
   
    /// Amount of free memory (in bytes).
    freeMemory: static SizeT {
        get {
            if(usedMemory > memorySize) {
                Bochs warn("The amount of allocated memory is higher than the amount of available memory!")
                return 0
            }
            memorySize - usedMemory
        }
    }

    /** Bitmap pages (each page is a 4 kB memory area). If a bit is
        set, the corresponding page is used. */
    bitmap: static Bitmap

    /// The last-used index in the Bitmap array.
    lastElement: static UInt

    heapInit: static func {
	heapStart = HEAP_START_ADDR as Pointer
	heapEnd = HEAP_START_ADDR as Pointer
	heapExtend(HEAP_INIT_SIZE)
    }

    heapExtend: static func(size: SizeT) -> Pointer {
	if (heapEnd as UInt + ((size+0xfff)&~0xfff) > HEAP_MAX_ADDR) {
		Bochs warn("Heap memory exhausted")
		panic("Heap memory exhausted")
		return null
	}

	head := heapEnd as HeapHead*

	// allocate pages
	i := 0
	while (i < (size+0xfff)>>12) {
		mapAddress(heapEnd as UInt + (i<<12))
		i += 1
	}
	heapEnd = (heapEnd as UInt + (i<<12)) as Pointer
	head@ size = (size+0xfff)&~0xfff // align size to 0x1000
	head@ magic = MAGIC_FREE
	foot := (heapEnd as UInt - HEAP_STRUCT_SIZE) as HeapFoot*
	foot@ magic = MAGIC_FOOT
	foot@ head = head

	return heapMerge(head)
    }

    heapMerge: static func(head: HeapHead*) -> Pointer {
	foot := (head as UInt + head@ size - HEAP_STRUCT_SIZE) as HeapFoot*
	prevFoot := (head as UInt - HEAP_STRUCT_SIZE) as HeapFoot*

	// merge with previous
	if (prevFoot as UInt < heapEnd as UInt && prevFoot as UInt > HEAP_START_ADDR && prevFoot@ head@ magic == MAGIC_FREE) {
		prevFoot@ head@ size += head@ size
		foot@ head = prevFoot@ head
		if (lastHead as UInt == head as UInt)
			lastHead = prevFoot@ head
		head = prevFoot@ head
	}

	// merge with next
	nextHead := (head as UInt + head@ size) as HeapHead*
	if (nextHead as UInt < heapEnd as UInt && nextHead@ magic == MAGIC_FREE) {
		if (lastHead as UInt == nextHead as UInt)
			lastHead = head
		head@ size += nextHead@ size
		foot = (head as UInt + head@ size - HEAP_STRUCT_SIZE) as HeapFoot*
		foot@ head = head
	}

	return head
    }

    alloc: static func (size: SizeT) -> Pointer {
	// try to find a free heap
	head := lastHead ? lastHead as HeapHead* : heapStart as HeapHead*
	size = (size + 2*HEAP_STRUCT_SIZE + HEAP_BLOCK_SIZE-1) & ~(HEAP_BLOCK_SIZE-1)

	bestHead := null as HeapHead*
	bestSize := -1 as UInt
	foundFree := false

	while (head as UInt < heapEnd as UInt) {
		if (!foundFree && head@ magic == MAGIC_FREE) {
			lastHead = head
			foundFree = true
		}

		if (head@ magic != MAGIC_FREE || head@ size < size) {
			head = (head as UInt + head@ size) as HeapHead*
			continue
		}

		if (head@ size == size) {
			// fits
			head@ magic = MAGIC_USED
			usedMemory += size
			if (!foundFree) {
				lastHead = head // start from this block next time
			}

			return (head as UInt + HEAP_STRUCT_SIZE) as Pointer
		} else if (bestSize > head@ size) {
			// keep looking
			bestSize = head@ size
			bestHead = head
			head = (head as UInt + head@ size) as HeapHead*
			continue
		}

		head = (head as UInt + head@ size) as HeapHead*		
	}

	if (!bestHead) {
		// none found, create new
		bestHead = heapExtend(size)

		// check size, if not match will split
		if (bestHead@ size == size) {
			bestHead@ magic = MAGIC_USED
			usedMemory += size
			if (!foundFree) {
				lastHead = heapEnd as HeapHead* // no free blocks
			}
			return (bestHead as UInt + HEAP_STRUCT_SIZE) as Pointer
		}
	}

	// not perfect fit, make a new one at the end of the current one
	// with remaining bytes

	// check if there will be enough remaining bytes
	req := (1 + 2*HEAP_STRUCT_SIZE + HEAP_BLOCK_SIZE-1) & ~(HEAP_BLOCK_SIZE-1)
	if (req > bestHead@ size - size) {
		// no need to split
		bestHead@ magic = MAGIC_USED
		usedMemory += bestHead@ size
		if (!foundFree) {
			lastHead = heapEnd as HeapHead* // no free blocks
		}
		return (bestHead as UInt + HEAP_STRUCT_SIZE) as Pointer			
	}

	foot := (bestHead as UInt + size - HEAP_STRUCT_SIZE) as HeapFoot*
	nexthead := (bestHead as UInt + size) as HeapHead*
	if (!foundFree) {
		lastHead = nexthead
	}
	nextfoot := (bestHead as UInt + bestHead@ size - HEAP_STRUCT_SIZE) as HeapFoot*
	foot@ head = bestHead
	foot@ magic = MAGIC_FOOT
	nextfoot@ head = nexthead
	nexthead@ size = bestHead@ size - size
	nexthead@ magic = MAGIC_FREE
	bestHead@ size = size
	bestHead@ magic = MAGIC_USED

	usedMemory += size
	return (bestHead as UInt + HEAP_STRUCT_SIZE) as Pointer
    }


    free: static func (ptr: Pointer) {
	addr := ptr as UInt

	if ((addr - HEAP_STRUCT_SIZE) & (HEAP_BLOCK_SIZE - 1)) {
		Bochs warn("free() failed, address not aligned")
		return
	}

	// mark heap structure as free
	head := (addr - HEAP_STRUCT_SIZE) as HeapHead*
	if (head@ magic != MAGIC_USED) {
		Bochs warn("Attempted to free block that was not allocated")
		return
	}

	foot := (head as UInt + head@ size - HEAP_STRUCT_SIZE) as HeapFoot*
	if (foot@ magic != MAGIC_FOOT || foot@ head as UInt != head as UInt) {
		Bochs warn("Attempted to free block with invalid footer")
		return
	}

	if (!lastHead || addr < lastHead as UInt)
		lastHead = head

	head@ magic = MAGIC_FREE
	usedMemory -= head@ size
    }

    // maps given virtual address, returns corresponding physical address
    mapAddress: static func(vaddr: UInt) -> UInt {
	pdind := vaddr >> 22
	ptind := vaddr >> 12

	// allocate page table if not already present
	if (!pd[pdind] present) {
		pd[pdind] present = 1
		pd[pdind] readwrite = 1
		pd[pdind] address = firstPage()
		usedMemory += PAGE_SIZE // is used memory until the table is deleted
		invalidatePage((pd[pdind]&) as UInt) // flush changes so we can access the new pt via virtual memory
	}

	address := firstPage() // does not count as used memory yet
	pt[ptind] present = 1
	pt[ptind] readwrite = 1
	pt[ptind] address = address
	invalidatePage(address)
	
	return address
    }

    // returns true if virtual address is mapped, false otherwise
    isMapped: static func(vaddr: UInt) -> Bool {
	pdind := vaddr >> 22
	ptind := vaddr >> 12

	if (!pd[pdind] present || !pt[ptind] present) {
		return false
	}	

	return true
    }

    // note: pages are allocated only when initializing the MM
    // and when mapping an unmapped space
    firstPage: static func -> UInt {
        // If we can't find a free page, we will try again from the
        // beginning if the lastElement wasn't already 0.
        tryAgain := lastElement != 0

        for(elem in lastElement..bitmap size) {
            if(bitmap allSet?(elem))
                continue

            for(bit in 0..32) {
                if(bitmap clear?(elem, bit)) {
                    // We've found ourselves a free bit, allocate and return it.
                    bitmap set(elem, bit)
                    lastElement = elem
                    return elem * 32 + bit
                }
            }

            lastElement += 1
        }

        // Maybe some pages we've already looked through have become available
        if(tryAgain) {
            lastElement = 0
            return firstPage()
        }
        
        // If still nothing was found, the entire bitmap is set, and there is
        // no free memory!
        panic("The physical memory manager did not find any free physical pages!")
        return 0
    }

    allocPage: static func ~address (address: SizeT) {
        address /= PAGE_SIZE
        bitmap set(address / 32, address & 31)
    }

    // note: pages should only be freed if they were mapped for a temporary space
    // i.e. a userland app, there is no connection between available memory
    // and free pages
    freePage: static func (address: SizeT) {
        address /= PAGE_SIZE
        bitmap clear(address / 32, address & 31)
    }

    setup: static func () {
	usedMemory = placementAddress as UInt - Kernel end as UInt
	Bochs debug("Used so far: %d" format(usedMemory))

	ent := multiboot mmapAddr as MMapEntry*
	maxAddr := 0
	while (ent as UInt < multiboot mmapAddr + multiboot mmapLength) {
		Bochs debug("MMEntry size = %08x" format(ent@ size))
		ent@ size += 4
			
		// RAM entry
		if (ent@ type == 1 && ent@ baseAddrLow + ent@ lengthLow > maxAddr)
			maxAddr = ent@ baseAddrLow + ent@ lengthLow

		// next
		ent = (ent as UInt + ent@ size) as MMapEntry*
	}

	if (maxAddr == 0) {
		// no RAM entries, use memUpper, >> 2 since memUpper is in KiB
		pageCount = (multiboot memUpper >> 2) + 256
		Bochs debug ("no ram, pages: %d" format(pageCount))
	} else {
		pageCount = maxAddr >> 12 // it's assumed that maxAddr is aligned with PAGE_SIZE
		Bochs debug ("pages: %d" format(pageCount))
	}

	// we use 1 bit per page.
	elementCount := pageCount / 32

	// Add an extra element for the remainder of the pages if not aligned
	if(pageCount & 31) {
		elementCount += 1
		bitmap = Bitmap new(elementCount)
	} else
		bitmap = Bitmap new(elementCount)

	allocatable := 0

	// Set up allocateable space
	ent = multiboot mmapAddr as MMapEntry*
	while (ent as UInt < multiboot mmapAddr + multiboot mmapLength)
	{		
		if (ent@ type == 1) { // if entry type is RAM
			base := (ent@ baseAddrLow + 0xFFF) / PAGE_SIZE
			len := ent@ lengthLow / PAGE_SIZE
			allocatable += len * PAGE_SIZE

			for (i in base..base + len) {
				bitmap clear(i/32, i & 31) 
			}

			Bochs debug("baselow: %08x base high: %08x length low: %d length high: %d allocatable: %d" format(ent@ baseAddrLow, ent@ baseAddrHigh, ent@ lengthLow, ent@ lengthHigh, allocatable))
		}
		// next
		ent = (ent as UInt + ent@ size) as MMapEntry*
	}
	
	// Get used page count for kernel and MM initialization
	kernelPages := ((placementAddress as UInt - Kernel start as UInt) + 0xfff) >> 12
	allocatable -= kernelPages * PAGE_SIZE

	// Fill page bitmap
	num := kernelPages/32
	// kernel starts at an address aligned with PAGE_SIZE
	begin := Kernel start as UInt / (PAGE_SIZE * 32)
	for (i in begin..begin + num) {
		bitmap setAll(i)
	}
	bitmap applyMask(begin + num, (1 << (kernelPages & 31)) - 1)

	firstPage() // mark first page as unusable
	usedMemory += PAGE_SIZE


	// Mark Multiboot's pages as taken
	// - Structure
	Bochs debug ("multiboot addr = %08x" format(multiboot& as UInt)) 
	if (bitmap clear?(multiboot& as UInt >> 17, (multiboot& as UInt >> 12) & 31)) {
		bitmap set(multiboot& as UInt >> 17, (multiboot& as UInt >> 12) & 31)
		usedMemory += PAGE_SIZE
	}
	Bochs debug ("module count = %d" format(multiboot modsCount))
	Bochs debug ("multiboot modsAddr = %08x" format(multiboot modsAddr)) 

	// - Module List
	modPages := (multiboot modsCount * 4 * UInt32 size + 0xFFF)>>12
	for (i in 0..modPages) {
		modAddr := multiboot modsAddr + (i << 12)
		Bochs debug("marking address at %08x for module list structure" format(modAddr))
		usedMemory += PAGE_SIZE
		bitmap set(modAddr >> 17, (modAddr >> 12) & 31)
	}

	// - Modules
	mods := multiboot modsAddr as MultibootModule*
	for (i in 0..multiboot modsCount) {
		mod := gc_malloc(3 * UInt32 size) as Module*
		mod@ start = mods[i] moduleStart
		mod@ end = mods[i] moduleEnd
		Modules add(mod)

		len := (mods[i] moduleEnd - mods[i] moduleStart + 0xFFF) >> 12

		for (j in 0..len) {
			addr := (mods[i] moduleStart & ~0xFFF) + (j<<12)
			Bochs debug("marking address at %08x for module %d" format(addr, i))
			usedMemory += PAGE_SIZE
			bitmap set(addr >> 17, (addr >> 12) & 31)
		}
	}

	Bochs debug("Kernel start: %08x Kernel end: %08x" format(Kernel start, Kernel end))

	// set the real value for the memory size
       	memorySize = (multiboot memLower + multiboot memUpper) * 1024
	Bochs debug("Bitmap size: %i B" format(bitmap size * 4))

	// create page directory
	cur_page_directory := (firstPage() << 12) as PageDirEntry*
	usedMemory += PAGE_SIZE
	for (i in 0..1024) {
		cur_page_directory[i] present = 0
		cur_page_directory[i] readwrite = 1
		cur_page_directory[i] user = 0
	}

	page_table := (firstPage() << 12) as PageTableEntry* 
	usedMemory += PAGE_SIZE
	// use 0 - 0x003ff ffff (lowest 4mb) for the kernel,
	address := 0
	for (i in 0..0x400) {
		// kernel addresses
		page_table[i] address = address >> 12
		page_table[i] present = 1
		page_table[i] readwrite = 1
		page_table[i] user = 0
		address += PAGE_SIZE
	}
	cur_page_directory[0] address = page_table as UInt >> 12
	cur_page_directory[0] present = 1
	cur_page_directory[0] readwrite = 1
	cur_page_directory[0] user = 0

	page_table = (firstPage() << 12) as PageTableEntry*
	usedMemory += PAGE_SIZE
	// set up heap at 0xe0000000 - 0xefffffff (this is only the initial 1024 tables)
	address = firstPage() << 12
	for (i in 0..0x400) {
		// heap addresses
		page_table[i] address = address >> 12
		page_table[i] present = 1
		page_table[i] readwrite = 1
		page_table[i] user = 0
		address = firstPage() << 12
		usedMemory += PAGE_SIZE
	}
	ind := HEAP_START_ADDR >> 22
	cur_page_directory[ind] address = page_table as UInt >> 12
	cur_page_directory[ind] present = 1
	cur_page_directory[ind] readwrite = 1
	cur_page_directory[ind] user = 0

	// map the pts at 0xFC000000
	ind = PAGE_TABLE_ADDR >> 22
	cur_page_directory[ind] address = cur_page_directory as UInt >> 12
	cur_page_directory[ind] present = 1
	cur_page_directory[ind] readwrite = 1
	cur_page_directory[ind] user = 0

	Bochs debug("Taking plunge")

	// switch address space and enable paging
	switchAddressSpace(cur_page_directory as UInt*)
	Bochs debug("Switched")
	activatePaging()
	Bochs debug("Activated")
	invalidatePage(PAGE_TABLE_ADDR)
	Bochs debug("Invalidated")
	setupDone = true

	heapInit();
	Bochs debug("Heap inited")

	Bochs debug("Total usable memory: %d" format(memorySize)) // includes isolated chunks
	Bochs debug("Total allocatable memory: %d" format(allocatable))
	memorySize = allocatable
	Bochs debug("Used memory: %d" format (usedMemory))
	Bochs debug("Free memory: %d" format (freeMemory))

	start := -1
	Bochs debug("Free memory map:")
	for (i in 0..bitmap size) {
		for (j in 0..32) {
			if (start != -1 && bitmap set?(i, j)) {
				Bochs debug("%08x - %08x" format(start, ((i<<5) + j)<<12))
				start = -1
			}

			if (start == -1 && !bitmap set?(i, j))
				start = ((i<<5) + j)<<12
		}
	}
    }

    activatePaging: static func {
        setCR0(getCR0() bitSet(31))
    }

    switchAddressSpace: static extern proto func (addressSpace: UInt*)
    getCR0: static extern proto func -> SizeT
    setCR0: static extern proto func (cr0: SizeT)
    invalidatePage: static extern proto func (addr: UInt)
}
