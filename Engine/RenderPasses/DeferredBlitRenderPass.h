#ifndef DEFERREDBLITRENDERPASS_H
#define DEFERREDBLITRENDERPASS_H

#include "Renderer/RenderPassI.h"

#include "GBuffer.h"

class DeferredBlitRenderPass : public RenderPassI
{
public:
	virtual ~DeferredBlitRenderPass ();

	virtual void Init ();
	virtual RenderVolumeCollection* Execute (Scene* scene, Camera* camera, RenderVolumeCollection* rvc);
protected:
	void EndDrawing (GBuffer* gBuffer);
};

#endif