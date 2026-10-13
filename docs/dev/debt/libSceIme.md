# libSceIme technical debt

### Unknown function info

- [sceImeGetPanelPositionAndForm](../../../core/libs/prx/libSceIme/Export.cpp) (libSceIme) - the output is taken to be the `PositionAndForm` that `sceImeDialogGetPanelPositionAndForm` fills. A headless open panel reports the type, position, alignment and size derived from the parameters passed to `sceImeOpen`; no host UI position adjustment is performed. A closed panel is reported before a null output, following the same state-first ordering as `sceImeSetCaret`. Console behavior is unverified.

### Functional

- [sceImeOpen, sceImeClose, sceImeSetText, sceImeSetCaret, sceImeSetTextGeometry and sceImeUpdate](../../../core/libs/prx/libSceIme/Export.cpp) (libSceIme) maintain one headless IME panel: there is no on-screen keyboard or event source, but opening validates the core parameter fields, later setters update the caller-owned text buffer and stored panel state, and close releases the session. The error values, basic parameter rules and state ordering follow the shadPS4 reimplementation of the PS4 IME ABI, whose structure layout and 0x80BC error namespace match the declarations used here; PS5 hardware behavior, SDK-version-specific rules and extended-parameter effects are not verified.
- [sceImeKeyboardGetInfo](../../../core/libs/prx/libSceIme/Export.cpp) (libSceIme) returns `0x80bc0023` (no resource id) for every id while a keyboard is open, since the IME keyboard never reports a connected device. [sceImeKeyboardSetMode](../../../core/libs/prx/libSceIme/Export.cpp) validates the mode bits but the mode has no effect, as no IME keyboard events are delivered.
- [sceImeKeyboardGetResourceId](../../../core/libs/prx/libSceIme/Export.cpp) (libSceIme) reports that no USB keyboard is connected (`0x80bc0004`, as [shadPS4](https://github.com/shadps4-emu/shadPS4) does) because IME keyboard events are not delivered to the handler, although [libSceKeyboard](../../../core/libs/prx/libSceKeyboard/src/keyboard_impl.cpp) exposes the host keyboard
