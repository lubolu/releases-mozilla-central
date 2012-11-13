/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(jsion_baseline_registers_x64_h__) && defined(JS_ION)
#define jsion_baseline_registers_x64_h__

#include "ion/IonMacroAssembler.h"

namespace js {
namespace ion {

static const Register BaselineFrameReg    = rbp;
static const Register BaselineStackReg    = rsp;

static const ValueOperand R0(rcx);
static const ValueOperand R1(rbx);
static const ValueOperand R2(rax);

// BaselineTailCallReg and BaselineStubReg reuse
// registers from R2.
static const Register BaselineTailCallReg = rsi;
static const Register BaselineStubReg     = rdi;

} // namespace ion
} // namespace js

#endif

