# libSceContentDelete technical debt

### Functional

- [sceContentDeleteById and sceContentDeleteByPath](../../../core/libs/prx/libSceContentDelete/Export.cpp) (libSceContentDelete) use the `int64_t contentId` and UTF-8 path signatures exposed by [SharpProspero](https://github.com/SvenGDK/SharpProspero/blob/main/src/SharpProspero/Interop/Content/ContentDelete.cs). A null or empty path returns `0x809D5001` (`INVALID_PARAM`, also used by KytyPS5 initialization), while a valid delete request returns SharpProspero's measured `0x809D5006` (`NO_PERMISSION`): AnyPS5 has no console content/capture library that a guest is allowed to delete. Calls still require the library to be initialized. Whether a real console reports `NO_PERMISSION`, a not-found error, or another service error for content absent from its database is unverified.
