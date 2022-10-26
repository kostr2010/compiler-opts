- Replace UnbindBlock / BindBlock with mark / flag that block is hidden

- Add dependency tree of passes in Analyzer

- add flags like "INVALIDATES_DOM_TREE" "INVALIDATES_RPO", check them whem doing pass (like AddBB, RemoveBB, etc)

- fill immediate dominator
