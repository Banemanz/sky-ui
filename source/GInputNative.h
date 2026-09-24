#pragma once
#include "GInputAPI.h"

// GInput is built with MSVC. MinGW's C++ destructor vtable layout is different;
// never dispatch an MSVC interface through a MinGW virtual call expression.
inline bool SkyGInputHasPad(IGInputPad* pad) {
#if defined(__MINGW32__)
    using Fn = bool(__thiscall*)(const IGInputPad*);
    auto vtable = *reinterpret_cast<void***>(pad);
    return reinterpret_cast<Fn>(vtable[2])(pad);
#else
    return pad->HasPadInHands();
#endif
}
inline void* SkyGInputEvent(IGInputPad* pad, eGInputEvent event, void* data, bool isConst = false) {
#if defined(__MINGW32__)
    using Fn = void*(__thiscall*)(IGInputPad*, eGInputEvent, void*);
    auto vtable = *reinterpret_cast<void***>(pad);
    return reinterpret_cast<Fn>(vtable[isConst ? 5 : 4])(pad, event, data);
#else
    return isConst ? pad->SendConstEvent(event, data) : pad->SendEvent(event, data);
#endif
}
