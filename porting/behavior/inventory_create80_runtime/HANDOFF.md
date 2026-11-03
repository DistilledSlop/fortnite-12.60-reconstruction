# Item creation source recovery80

One uncounted draft of14205f090, class selection from the item definition,
class ancestry gates, selected-class allocation or fallback, owner/entry copying,
parent update, adjacent-byte write and captured-vtable initialization callback.
The selected allocator returning null does not retry fallback. The final class
null test follows ancestry reads in the source. No pre-zeroed output is invented.
Full C source reviewed; class/layout/virtual ABI, constructor/allocator semantics,
entry ownership and initialization remain open. Related fallback142040900 is
recovered in broad_dependencies80_runtime, with raw RAX preservation supported
by its assembly epilogue. No compiler, tests, generation, runtime or promotion.
