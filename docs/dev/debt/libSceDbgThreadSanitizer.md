# libSceDbgThreadSanitizer technical debt

### Functional

- [__tsan_acquire, __tsan_release](../../../core/libs/prx/libSceDbgThreadSanitizer/Export.cpp) (libSceDbgThreadSanitizer, NIDs `XyX1104ACcU` and `IkoI44e0OXc`) use the `void (void* addr)` ABI from LLVM compiler-rt's [tsan_interface.h](https://github.com/llvm/llvm-project/blob/87f0227cb60147a26a1eeb4fb06e3b505e9c7261/compiler-rt/include/sanitizer/tsan_interface.h#L24-L25), whose names hash to the imported NIDs. AnyPS5 always reports ThreadSanitizer disabled through `sceKernelIsThreadSanitizerEnabled`, so these annotations are no-ops in the current runtime. PPSA23226's bundled `libSceJobManager_nosubmission.prx` calls them only after that enable query. If AnyPS5 ever gains an active race detector, these functions will need to forward acquire/release annotations to it. The console ABI remains inferred rather than hardware-measured.
