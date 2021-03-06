#pragma once

#include "ContextObject.h"
#include <RadeonProRender.h>
#include <cstdint>

namespace rprf
{
class Plugin;
class Scene;
class FrameBuffer;

class Context : public ContextObject<rpr_context>
{
public:
	Context(const Plugin& plugin, const char* cachePath = nullptr, int createFlags = RPR_CREATION_FLAGS_ENABLE_GPU0);

	void setScene(const Scene& scene);
	void setAOV(const FrameBuffer& frameBuffer);
	void unsetAOV();
	void resolve(FrameBuffer* src, FrameBuffer* dst, bool noDisplayGamma = true);

	void setParameter1u(int name, unsigned int value);
	void setParameter1f(int name, float value);

	void render();

	int createFlags() const { return m_createFlags; }
private:
	int m_createFlags;
};

}
