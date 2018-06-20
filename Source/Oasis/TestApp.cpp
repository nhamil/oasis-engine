#include <Oasis/Common.h>
#include <Oasis/Core/Application.h>
#include <Oasis/Core/Window.h>
#include <Oasis/Graphics/GraphicsDevice.h>
#include <Oasis/Graphics/Mesh.h>
#include <Oasis/Graphics/Parameter.h>

#include <iostream>
#include <string>

#include <stdio.h> 

using namespace Oasis;
using namespace std;

static const string OGL_VS = ""
    "#version 120 \n"
    "attribute vec3 a_Position; \n"
    "uniform mat4 oa_Model; \n"
    "uniform mat4 oa_View; \n"
    "uniform mat4 oa_Proj; \n"
    "void main() { \n"
    "  gl_Position = oa_Proj * oa_View * oa_Model * vec4(a_Position, 1.0); \n"
    "} \n";

static const string OGL_FS = ""
    "#version 120 \n"
    "uniform vec3 u_Color; \n"
    "void main() { \n"
    "  gl_FragColor = vec4(u_Color, 1.0); \n"
    "} \n";

class TestApp : public Application
{
public:
    ~TestApp() {}

    void Init();
    void Update(float dt);
    void Render();
    void Exit();

private:
    Mesh mesh;
    ShaderResource* ShaderResource;
    VertexArrayResource* geom;
    Vector3 pos;
    float angle;
};

void TestApp::Init()
{
    cout << "Init" << endl;

    Parameter p = 3.2;

    cout << "type = " << (int) p.GetType() << " " << (int) ParameterType::INT << endl;

    cout << p.GetInt() << " " << p.GetFloat() << " " << p.GetVector2() << endl;

    cout << (p == 3.2) << endl;

    GraphicsDevice* g = Engine::GetGraphics();

    = g->CreateShader(OGL_VS, OGL_FS);
    geom = g->CreateVertexArray();

    VertexBufferResource* vb = g->CreateVertexBuffer(4, VertexFormat::POSITION);
    IndexBufferResource* ib = g->CreateIndexBuffer(6);

    float verts[] =
    {
        -0.5, -0.5, 0,
         0.5, -0.5, 0,
         0.5,  0.5, 0,
        -0.5,  0.5, 0,
    };

    short inds[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    vb->SetData(0, 4, verts);

    ib->SetData(0, 6, inds);

    geom->SetVertexBuffer(vb);
    geom->SetIndexBuffer(ib);

    Vector3 positions[] =
    {
        { -0.5, -0.5, 0 },
        {  0.5, -0.5, 0 },
        {  0.5,  0.5, 0 },
        { -0.5,  0.5, 0 },
    };

    mesh.SetPositions(4, positions);
    mesh.SetIndices(0, 6, inds);
    mesh.Upload();
}

void TestApp::Update(float dt)
{
    Keyboard* keys = Engine::GetKeyboard(); 

    if (keys->IsKeyDown(Key::KEY_ESCAPE)) Engine::Stop(); 

    float move = 5 * dt; 

    if (keys->IsKeyDown(Key::KEY_I)) { pos.z -= move; }
    if (keys->IsKeyDown(Key::KEY_K)) { pos.z += move; }
    if (keys->IsKeyDown(Key::KEY_J)) { pos.x -= move; }
    if (keys->IsKeyDown(Key::KEY_L)) { pos.x += move; }

    angle += 180 * dt * OASIS_TO_RAD;
}

void TestApp::Render()
{
    GraphicsDevice* g = Engine::GetGraphics();
    Window* w = Engine::GetWindow();

    g->SetClearColor({0.6, 0.7, 0.9, 1.0});
    g->Clear();

    g->SetShader(ShaderResource);
    g->SetUniform("u_Color", (Vector3) {1, 1, 0});
    g->SetUniform("oa_View", Matrix4::Translation(-pos));
    g->SetUniform("oa_Model", Matrix4::RotationY(angle));
    g->SetUniform("oa_Proj", Matrix4::Matrix4::Perspective(70 * OASIS_TO_RAD, w->GetAspectRatio(), 0.1, 100.0));

    g->SetVertexArray(mesh.GetVertexArray(0));
    g->DrawIndexed(Primitive::TRIANGLE_LIST, 0, 2);
}

void TestApp::Exit()
{
    delete geom->GetVertexBuffer(0);
    delete geom;
}

OASIS_MAIN(TestApp)