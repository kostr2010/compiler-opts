# disadvantages

- for now, no cast instruction (uint32_t == uint64_t). it will be added shortly, and painlessly, as there is all needed functionalty for it
- not sure about phi instructions
- ISA might need to be reworked, as i am very unsure in my design

# structure

- `ir/` - source dir for IR (1st assignment)
- `test.cpp` - example file with 1st assignment done

# build

```
mdir build
cd $_
cmake <path_to_project> && make
```

running `test` executable will dump CFG & DFG of 1st assignment

# Graph (`graph.h`, `graph.cpp`)

structure that holds `CFG` and `DFG`. `BasicBlock`s are stored in vector, also there are pointers to 1st and last `BasicBlock`. also, `Graph` stores all constants in separate map. this is not currently used, but may be useful for firther use, as well as parent `Graph` (inlining ?)

# BasicBlock (`bb.h`, `bb.cpp`)
