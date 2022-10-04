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

structure that holds `CFG` and `DFG`. `BasicBlock`s are stored in vector, also there are pointers to 1st and last `BasicBlock`. `BasicBlock`s id matches it's position in vector. also, `Graph` stores all constants in separate map. this is not currently used, but may be useful for firther use

# BasicBlock (`bb.h`, `bb.cpp`)

structure that holds `Inst`ructions in an intrusive list. Also contains vector of successors and predecessors, needed for CFG. Instuctions are stored in following order:

```
nullptr
  |
phi_0
  |
phi_1
  |
phi_2   <---- first_phi_
  |     <---- first_inst_
inst_0
  |
inst_1
  |
inst_2
  |
nullptr <---- last_inst_
```

it is done in order to separate instructions and phi's, but still be able to iterate over all of them if needed (as it is unused now, might change: f.ex. add last_phi for convinience, etc)

# Inst (`inst.h`, `inst.cpp`)

instructions are impllemented using OOP. all needed extensions are encoded as `Has...` if extension contains data. Using these extensions, specific instruction types are constructed.

# usage

`test.cpp` contains example
