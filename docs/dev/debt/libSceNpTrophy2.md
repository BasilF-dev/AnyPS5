# libSceNpTrophy2 technical debt

### Silent stubs

- [libSceNpTrophy2](../../../core/libs/prx/libSceNpTrophy2/src/GameInfo.cpp): `sceNpTrophy2RegisterUnlockCallback` accepts a callback that is never called, as no trophy is ever unlocked, `sceNpTrophy2UnregisterUnlockCallback` only returns success, and `sceNpTrophy2ShowTrophyList` returns success without showing a list (there is no system trophy UI; seen on PPSA15552).
### Functional

- [Trophy icons](../../../core/libs/prx/libSceNpTrophy2/src/Context.cpp) (libSceNpTrophy2) use the `sceNpTrophy2GetRewardIcon(context, handle, trophyId, buffer, size)` signature exposed by [SharpProspero](https://github.com/SvenGDK/SharpProspero/blob/main/src/SharpProspero/Interop/Np/NpTrophy2.cs). The emulated trophy set has no icon assets, so game, group, trophy and reward icon queries set a supplied size to 0 and report `icon file not found` without writing caller image memory. The exact console error-return convention and validation precedence remain unverified.
