/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla SpiderMonkey JavaScript 1.9 code, released
 * May 28, 2008.
 *
 * The Initial Developer of the Original Code is
 *   Brendan Eich <brendan@mozilla.org>
 *
 * Contributor(s):
 *   David Anderson <danderson@mozilla.com>
 *   David Mandelin <dmandelin@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if !defined jsjaeger_mono_ic_h__ && defined JS_METHODJIT && defined JS_MONOIC
#define jsjaeger_mono_ic_h__

#include "assembler/assembler/MacroAssembler.h"
#include "assembler/assembler/CodeLocation.h"
#include "assembler/moco/MocoStubs.h"
#include "methodjit/MethodJIT.h"
#include "CodeGenIncludes.h"
#include "methodjit/ICRepatcher.h"

namespace js {
namespace mjit {

class FrameSize
{
    uint32 frameDepth_ : 16;
    uint32 argc_;
  public:
    void initStatic(uint32 frameDepth, uint32 argc) {
        JS_ASSERT(frameDepth > 0);
        frameDepth_ = frameDepth;
        argc_ = argc;
    }

    void initDynamic() {
        frameDepth_ = 0;
        argc_ = -1;  /* quiet gcc */
    }

    bool isStatic() const {
        return frameDepth_ > 0;
    }

    bool isDynamic() const {
        return frameDepth_ == 0;
    }

    uint32 staticLocalSlots() const {
        JS_ASSERT(isStatic());
        return frameDepth_;
    }

    uint32 staticArgc() const {
        JS_ASSERT(isStatic());
        return argc_;
    }

    uint32 getArgc(VMFrame &f) const {
        return isStatic() ? staticArgc() : f.u.call.dynamicArgc;
    }
};

namespace ic {

struct GlobalNameIC
{
    typedef JSC::MacroAssembler::RegisterID RegisterID;

    JSC::CodeLocationLabel  fastPathStart;
    JSC::CodeLocationCall   slowPathCall;

    /*
     * - ARM and x64 always emit exactly one instruction which needs to be
     *   patched. On ARM, the label points to the patched instruction, whilst
     *   on x64 it points to the instruction after it.
     * - For x86, the label "load" points to the start of the load/store
     *   sequence, which may consist of one or two "mov" instructions. Because
     *   of this, x86 is the only platform which requires non-trivial patching
     *   code.
     */
    int32 loadStoreOffset   : 15;
    int32 shapeOffset       : 15;
    bool usePropertyCache   : 1;
};

struct GetGlobalNameIC : public GlobalNameIC
{
};

struct SetGlobalNameIC : public GlobalNameIC
{
    JSC::CodeLocationLabel  slowPathStart;

    /* Dynamically generted stub for method-write checks. */
    JSC::JITCode            extraStub;

    /* SET only, if we had to generate an out-of-line path. */
    int32 inlineShapeJump : 10;   /* Offset into inline path for shape jump. */
    int32 extraShapeGuard : 6;    /* Offset into stub for shape guard. */
    bool objConst : 1;          /* True if the object is constant. */
    RegisterID objReg   : 5;    /* Register for object, if objConst is false. */
    RegisterID shapeReg : 5;    /* Register for shape; volatile. */
    bool hasExtraStub : 1;      /* Extra stub is preset. */

    int32 fastRejoinOffset : 16;  /* Offset from fastPathStart to rejoin. */
    int32 extraStoreOffset : 16;  /* Offset into store code. */

    /* SET only. */
    ValueRemat vr;              /* RHS value. */

    void patchInlineShapeGuard(Repatcher &repatcher, int32 shape);
    void patchExtraShapeGuard(Repatcher &repatcher, int32 shape);
};

struct TraceICInfo {
    TraceICInfo() {}

    JSC::CodeLocationLabel stubEntry;
    JSC::CodeLocationLabel jumpTarget;
    JSC::CodeLocationJump traceHint;
    JSC::CodeLocationJump slowTraceHint;
#ifdef DEBUG
    jsbytecode *jumpTargetPC;
#endif
    
    /* This data is used by the tracing JIT. */
    void *traceData;
    uintN traceEpoch;
    uint32 loopCounter;
    uint32 loopCounterStart;

    bool initialized : 1;
    bool hasSlowTraceHint : 1;
};

static const uint16 BAD_TRACEIC_INDEX = (uint16)0xffff;

void JS_FASTCALL GetGlobalName(VMFrame &f, ic::GetGlobalNameIC *ic);
void JS_FASTCALL SetGlobalName(VMFrame &f, ic::SetGlobalNameIC *ic);

struct EqualityICInfo {
    typedef JSC::MacroAssembler::RegisterID RegisterID;

    JSC::CodeLocationLabel stubEntry;
    JSC::CodeLocationCall stubCall;
    BoolStub stub;
    JSC::CodeLocationLabel target;
    JSC::CodeLocationLabel fallThrough;
    JSC::CodeLocationJump jumpToStub;

    ValueRemat lvr, rvr;

    bool generated : 1;
    JSC::MacroAssembler::RegisterID tempReg : 5;
    Assembler::Condition cond;
};

JSBool JS_FASTCALL Equality(VMFrame &f, ic::EqualityICInfo *ic);

struct CallDescription {
    CallDescription(uint32 argc) : argc(argc), code(NULL)
    { }
    JSObject    *callee;
    JSFunction  *fun;
    uint32      argc;
    void        *code;
    bool        unjittable;     // Only set if fun && fun->isInterpreted()
};

struct CallIC {
    typedef JSC::MacroAssembler::RegisterID RegisterID;
    typedef JSC::MacroAssembler::Call Call;

    JSC::CodeLocationLabel fastPathStart;
    JSC::CodeLocationLabel slowPathStart;

    jsbytecode  *pc;
    FrameSize   frameSize;

    uint32      slowCallOffset : 12;        // For repatching the slow call.
    uint32      inlineRejoinOffset : 10;    // Return address for inline calls.
    uint32      completedRejoinOffset : 10; // Return address for completed clals.
    uint32      inlineJumpOffset : 10;      // For patching the inline path.
    uint32      inlineCalleeGuard : 10;     // If there is an extended inline
                                            // path, this is the offest to
                                            // patch the guarded callee obj.
    uint32      inlineCallOffset : 10;      // Offset of jump-to-script instruction

    RegisterID  calleeData : 5;
    bool hasExtendedInlinePath : 1;

    // Offset the return value from JSFrameReg.
    uint32 rvalOffset;

    // If a path has been generated, |pool| is non-NULL.
    mutable JSC::ExecutablePool *pool;

    // inlineCallee is the funobj being guarded on the inline path,
    // if hasExtendedInlinePath is true. Otherwise, it is unused.
    mutable JSObject *inlineCallee;

    // guardedNative is non-NULL if |pool| is non-NULL and a native stub
    // has been generated.
    mutable JSObject *guardedNative;

    bool shouldDisable(JSContext *cx, JITScript *jit, CallDescription &call);
    void purge(JITScript *jit, Repatcher &repatcher);
    void disable(Repatcher &repatcher);
    bool update(JSContext *cx, JITScript *jit, CallDescription &call);
    JSOp op() const;
    void reenable(JITScript *jit, Repatcher &repatcher);
    void purgeInlineGuard(Repatcher &repatcher);
    void purgeStub(Repatcher &repatcher);

    JSC::CodeLocationLabel inlineRejoinLabel() const {
       return fastPathStart.labelAtOffset(inlineRejoinOffset);
    }
    void *returnAddress() const {
        return inlineRejoinLabel().executableAddress();
    }
    JSC::CodeLocationLabel completedRejoinLabel() const {
        return fastPathStart.labelAtOffset(completedRejoinOffset);
    }
    JSC::CodeLocationJump inlineJump() const {
        return fastPathStart.jumpAtOffset(inlineJumpOffset);
    }
    JSC::CodeLocationCall slowCall() const {
        return slowPathStart.callAtOffset(slowCallOffset);
    }
    JSC::CodeLocationDataLabelPtr calleePtr() const {
        JS_ASSERT(hasExtendedInlinePath);
        return fastPathStart.dataLabelPtrAtOffset(inlineCalleeGuard);
    }
    JSC::CodeLocationJump inlineCall() const {
        JS_ASSERT(hasExtendedInlinePath);
        return fastPathStart.jumpAtOffset(inlineCallOffset);
    }
};

template <bool UpdateIC> void * JS_FASTCALL New(VMFrame &f, ic::CallIC *ic);
template <bool DynamicArgc, bool UpdateIC> void * JS_FASTCALL Call(VMFrame &f, ic::CallIC *ic);
template <bool LazyArgsObj> JSBool JS_FASTCALL SplatApplyArgs(VMFrame &f);
void * JS_FASTCALL FailedFunCall(VMFrame &f, ic::CallIC *ic);
void * JS_FASTCALL FailedFunApply(VMFrame &f, ic::CallIC *ic);
void * JS_FASTCALL FailedFunApplyLazyArgs(VMFrame &f, ic::CallIC *ic);

void PurgeMICs(JSContext *cx, JSScript *script);
void SweepICs(JSContext *cx, JSScript *script);
void PurgeICs(JSContext *cx, JSScript *script);

} /* namespace ic */
} /* namespace mjit */
} /* namespace js */

#endif /* jsjaeger_mono_ic_h__ */

