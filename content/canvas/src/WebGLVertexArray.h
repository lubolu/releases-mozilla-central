/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBGLVERTEXARRAY_H_
#define WEBGLVERTEXARRAY_H_

#include "WebGLObjectModel.h"
#include "WebGLBuffer.h"
#include "WebGLVertexAttribData.h"

#include "nsWrapperCache.h"

#include "mozilla/LinkedList.h"

namespace mozilla {

class WebGLVertexArray MOZ_FINAL
    : public nsISupports
    , public WebGLRefCountedObject<WebGLVertexArray>
    , public LinkedListElement<WebGLVertexArray>
    , public WebGLContextBoundObject
    , public nsWrapperCache
{
// -----------------------------------------------------------------------------
// PUBLIC
public:

    // -------------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR

    WebGLVertexArray(WebGLContext *context);

    ~WebGLVertexArray() {
        DeleteOnce();
    };


    // -------------------------------------------------------------------------
    // IMPLMENET PARENT CLASSES

    void Delete();

    WebGLContext* GetParentObject() const {
        return Context();
    }

    virtual JSObject* WrapObject(JSContext *cx,
                                 JS::Handle<JSObject*> scope) MOZ_OVERRIDE;

    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(WebGLVertexArray)


    // -------------------------------------------------------------------------
    // MEMBER FUNCTIONS

    bool HasEverBeenBound() { return mHasEverBeenBound; }
    void SetHasEverBeenBound(bool x) { mHasEverBeenBound = x; }
    WebGLuint GLName() const { return mGLName; }

    bool EnsureAttribIndex(WebGLuint index, const char *info);


// -----------------------------------------------------------------------------
// PRIVATE
private:

    // -------------------------------------------------------------------------
    // MEMBERS

    WebGLuint mGLName;
    bool mHasEverBeenBound;
    nsTArray<WebGLVertexAttribData> mAttribBuffers;
    WebGLRefPtr<WebGLBuffer> mBoundElementArrayBuffer;


    // -------------------------------------------------------------------------
    // FRIENDSHIPS

    friend class WebGLContext;
};

} // namespace mozilla

#endif
