# libSceGameUpdate technical debt

### Unknown function info

- [sceGameUpdateGetAddcontLatestVersion](../../../core/libs/prx/libSceGameUpdate/Export.cpp) (libSceGameUpdate) - a null `entitlement_label` returns `0x80412803`, matching prosper and [dlc_emu](https://github.com/drakmor/dlc_emu/blob/fcf69b16d46a9850578bce19dc2c1b4dc867cbb5/src/dlc_modules/dlc_content.cpp#L3108-L3123); KytyPS5 accepts it. Bytes after `found` are zeroed as in KytyPS5; prosper leaves them untouched. Console error precedence remains unverified
