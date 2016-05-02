#ifndef GBUFFER_H
#define GBUFFER_H

#include <GL/glew.h>

class GBuffer
{
public:
    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };

private:
    GLuint m_fbo;
    GLuint m_textures[GBUFFER_NUM_TEXTURES];
    GLuint m_depthTexture;

public:
    GBuffer();
    ~GBuffer();

    bool Init(unsigned int screenWidth, unsigned int screenHeight);

    void BindForWriting();

    void BindForReading();

    void SetReadBuffer(GBUFFER_TEXTURE_TYPE);
}; 

#endif