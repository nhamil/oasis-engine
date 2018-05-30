#pragma once

#include "Oasis/Graphics/Graphics.h"

#include "Oasis/Graphics/OpenGL/OGLIndexBuffer.h"
#include "Oasis/Graphics/OpenGL/OGLVertexArray.h"
#include "Oasis/Graphics/OpenGL/OGLShader.h"
#include "Oasis/Graphics/OpenGL/OGLVertexBuffer.h"

namespace Oasis
{

class OASIS_API OGLGraphics : public Graphics
{
public:
    OGLGraphics();
    ~OGLGraphics();

    int GetMaxTextureCount() const;

    void PreRender();
    void PostRender();

    void SetViewport(int x, int y, int w, int h);

    void SetClearColor(const Vector4& color);
    void Clear();

    void SetVertexArray(VertexArray* geom);
    void SetShader(Shader* shader);

    void SetTexture(int unit, Texture* tex);

    Uniform GetUniform(const std::string& name);
    bool HasUniform(const std::string& name);
    bool ClearUniform(const std::string& name);
    bool SetUniform(const std::string& name, int value);
    bool SetUniform(const std::string& name, float value);
    bool SetUniform(const std::string& name, const Vector2& value);
    bool SetUniform(const std::string& name, const Vector3& value);
    bool SetUniform(const std::string& name, const Vector4& value);
    bool SetUniform(const std::string& name, const Matrix3& value);
    bool SetUniform(const std::string& name, const Matrix4& value);

    void DrawArrays(Primitive prim, int start, int primCount);
    void DrawIndexed(Primitive prim, int start, int primCount);

    IndexBuffer* CreateIndexBuffer(int numElements, BufferUsage usage = BUFFER_USAGE_DYNAMIC);
    VertexBuffer* CreateVertexBuffer(int numElements, const VertexFormat& format, BufferUsage usage = BUFFER_USAGE_DYNAMIC);
    VertexArray* CreateVertexArray();
    Shader* CreateShader(const std::string& vs, const std::string& fs);
    Texture2D* CreateTexture2D();

private:
    void BindVertexArray();

    OGLVertexArray* vertexArray_;
    OGLShader* shader_;
};

}
