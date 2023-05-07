# IR DESIGN RATIONALE

- IR form is CFG + DFG in SSA form
- I believe that IR is mostly defined by ISA, so it's important for implementing classes like `Graph`, `BasicBlock` and `Instruction` to be able to configure themselves according to any changes in ISA. Here are the invariants of the IR:
  - each `BasicBlock` can have unlimited number of predecessors.
  - each `BasicBlock` must have an `BRANCH` instruction as it's last instruction. This instruction defines number of this block's successors. `BRANCH` instruction may only be at the end of the `BasicBlock`.
  - each `BasicBlock` without successors must have a `BRANCH` instruction at it's end with exactly 0 successors.
  - each `Instruction` must have fixed signature. If inputs are dynamic, it must be specified so.
- Graph must have the following properties:
  - each `Graph` must have a `START_BLOCK`. This block is only used to store graph's constants and parameters. Constants and parameters may only be in `START_BLOCK`.
  - [TODO:] each `Graph` must have an `END_BLOCK`. This block must remain empty and all `BasicBlock` without successors must link to it.
  - [TODO:] make it deductible whether `Instruction` can have users or not

# ISA DESIGN

Having all of this in mind, I propose the following definition of ISA `Instruction`:

- `OPCODE` - defines `Instruction` unique name.
- `SIGNATURE` - defines `Instruction` number of inputs.
- `FLAGS` - define any unique properties of instruction, like `NO_DCE`, `BRANCH`, etc.
