# libSceNpWebApi2 technical debt

### Functional

- [sceNpWebApi2AddWebTraceTag](../../../core/libs/prx/libSceNpWebApi2/Export.cpp) uses the PS4 `sceNpWebApi2AddWebTraceTag(int64_t requestId, const char* value)` signature from [shadPS4](https://github.com/shadps4-emu/shadPS4/blob/443f022450f6857256e780daa3d5a14029a748f7/src/core/libraries/np/np_web_api2/np_web_api2.cpp) and forwards a non-null value through the existing HTTP-header path as `X-Psn-WebTrace-Tag`. A null tag returns `SCE_NP_WEBAPI2_ERROR_INVALID_ARGUMENT`. The offline header backend itself does not retain headers, so the tag has no network-visible effect.
- [sceNpWebApi2PushEventCreateHandle, sceNpWebApi2PushEventAbortHandle, sceNpWebApi2PushEventDeleteHandle and sceNpWebApi2PushEventSetHandleTimeout](../../../core/libs/prx/libSceNpWebApi2/Export.cpp) keep push handles associated with their library context. Abort and timeout reject an unknown context or a handle belonging to another context using the PS4 lib-context/handle-not-found codes from shadPS4. Abort is recorded locally, but no push wait exists in the offline backend, and handle timeouts are not consumed. The PS5 signatures and error precedence remain unverified.
- [sceNpWebApi2SetRequestTimeout](../../../core/libs/prx/libSceNpWebApi2/Export.cpp) uses the PS4 `(int64_t requestId, uint32_t timeout)` signature from shadPS4. Requests created by this backend are tracked until deletion and their timeout value is retained; setting a timeout after deletion returns the PS4 request-not-found code. Network sends are still unavailable, so the stored timeout never expires an actual HTTP transaction. PS5 behavior is unverified.

### Unknown function info

- [sceNpWebApi2GetMemoryPoolStats](../../../core/libs/prx/libSceNpWebApi2/Export.cpp) (libSceNpWebApi2) - imported by PPSA21703's Unity PSN plugin. The signature `(int libCtxId, stats*)` and the layout of the Http2 statistics above (measured on a PS5 for Http2, Ssl and Net) are assumed for this library, not measured. The pool size is the one given to `sceNpWebApi2Initialize`, and the peak and current use are 0. The error codes are unknown, so a null output or an unknown context throws.
