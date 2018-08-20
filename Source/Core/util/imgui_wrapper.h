#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#include <external/imgui/imgui.h>
#include <util/gl_render_state.h>
#include <resource.h>
#include <texture2d.h>
#include <util/gfxm.h>
#include <common.h>

extern ImGuiContext* imGuiCtx;
extern GLuint imGuiVBuf;
extern GLuint imGuiIBuf;
extern gl::ShaderProgram* imGuiProgram;
inline void ImGuiInit()
{
    imGuiProgram = new gl::ShaderProgram();
    imGuiCtx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)Common.frameSize.x;
    io.DisplaySize.y = (float)Common.frameSize.y;
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    resource<Texture2D> font_atlas = resource<Texture2D>::get("$imgui_font_atlas");
    font_atlas->Data(pixels, width, height, 4);
    io.Fonts->TexID = (void*)font_atlas;

    ImGui::SetCurrentContext(imGuiCtx);

    io.KeyMap[ImGuiKey_Backspace] = 8;
    io.KeyMap[ImGuiKey_Enter] = 13;

    glGenBuffers(1, &imGuiVBuf);
    glGenBuffers(1, &imGuiIBuf);

    gl::Shader imGuiVS;
    gl::Shader imGuiFS;
    imGuiVS.Init(GL_VERTEX_SHADER);
    imGuiFS.Init(GL_FRAGMENT_SHADER);
    imGuiVS.Source("#version 450\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n");
    imGuiFS.Source("#version 450\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n");
    imGuiVS.Compile();
    imGuiFS.Compile();
    imGuiProgram->AttachShader(&imGuiVS);
    imGuiProgram->AttachShader(&imGuiFS);

    imGuiProgram->BindAttrib(0, "Position");
    imGuiProgram->BindAttrib(1, "UV");
    imGuiProgram->BindAttrib(2, "Color");

    imGuiProgram->BindFragData(0, "Out_Color");

    imGuiProgram->Link();
    glUniform1i(imGuiProgram->GetUniform("Texture"), 0);
}
inline void ImGuiCleanup()
{
    ImGui::DestroyContext(imGuiCtx);
    delete imGuiProgram;
}
inline void ImGuiUpdate(float dt)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = dt;
    io.DisplaySize.x = (float)Common.frameSize.x;
    io.DisplaySize.y = (float)Common.frameSize.y;
    ImGui::NewFrame();
}
inline void ImGuiDraw()
{
    glActiveTexture(GL_TEXTURE0);
    //ImGui::ShowDemoWindow();
    ImGui::Render();

    glEnable(GL_SCISSOR_TEST);

    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    
    ImDrawData* draw_data = ImGui::GetDrawData();
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

    gfxm::mat4 proj = 
        gfxm::ortho(0.0f, (float)Common.frameSize.x, (float)Common.frameSize.y, 0.0f, -1.0f, 1.0f);
    imGuiProgram->Use();
    glUniform1i(imGuiProgram->GetUniform("Texture"), 0);
    glUniformMatrix4fv(imGuiProgram->GetUniform("ProjMtx"), 1, GL_FALSE, &proj[0][0]);

    GLuint vao_handle = 0;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, imGuiVBuf);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));


    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, imGuiVBuf);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imGuiIBuf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, ((Texture2D*)pcmd->TextureId)->GetGlName());
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }
    glDeleteVertexArrays(1, &vao_handle);

    glDisable(GL_SCISSOR_TEST);
}

#endif
