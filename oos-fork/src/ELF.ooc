import Bochs, Panic, memory/MM, Modules

HeaderType: enum {
	PT_LOAD = 1,
	PT_DYNAMIC = 2
}

DynamicType: enum {
	DT_NULL = 0,
	DT_NEEDED = 1,
	DT_PLTRELSZ = 2,
	DT_HASH = 4,
	DT_STRTAB = 5,
	DT_SYMTAB = 6,
	DT_RELA = 7,
	DT_RELASZ = 8,
	DT_RELAENT = 9,
	DT_SONAME = 14,
	DT_REL = 17,
	DT_RELSZ = 18,
	DT_RELENT = 19,
	DT_PLTREL = 20,
	DT_JMPREL = 23
}

RelocationType: enum {
	R_386_32 = 1,
	R_386_PC32 = 2,
	R_386_GLOB_DAT = 6,
	R_386_JMP_SLOT = 7,
	R_386_RELATIVE = 8
}

Elf32_Ehdr: cover {
	// 16 bytes
	Ident1: UInt32
	Ident2: UInt32
	HashTable: UInt32
	SymTable: UInt32

	// 36 bytes
	filetype: UInt16 //!< File Type
	machine: UInt16	//!< Machine / Arch
	version: UInt32	//!< Version (File?)
	entrypoint: UInt32 //!< Entry Point
	phoff: UInt32 //!< Program Header Offset
	shoff: UInt32 //!< Section Header Offset
	flags: UInt32 //!< Flags
	headersize: UInt16 //!< Header Size
	phentsize: UInt16 //!< Program Header Entry Size
	phentcount: UInt16 //!< Program Header Entry Count
	shentsize: UInt16 //!< Section Header Entry Size
	shentcount: UInt16 //!< Section Header Entry Count
	shstrindex: UInt16 //!< Section Header String Table Index	
}

Elf32_Phdr: cover {
	Type: UInt32
	Offset: UInt32
	VAddr: UInt32
	PAddr: UInt32
	FileSize: UInt32
	MemSize: UInt32
	Flags: UInt32
	Align: UInt32
}

Elf32_Dyn: cover {
	d_tag: UInt32
	d_val: UInt32
}

Elf32_Sym: cover {
	namePtr: UInt32
	value: UInt32
	size: UInt32
	info: UInt8
	other: UInt8
	shndx: UInt16
}

Elf32_Rel: cover {
	r_offset: UInt32
	r_info: UInt32
}

Elf32_Rela: cover {
	r_offset: UInt32
	r_info: UInt32
	r_addend: Int
}

ELF: cover {
	SHN_UNDEF := static 0
	STN_UNDEF := static 0

	getSymbol: static func (base: UInt32, name: Char*, ret: UInt32*) -> Bool {
		if (base == 0) return false

		hdr := base as Elf32_Ehdr*
		pBuckets := hdr@ HashTable as UInt32*
		symtab := hdr@ SymTable as Elf32_Sym*

		nbuckets := pBuckets[0]
		pBuckets = pBuckets[2]&
		pChains := pBuckets[nbuckets]&

		// Get hash
		hash := hashString(name)
		hash = hash % nbuckets

		// Check Bucket
		i := pBuckets[hash]
		if (symtab[i] shndx != SHN_UNDEF && symtab[i] namePtr as Char* == name) {
		        if (ret as UInt32 != 0) ret@ = symtab[i] value
		        return true
		}

		// Walk Chain
		while (pChains[i] != STN_UNDEF) {
		        i = pChains[i]
		        if (symtab[i] shndx != SHN_UNDEF && symtab[i] namePtr as Char* == name) {
		                if (ret as UInt32 != 0) ret@ = symtab[i] value
		                return true
		        }
		}
		return false
	}

	hashString: static func (str: Char*) -> UInt32 {
		h := 0 as UInt32
		g: UInt32

		while (str@ != 0) {
		        h = (h << 4) + str@
			g = h & 0xf0000000
		        if (g != 0)
		                h ^= g >> 24
		        h &= ~g
			str += 1
		}
		return h
	}

	relocate: static func(base: UInt32, baseDiff: UInt32, dynamicTab: Elf32_Dyn*) {
		hdr := base as Elf32_Ehdr*

		dynstrtab := 0 as Char*
		dynsymtab := 0 as Elf32_Sym*
		symCount := 0 as UInt32

		rel := 0 as Elf32_Rel*
		rela := 0 as Elf32_Rela*
		relSz := 0 as UInt32
		relaSz := 0 as UInt32
		relEntSz := 8 as UInt32
		relaEntSz := 8 as UInt32
		pltSz := 0 as UInt32
		pltType := 0 as UInt32
		plt := 0 as Pointer
		ptr := 0 as UInt32*

		failed := false
		j := 0

		while (dynamicTab[j] d_tag != DynamicType DT_NULL) {
			match (dynamicTab[j] d_tag) {
				case DynamicType DT_SYMTAB =>
					// symbol table
					dynamicTab[j] d_val += baseDiff
					dynsymtab = dynamicTab[j] d_val as Elf32_Sym*
					Bochs debug ("symtab found at 0x%p" format(dynsymtab))
					hdr@ SymTable = dynamicTab[j] d_val // save it

				case DynamicType DT_STRTAB =>
					// string table
					dynamicTab[j] d_val += baseDiff
					dynstrtab = dynamicTab[j] d_val as Char*
					Bochs debug ("strtab found at 0x%p" format(dynstrtab))
				
				case DynamicType DT_HASH =>
					// hash table
					dynamicTab[j] d_val += baseDiff
					hdr@ HashTable = dynamicTab[j] d_val // save it
					symCount = (dynamicTab[j] d_val as UInt32*)[1]
					Bochs debug ("hashtable found at 0x%08x, symbol count = %d" format(hdr@ HashTable, symCount))
			}

			j += 1
		}

		// alter symbols to true base
		for (i in 0..symCount) {
			dynsymtab[i] value += baseDiff
			dynsymtab[i] namePtr += dynstrtab as UInt
		}

		// parse relocation data
		j = 0
		while (dynamicTab[j] d_tag != DynamicType DT_NULL) {
			Bochs debug("d_tag = %d" format(dynamicTab[j] d_tag))
			match (dynamicTab[j] d_tag) {
				case DynamicType DT_SONAME =>
					// shared library name
					Bochs debug("so name: %s" format((dynstrtab as UInt32 + dynamicTab[j] d_val) as Char*))

				case DynamicType DT_NEEDED =>
					// needed library
					Bochs debug("requires library: %s" format((dynstrtab as UInt32 + dynamicTab[j] d_val) as Char*))

				case DynamicType DT_JMPREL =>
					plt = (dynamicTab[j] d_val + baseDiff) as Pointer

				case DynamicType DT_PLTREL =>
					pltType = dynamicTab[j] d_val

				case DynamicType DT_PLTRELSZ =>
					pltSz = dynamicTab[j] d_val

				// relocation stuff
				case DynamicType DT_REL =>
					rel = (dynamicTab[j] d_val + baseDiff) as Elf32_Rel*

				case DynamicType DT_RELSZ =>
					relSz = dynamicTab[j] d_val			

				case DynamicType DT_RELENT =>
					relEntSz = dynamicTab[j] d_val

				case DynamicType DT_RELA =>
					rela = (dynamicTab[j] d_val + baseDiff) as Elf32_Rela*

				case DynamicType DT_RELASZ =>
					relaSz = dynamicTab[j] d_val			

				case DynamicType DT_RELAENT =>
					relaEntSz = dynamicTab[j] d_val
			}

			j += 1
		}

		// parse relocation entries
		if (rel as UInt32 != 0 && relSz != 0 && relEntSz != 0) {
			Bochs debug("parsing relocation 1")
			j = relSz / relEntSz
			for (i in 0..j) {
		                ptr = (baseDiff + rel[i] r_offset) as UInt32*
		                if( !doRelocate(rel[i] r_info, ptr, ptr@, dynsymtab, base) ) {
		                        failed = true
		                }
			} 

		}

		if (rela as UInt32 != 0 && relaSz != 0 && relaEntSz != 0) {
			Bochs debug("parsing relocation 2")
			j = relaSz / relaEntSz
			for (i in 0..j) {
		                ptr = (baseDiff + rela[i] r_offset) as UInt32*
		                if( !doRelocate(rel[i] r_info, ptr, rela[i] r_addend, dynsymtab, base) ) {
		                        failed = true
		                }
			}
		}

		// process procedure linkage table
		if (plt as UInt32 != 0 && pltSz != 0) {
			Bochs debug("processing PLT")
			if (pltType == DynamicType DT_REL) {
				pltRel := plt as Elf32_Rel*
		                j = pltSz / (2 * UInt32 size)
		                for (i in 0..j) {
		                        ptr = (baseDiff + pltRel[i] r_offset) as UInt32*
		                        if (!doRelocate(pltRel[i] r_info, ptr, ptr@, dynsymtab, base)) {
		                                failed = true
		                        }
		                }
		        } else {
				pltRela := plt as Elf32_Rela*
		                j = pltSz / (3 * UInt32 size)
		                for (i in 0..j) {
		                        ptr = (baseDiff + pltRela[i] r_offset) as UInt32*
		                        if (!doRelocate(pltRela[i] r_info, ptr, pltRela[i] r_addend, dynsymtab, base)) {
		                                failed = true
		                        }
		                }
		        }
		}

		if (failed) {
			Bochs warn("Loading module failed")
			panic("Loading module failed")
		}

	}

	doRelocate: static func (r_info: UInt32, ptr: UInt32*, addend: UInt32, symtab: Elf32_Sym*, base: UInt32) -> Bool {
		type := r_info & 0xFF
         	sym := r_info >> 8
		symname := symtab[sym] namePtr as Char*
		val := 0 as UInt32

		match type {
			case RelocationType R_386_32 =>
        			// Standard 32 Bit Relocation (S+A)
				if (!getSymbol(base, symname, val&) )
					// Search this binary first
				        if( !Modules getSymbol( symname, val& ) )
				                return false

				ptr@ = val + addend

			case RelocationType R_386_PC32 =>
        			// 32 Bit Relocation wrt. Offset (S+A-P)
				if (!getSymbol( base, symname, val& ) )
				        if( !Modules getSymbol( symname, val& ) )
				                return false

				ptr@ = val + addend - (ptr as UInt32)
			
			case RelocationType R_386_GLOB_DAT =>
        			// Absolute Value of a symbol (S)
				if (!getSymbol(base, symname, val& ) )
				        if( !Modules getSymbol( symname, val& ) )
				                return false

				ptr@ = val

			case RelocationType R_386_JMP_SLOT =>
        			// Absolute Value of a symbol (S)
				if (!getSymbol(base, symname, val&))
				        if( !Modules getSymbol( symname, val& ) )
				                return false

				ptr@ = val

			case RelocationType R_386_RELATIVE =>
        			// Base Address (B+A)
				ptr@ = base + addend
		}

		return true
	}

}
