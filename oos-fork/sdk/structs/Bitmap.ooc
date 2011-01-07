Bitmap: class {
    size: UInt
    data: UInt32*

    init: func (=size) {
        data = gc_malloc(UInt32 size * size) as UInt32*
        memset(data, -1 as UInt32, UInt32 size * size)
    }

    setAll: func(index: UInt) {
        data[index] = 0xFFFFFFFF
    }

    clearAll: func(index: UInt) {
        data[index] = 0x00000000
    }

    set: func (index, bit: UInt) {
        data[index] |= (1 << bit)
    }

    applyMask: func (index, mask: UInt) {
        data[index] |= mask
    }

    set?: func (index, bit: UInt) -> Bool {
        (data[index] & (1 << bit)) as Bool
    }

    allSet?: func (index: UInt) -> Bool {
        data[index] == 0xFFFFFFFF
    }

    clear: func (index, bit: UInt) {
        data[index] &= ~(1 << bit)
    }

    clear?: func (index, bit: UInt) -> Bool {
        !(data[index] & (1 << bit))
    }
}
