/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GFX_CLIENTCANVASLAYER_H
#define GFX_CLIENTCANVASLAYER_H

#include "mozilla/layers/CanvasClient.h"  // for CanvasClient, etc
#include "ClientLayerManager.h"         // for ClientLayerManager, etc
#include "CopyableCanvasLayer.h"        // for CopyableCanvasLayer
#include "Layers.h"                     // for CanvasLayer, etc
#include "mozilla/Attributes.h"         // for MOZ_OVERRIDE
#include "mozilla/RefPtr.h"             // for RefPtr
#include "mozilla/layers/LayerTransaction.h"  // for CanvasLayerAttributes, etc
#include "mozilla/mozalloc.h"           // for operator delete
#include "nsAutoPtr.h"                  // for nsRefPtr
#include "nsDebug.h"                    // for NS_ASSERTION
#include "nsRegion.h"                   // for nsIntRegion
#include "nsTraceRefcnt.h"              // for MOZ_COUNT_CTOR, etc

using namespace mozilla::gfx;

namespace mozilla {
namespace layers {

class CompositableClient;
class ShadowableLayer;

class ClientCanvasLayer : public CopyableCanvasLayer,
                          public ClientLayer
{
  typedef CanvasClient::CanvasClientType CanvasClientType;
public:
  ClientCanvasLayer(ClientLayerManager* aLayerManager) :
    CopyableCanvasLayer(aLayerManager,
                        static_cast<ClientLayer*>(MOZ_THIS_IN_INITIALIZER_LIST()))
  {
    MOZ_COUNT_CTOR(ClientCanvasLayer);
  }
  virtual ~ClientCanvasLayer()
  {
    MOZ_COUNT_DTOR(ClientCanvasLayer);
    if (mCanvasClient) {
      mCanvasClient->OnDetach();
      mCanvasClient = nullptr;
    }
  }

  virtual void SetVisibleRegion(const nsIntRegion& aRegion)
  {
    NS_ASSERTION(ClientManager()->InConstruction(),
                 "Can only set properties in construction phase");
    CanvasLayer::SetVisibleRegion(aRegion);
  }
  
  virtual void Initialize(const Data& aData);

  virtual void RenderLayer();
  
  virtual void FillSpecificAttributes(SpecificLayerAttributes& aAttrs)
  {
    aAttrs = CanvasLayerAttributes(mFilter, mBounds);
  }

  virtual Layer* AsLayer() { return this; }
  virtual ShadowableLayer* AsShadowableLayer() { return this; }
  
  virtual void Disconnect()
  {
    mCanvasClient = nullptr;
    ClientLayer::Disconnect();
  }

  virtual CompositableClient* GetCompositableClient() MOZ_OVERRIDE
  {
    return mCanvasClient;
  }
protected:
  ClientLayerManager* ClientManager()
  {
    return static_cast<ClientLayerManager*>(mManager);
  }
  
  CanvasClientType GetCanvasClientType()
  {
    if (mGLContext) {
      return CanvasClient::CanvasClientGLContext;
    }
    return CanvasClient::CanvasClientSurface;
  }

  RefPtr<CanvasClient> mCanvasClient;

  friend class DeprecatedCanvasClient2D;
  friend class CanvasClient2D;
  friend class DeprecatedCanvasClientSurfaceStream;
};
}
}

#endif
