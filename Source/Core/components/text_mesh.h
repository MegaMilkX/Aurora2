#ifndef TEXT_MESH_H
#define TEXT_MESH_H

#include "../scene_object.h"
#include <component.h>

#include "renderer.h"

#include <font_data.h>
#include "../lib/nimbusmono_bold.otf.h"

#undef GetObject

struct TextVertex
{
    float x, y, z;
    float u, v;
    float r, g, b;
};

class TextMesh : public SceneObject::Component
{
    CLONEABLE(TextMesh)
    RTTR_ENABLE(SceneObject::Component)
public:
    TextMesh()
    {
        static FontData* fd = CreateDefaultFontData();
        fontData = fd;
    }
    ~TextMesh()
    {
        vao.Cleanup();
    }
    void SetText(const std::string& text)
    {
        this->text = text;
        RebuildMesh();
    }
    
    void RebuildMesh()
    {/*
        std::vector<TextVertex> vertexData;
        std::vector<unsigned short> indexData;
        float advX = 0.0f, advY = 0.0f;
        unsigned short index = 0;
        for(unsigned i = 0; i < text.size(); ++i)
        {
            char charCode = text[i];
            GlyphInfo* glyph = fontData->GetGlyph(charCode);
            vertexData.push_back(
                { advX, 0.0f, 0.0f,
                  0.0f, 0.0f,
                  1.0f, 1.0f, 1.0f }
            );
            vertexData.push_back(
                { advX + glyph->width, 0.0f, 0.0f,
                  1.0f, 0.0f,
                  1.0f, 1.0f, 1.0f }
            );
            vertexData.push_back(
                { advX + glyph->width, glyph->height, 0.0f,
                  1.0f, 1.0f,
                  1.0f, 1.0f, 1.0f }
            );
            vertexData.push_back(
                { advX, glyph->height, 0.0f,
                  0.0f, 1.0f,
                  1.0f, 1.0f, 1.0f }
            );
            advX += glyph->advX;
            advY += glyph->advY;
            indexData.push_back(index + 0);
            indexData.push_back(index + 1);
            indexData.push_back(index + 2);
            indexData.push_back(index + 2);
            indexData.push_back(index + 3);
            indexData.push_back(index + 0);
            index += 4;
        }
        mesh->VertexData(vertexData);
        mesh->IndexData(indexData);
       */ 
      /*
        std::vector<float> posData;
        std::vector<float> uvData;
        std::vector<unsigned short> indexData;
        float advX = 0.0f, advY = 0.0f;
        unsigned short index = 0;
        for(unsigned i = 0; i < text.size(); ++i)
        {
            char charCode = text[i];
            FontRasterizer::GlyphInfo* glyph = fontData->GetGlyph(charCode, 32);
            std::vector<float> newPosData = {
                advX, 0.0f, 0.0f, 
                advX + glyph->width, 0.0f, 0.0f,
                advX + glyph->width, glyph->height, 0.0f,
                advX, glyph->height, 0.0f
            };
            posData.insert(posData.end(), newPosData.begin(), newPosData.end());
            std::vector<float> newUvData = {
                0.0f, 0.0f, 
                1.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 1.0f,
            };
            uvData.insert(uvData.end(), newUvData.begin(), newUvData.end());
            
            advX += glyph->advX;
            advY += glyph->advY;
            indexData.push_back(index + 0);
            indexData.push_back(index + 1);
            indexData.push_back(index + 2);
            indexData.push_back(index + 2);
            indexData.push_back(index + 3);
            indexData.push_back(index + 0);
            index += 4;
        }
        vao.FillArrayBuffer("Position", (void*)posData.data(), posData.size() * sizeof(float));
        vao.FillArrayBuffer("UV", (void*)uvData.data(), uvData.size() * sizeof(float));
        vao.FillIndexBuffer(indexData);
        */
    }
    
    void Render(GLuint shaderProgram)
    {
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "MatrixModel"), 
            1, 
            GL_FALSE, 
            (float*)&GetComponent<Transform>()->GetTransform()
        );
        vao.Bind();
        vao.DrawElements(GL_TRIANGLES);
    }
    
    void OnInit()
    {
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        
        vao.Init({
            { "Position", 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, GL_STATIC_DRAW },
            { "UV", 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, GL_STATIC_DRAW }
        });
    }
    
protected:
    GLVertexArrayObject vao;
    Renderer* renderer;
    
    FontData* CreateDefaultFontData()
    {
        FontData* fd = new FontData();
        fd->LoadMemory((void*)nimbusmono_bold_otf, sizeof(nimbusmono_bold_otf));
        return fd;
    }

    std::string text;
    FontData* fontData;
};

#endif
