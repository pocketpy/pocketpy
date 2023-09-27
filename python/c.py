class array(struct):
    item_count: int
    item_size: int

    def __new__(cls, item_count: int, item_size: int = 1):
        obj = struct.__new__(cls, item_count * item_size)
        obj._enable_instance_dict()
        obj.item_count = item_count
        obj.item_size = item_size
        return obj
    
    def __getitem__(self, index: int) -> struct:
        if index < 0 or index >= self.item_count:
            raise IndexError("array index out of range")
        p = self.addr() + self.item_size * index
        return p.read_struct(self.item_size)
    
    def __setitem__(self, index: int, value: struct) -> None:
        if index < 0 or index >= self.item_count:
            raise IndexError("array index out of range")
        if value.sizeof() != self.item_size:
            raise ValueError(f"array item size mismatch: {value.sizeof()} != {self.item_size}")
        p = self.addr() + self.item_size * index
        p.write_struct(value)

    def __len__(self) -> int:
        return self.item_count
