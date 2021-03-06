#include "Oasis/Graphics/GL/GLGraphicsDevice.h" 

#include "Oasis/Core/Engine.h" 
#include "Oasis/Core/Display.h" 
#include "Oasis/Graphics/GL/GLIndexBuffer.h" 
#include "Oasis/Graphics/GL/GLRenderTexture2D.h" 
#include "Oasis/Graphics/GL/GLShader.h"
#include "Oasis/Graphics/GL/GLTexture2D.h" 
#include "Oasis/Graphics/GL/GLUtil.h"  
#include "Oasis/Graphics/GL/GLVertexBuffer.h" 

#include <GL/glew.h> 

#include <string> 

using namespace std; 

namespace Oasis 
{

static const string GL_VS = R"(#version 120 
attribute vec3 a_Position; 
uniform mat4 oa_Model; 
uniform mat4 oa_View; 
uniform mat4 oa_Proj; 
void main() 
{
    gl_Position = oa_Proj * oa_View * oa_Model * vec4(a_Position, 1.0); 
}
)"; 

static const string GL_FS = R"(#version 120
uniform vec3 u_Color; 
void main() 
{
    gl_FragColor = vec4(u_Color, 1.0); 
}
)";

static const GLuint PRIMITIVE_TYPES[(int) Primitive::count] =
{
    GL_LINES,
    GL_LINE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP
};

GLGraphicsDevice::GLGraphicsDevice() 
{
    for (int i = 0; i < GetMaxTextureUnitCount(); i++) 
    {
        textureUnits_[i] = nullptr; 
    }
    for (int i = 0; i < GetMaxRenderTargetCount(); i++) 
    {
        renderTargets_[i] = nullptr; 
    }
}

GLGraphicsDevice::~GLGraphicsDevice() {} 

void GLGraphicsDevice::PreRender() 
{
    // Logger::Debug("Graphics: PreRender"); 

    Display* d = Engine::GetDisplay(); 

    GLCALL(glEnable(GL_BLEND)); 
    GLCALL(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA)); 
    GLCALL(glEnable(GL_DEPTH_TEST)); 

    SetShader(nullptr); 
    SetVertexBuffer(nullptr); 
    SetIndexBuffer(nullptr); 
    SetViewport(0, 0, d->GetWidth(), d->GetHeight()); 
    SetClearColor(0.7, 0.8, 0.9); 
    Clear(); 

    if (!fbo_) GLCALL(glGenFramebuffers(1, &fbo_)); 
    if (!drawFBO_) GLCALL(glGenFramebuffers(1, &drawFBO_)); 
    if (!readFBO_) GLCALL(glGenFramebuffers(1, &readFBO_)); 
}

void GLGraphicsDevice::PostRender() 
{
    // Logger::Debug("Graphics: PostRender"); 
}

void GLGraphicsDevice::SetClearColor(float r, float g, float b) 
{
    clearColor_ = { r, g, b }; 
}

void GLGraphicsDevice::Clear(bool color, bool depth) 
{
    SetupFramebuffer(); 

    GLCALL(glClearColor(clearColor_.x, clearColor_.y, clearColor_.z, 1)); 
    unsigned bits = 0; 

    if (!color && !depth) return; 

    if (color) bits |= GL_COLOR_BUFFER_BIT; 
    if (depth) bits |= GL_DEPTH_BUFFER_BIT; 

    GLCALL(glClear(bits)); 
}

void GLGraphicsDevice::SetShader(Shader* shader) 
{
    shaderProgram_ = dynamic_cast<GLShader*>(shader); 
}

void GLGraphicsDevice::SetIndexBuffer(IndexBuffer* ib) 
{
    indexBuffer_ = dynamic_cast<GLIndexBuffer*>(ib); 
}

void GLGraphicsDevice::SetVertexBuffers(int count, VertexBuffer** vbs) 
{
    vertexBuffers_.clear(); 

    for (int i = 0; i < count; i++) 
    {
        GLVertexBuffer* vb = dynamic_cast<GLVertexBuffer*>(vbs[i]); 
        if (vb) vertexBuffers_.push_back(vb); 
    }
}

void GLGraphicsDevice::SetTextureUnit(int unit, Texture* texture) 
{
    Texture* set = nullptr; 

    if (texture) 
    {
        // make sure texture is a GLTexture 
        switch (texture->GetType()) 
        {
        case TextureType::TEXTURE_2D: 
            set = dynamic_cast<GLTexture2D*>(texture); 
            break; 
        case TextureType::RENDER_TEXTURE_2D: 
            set = dynamic_cast<GLRenderTexture2D*>(texture); 
            break; 
        case TextureType::TEXTURE_CUBE: 
            // TODO 
            break; 
        default: break; 
        }

        textureUnits_[unit] = set; 
    }
    else 
    {
        textureUnits_[unit] = nullptr; 
    }
}

void GLGraphicsDevice::SetViewport(int x, int y, int w, int h) 
{
    viewport_ = Vector4(x, y, w, h); 
    GLCALL(glViewport(x, y, w, h)); 
}

void GLGraphicsDevice::Draw(Primitive prim, int start, int triCount) 
{
    // TODO 
    (void) prim; 
    (void) start; 
    (void) triCount; 
}  

void GLGraphicsDevice::DrawIndexed(Primitive prim, int start, int triCount) 
{
    (void) prim; 

    if (!indexBuffer_) return; 

    if (PrepareToDraw()) 
    {
        GLCALL(glDrawElements(/*PRIMITIVE_TYPES[(int) prim]*/ GL_TRIANGLES, triCount, GL_UNSIGNED_SHORT, (void*)(start * sizeof (short)))); 

        PostDraw(); 
    } 
}  

Shader* GLGraphicsDevice::CreateShader(const string& vs, const string& fs) 
{
    return new GLShader(this, vs, fs); 
}

IndexBuffer* GLGraphicsDevice::CreateIndexBuffer(int numElements, BufferUsage usage)
{
    return new GLIndexBuffer(this, numElements, usage); 
}  

VertexBuffer* GLGraphicsDevice::CreateVertexBuffer(int numElements, const VertexFormat& format, BufferUsage usage) 
{
    return new GLVertexBuffer(this, numElements, format, usage); 
}  

Texture2D* GLGraphicsDevice::CreateTexture2D(TextureFormat format, int width, int height) 
{
    return new GLTexture2D(this, format, width, height); 
}

RenderTexture2D* GLGraphicsDevice::CreateRenderTexture2D(TextureFormat format, int width, int height, int samples) 
{
    return new GLRenderTexture2D(this, format, width, height, samples); 
}

int GLGraphicsDevice::GetMaxRenderTargetCount() 
{
    return 4; 
}

void GLGraphicsDevice::ClearRenderTargets(bool color, bool depth) 
{
    if (color) 
    {
        for (int i = 0; i < GetMaxRenderTargetCount(); i++) 
        {
            if (renderTargets_[i]) 
            {
                Texture* tex = renderTargets_[i]; 

                switch (tex->GetType()) 
                {
                case TextureType::RENDER_TEXTURE_2D: 
                    ((GLRenderTexture2D*) tex)->SetInUse(false); 
                    break; 
                default: // not supported 
                    break; 
                }
            }
            renderTargets_[i] = nullptr; 
        }
    }
    
    if (depth) 
    {
        if (depthTarget_) 
        {
            Texture* tex = depthTarget_; 
            switch (tex->GetType()) 
            {
            case TextureType::RENDER_TEXTURE_2D: 
                ((GLRenderTexture2D*) tex)->SetInUse(false); 
                break; 
            default: // not supported 
                break; 
            }
        }
        depthTarget_ = nullptr; 
    }
}

void GLGraphicsDevice::SetRenderTarget(int index, RenderTexture2D* texture) 
{
    renderTargets_[index] = texture ? dynamic_cast<GLRenderTexture2D*>(texture) : nullptr; 
}

void GLGraphicsDevice::SetDepthTarget(RenderTexture2D* texture) 
{
    depthTarget_ = texture ? dynamic_cast<GLRenderTexture2D*>(texture) : nullptr; 
}

bool GLGraphicsDevice::PrepareToDraw() 
{
    if (!shaderProgram_) return false; 

    shaderProgram_->Update(); 

    if (indexBuffer_) indexBuffer_->Update(); 
    
    for (int i = 0; i < GetVertexBufferCount(); i++) 
    {
        if (vertexBuffers_[i]) vertexBuffers_[i]->Update(); 
    }

    // TODO 
    // GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_->GetId())); 
    // GLCALL(glUseProgram(shaderProgram_->GetId())); 

    BindIndexBuffer(indexBuffer_->GetId()); 
    BindShader(shaderProgram_->GetId()); 

    int attribs[(int) Attribute::count]; 

    for (int i = 0; i < (int) Attribute::count; i++) attribs[i] = -1; 

    for (int i = 0; i < GetVertexBufferCount(); i++) 
    {
        if (!vertexBuffers_[i]) continue; 

        GLVertexBuffer* vb = vertexBuffers_[i]; 
        const VertexFormat& format = vb->GetVertexFormat(); 

        for (int j = 0; j < format.GetAttributeCount(); j++) 
        {
            Attribute attrib = format.GetAttribute(j); 

            if (attribs[(int) attrib] == -1) attribs[(int) attrib] = i; 
        }
    }

    for (int i = 0; i < (int) Attribute::count; i++) 
    {
        if (attribs[i] == -1) 
        {
            // GLCALL(glDisableVertexAttribArray(GLShader::GetAttributeIndex((Attribute) i))); 
            SetVertexAttribArrayEnabled(GLShader::GetAttributeIndex((Attribute) i), false); 
            continue; 
        }

        GLVertexBuffer* vb = vertexBuffers_[attribs[i]]; 
        SetVertexAttribPointer(
            GLShader::GetAttributeIndex((Attribute) i),  
            vb->GetId(),  
            GetAttributeSize((Attribute) i), 
            vb->GetVertexFormat().GetSize() * sizeof (float), 
            vb->GetVertexFormat().GetOffset((Attribute) i) * sizeof (float) 
        ); 
    }

    for (int i = 0; i < GetMaxTextureUnitCount(); i++) 
    {
        // GLCALL(glActiveTexture(GL_TEXTURE0 + i)); 

        Texture* tex = textureUnits_[i]; 

        if (tex) 
        {
            tex->Update(); 

            switch (tex->GetType()) 
            {
            case TextureType::TEXTURE_2D:  
                // GLCALL(glBindTexture(GL_TEXTURE_2D, ((GLTexture2D*) tex)->GetId())); 
                BindTexture2D(i, ((GLTexture2D*) tex)->GetId()); 
                break; 
            case TextureType::RENDER_TEXTURE_2D: {
                    // GLCALL(glBindTexture(GL_TEXTURE_2D, ((GLRenderTexture2D*) tex)->GetId())); 
                    GLRenderTexture2D* rt = ((GLRenderTexture2D*) tex); 
                    rt->ResolveTextureIfNeeded(); 

                    if (rt->IsInUse()) 
                    {
                        // Logger::Debug("Using backup texture"); 
                        rt->UpdateBackupTexture(); 
                        BindTexture2D(i, rt->GetBackupId()); 
                    }
                    else 
                    {
                        // Logger::Debug("Using main texture"); 
                        BindTexture2D(i, rt->GetMainId()); 
                    }
                }
                break; 
            default: 
                // GLCALL(glBindTexture(GL_TEXTURE_2D, 0)); 
                BindTexture2D(i, 0); 
                break; 
            }
        }
        else 
        {
            // GLCALL(glBindTexture(GL_TEXTURE_2D, 0)); 
            BindTexture2D(i, 0); 
        }
    }

    SetupFramebuffer(); 

    return true; 
}

void GLGraphicsDevice::PostDraw() 
{
    SetRenderbuffersDirty(); 
}

void GLGraphicsDevice::SetRenderbuffersDirty() 
{
    auto depth = (GLRenderTexture2D*) depthTarget_; 
    if (depth) 
    {
        depth->SetRenderedTo(); 
    }

    for (int i = 0; i < GetMaxRenderTargetCount(); i++) 
    {
        auto tex = (GLRenderTexture2D*) renderTargets_[i]; 
        if (tex) tex->SetRenderedTo(); 
    }
}

void GLGraphicsDevice::ResolveRenderTexture2D(GLRenderTexture2D* texture) 
{
    if (texture && !IsDepthTextureFormat(texture->GetFormat()))  
    {
        // TODO 

        GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO_)); 
        GLCALL(glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, texture->GetRenderbufferId())); 

        GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO_)); 
        GLCALL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->GetMainId(), 0)); 

        GLCALL(glBlitFramebuffer(0, 0, texture->GetWidth(), texture->GetHeight(), 0, 0, texture->GetWidth(), texture->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST)); 

        // reset framebuffer 
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, context_.fbo)); 
    }
}

void GLGraphicsDevice::CopyToBackupTexture(GLuint src, GLuint dst, int width, int height, bool color)
{
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO_)); 
    if (color) 
    {
        GLCALL(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src, 0)); 
    }
    else 
    {
        GLCALL(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, src, 0)); 
    }

    GLCALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO_)); 
    if (color) 
    {
        GLCALL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst, 0)); 
    }
    else 
    {
        GLCALL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dst, 0)); 
    }

    GLCALL(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST)); 

    // reset framebuffer 
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, context_.fbo)); 
}

void GLGraphicsDevice::SetupFramebuffer() 
{
    if (HasCustomRenderTarget()) 
    {
        // bind 
        // GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, fbo_)); 
        BindFramebuffer(fbo_); 

        vector<GLuint> drawBuffers; 
        vector<GLuint> colorBuffers; 
        vector<bool> isRenderbuffer; 

        for (int i = 0; i < GetMaxRenderTargetCount(); i++) 
        {
            if (renderTargets_[i]) 
            {
                // TODO change if RenderTextureCube is added 
                GLRenderTexture2D* tex = (GLRenderTexture2D*) renderTargets_[i]; 

                tex->Update(); 
                tex->SetInUse(true); 

                // Logger::Debug(tex->GetId()); 
                // GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex->GetId(), 0)); 
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i); 
                colorBuffers.push_back(tex->IsMultisampled() ? tex->GetRenderbufferId() : tex->GetMainId()); 
                isRenderbuffer.push_back(tex->IsMultisampled()); 
            }
        }

        if (depthTarget_) 
        {
            GLRenderTexture2D* tex = (GLRenderTexture2D*) depthTarget_; 

            tex->Update(); 
            tex->SetInUse(true); 

            GLuint id = tex->IsMultisampled() ? tex->GetRenderbufferId() : tex->GetMainId(); 

            if (context_.fboContents.depthBuffer != id || context_.fboContents.isDepthRenderbuffer != tex->IsMultisampled())  
            {
                context_.fboContents.depthBuffer = id; 
                context_.fboContents.isDepthRenderbuffer = tex->IsMultisampled(); 

                if (tex->IsMultisampled()) 
                {
                    GLCALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id)); 
                }
                else 
                {
                    GLCALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, id, 0)); 
                }
            }
        }

        // Logger::Debug("DrawBuffers: ", drawBuffers.size()); 

        bool setDrawBuffers = context_.fboContents.numBuffers != drawBuffers.size(); 

        // Logger::Debug("setDrawBuffers: ", setDrawBuffers, " (", context_.fboContents.numBuffers, ")"); 

        if (!setDrawBuffers) 
        {
            for (unsigned i = 0; i < drawBuffers.size(); i++) 
            {
                if (context_.fboContents.drawBuffer[i] != drawBuffers[i] || context_.fboContents.colorBuffer[i] != colorBuffers[i] || context_.fboContents.isRenderbuffer[i] != isRenderbuffer[i])  
                {
                    // Logger::Debug("Different draw/color buffer: ", context_.fboContents.drawBuffer[i], " ", drawBuffers[i], " : ", context_.fboContents.colorBuffer[i], " ", colorBuffers[i]); 
                    setDrawBuffers = true; 
                    break; 
                }
            }
        }

        if (setDrawBuffers) 
        {
            unsigned max = context_.fboContents.numBuffers > drawBuffers.size() ? context_.fboContents.numBuffers : drawBuffers.size(); 

            for (unsigned i = 0; i < max; i++) 
            {
                if (i < drawBuffers.size()) 
                {
                    if (isRenderbuffer[i]) 
                    {
                        GLCALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, colorBuffers[i])); 
                    }
                    else 
                    {
                        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0)); 
                    }
                }
                else 
                {
                    GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0)); 
                }

                context_.fboContents.drawBuffer[i] = drawBuffers[i]; 
                context_.fboContents.colorBuffer[i] = colorBuffers[i]; 
                context_.fboContents.isRenderbuffer[i] = isRenderbuffer[i]; 
            }

            context_.fboContents.numBuffers = drawBuffers.size(); 
            GLCALL(glDrawBuffers(drawBuffers.size(), &drawBuffers[0])); 
        }

        // int status; 

        // GLCALL(status = glCheckFramebufferStatus(GL_FRAMEBUFFER)); 

        // if (status != GL_FRAMEBUFFER_COMPLETE) 
        // {
        //     Logger::Warning("Framebuffer is not complete: ", status); 
        // }
    }
    else 
    {
        // GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 
        BindFramebuffer(0); 
    }
}

bool GLGraphicsDevice::HasCustomRenderTarget() 
{
    if (depthTarget_) return true; 

    for (int i = 0; i < GetMaxRenderTargetCount(); i++) 
    {
        if (renderTargets_[i]) return true; 
    }

    return false; 
}

bool GLGraphicsDevice::BindFramebuffer(GLuint id) 
{
    if (context_.fbo != id) 
    {
        context_.fbo = id; 
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, id)); 
        return true; 
    }

    return false; 
}

bool GLGraphicsDevice::BindIndexBuffer(GLuint id) 
{
    if (context_.ibo != id) 
    {
        context_.ibo = id; 
        GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); 
        return true; 
    }

    return false; 
}

bool GLGraphicsDevice::BindShader(GLuint id) 
{
    if (context_.program != id) 
    {
        context_.program = id; 
        GLCALL(glUseProgram(id)); 
        return true; 
    }

    return false; 
}

bool GLGraphicsDevice::BindTexture2D(GLuint index, GLuint id) 
{
    if (context_.texture[index] != id) 
    {
        if (context_.textureUnit != index) 
        {
            context_.textureUnit = index; 
            GLCALL(glActiveTexture(GL_TEXTURE0 + index)); 
        }

        context_.texture[index] = id; 
        GLCALL(glBindTexture(GL_TEXTURE_2D, id)); 
        return true; 
    }

    return false; 
}

bool GLGraphicsDevice::BindVertexBuffer(GLuint id) 
{
    if (context_.vbo != id) 
    {
        context_.vbo = id; 
        GLCALL(glBindBuffer(GL_ARRAY_BUFFER, id)); 
        return true; 
    }

    return false; 
}

bool GLGraphicsDevice::SetVertexAttribArrayEnabled(GLuint index, bool enabled) 
{
    if (context_.attribArrayEnabled[index] != enabled) 
    {
        if (enabled) 
        {
            GLCALL(glEnableVertexAttribArray(index)); 
        }
        else 
        {
            GLCALL(glDisableVertexAttribArray(index)); 
        }

        context_.attribArrayEnabled[index] = enabled; 

        return true; 
    }

    return false; 
}

bool GLGraphicsDevice::SetVertexAttribPointer(GLuint index, GLuint vbo, GLuint count, GLuint size, GLuint64 offset) 
{
    bool justEnabled = SetVertexAttribArrayEnabled(index, true); 
    
    GLContext::AttribArray& arr = context_.attribArray[index]; 

    if (justEnabled || arr.vbo != vbo || arr.count != count || arr.size != size || arr.offset != offset) 
    {
        BindVertexBuffer(vbo); 
        GLCALL(
            glVertexAttribPointer(
                index,  
                count, 
                GL_FLOAT, 
                GL_FALSE, 
                size, 
                (void*) offset
            )
        ); 

        arr.vbo = vbo; 
        arr.count = count; 
        arr.size = size; 
        arr.offset = offset; 

        return true; 
    }

    return false; 
}

}