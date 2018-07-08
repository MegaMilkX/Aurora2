#ifndef GFX_TEXT_H
#define GFX_TEXT_H

#include <texture2d.h>
#include <mesh.h>

#include <font_data.h>
#include "../lib/nimbusmono_bold.otf.h"
#include "../util/bitmap.h"

class GfxText
{
public:
    GfxText()
    {
        static FontData* fd = CreateDefaultFontData();
        fontData = fd;
        mesh = new Mesh();
    }
    ~GfxText()
    {
        delete mesh;
    }

    void SetText(const std::string& str)
    {
        SetText(std::vector<int>(str.begin(), str.end()));
    }
    void SetText(const std::vector<int>& str)
    {
        string = str;
    }
    void SetSize(unsigned sz)
    {
        size = sz;
    }
    void SetFont(const std::string& name)
    {
        fontData = asset<FontData>::get(name);
    }

    void Draw(int modelUniform, const gfxm::mat4 model, int glyphPageCountUniform)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, fontData->GetGlyphAtlas(size)->texture->GetGlName());
        glUniformMatrix4fv(
            modelUniform, 1, GL_FALSE,
            (float*)&model
        );
        glUniform1f(
            glyphPageCountUniform,
            (float)fontData->GetGlyphAtlas(size)->pages
        );

        glBindVertexArray(mesh->GetVao({
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UVW", 3, GL_FLOAT, GL_FALSE }
        }));
        glDrawArrays(GL_TRIANGLES, 0, charCount * 6);
    }

    void Rebuild()
    {
        std::vector<float> vertices;
        std::vector<float> uvw;
        float adv = 0.0f;
        charCount = string.size();
        FontData::GlyphAtlas* atlas = fontData->GetGlyphAtlas(size);
        float line = 1.0f;
        for(unsigned i = 0; i < string.size(); ++i)
        {
            int charCode = string[i];
            if(charCode == 13 || charCode == '\n')
            {
                line += 1.0f;
                adv = 0.0f;
                continue;
            }
            FontData::Glyph* g = fontData->GetGlyph(charCode, size);
            float heightBearingDiff = (float)(g->height - g->hBearingY);
            float loff = line * atlas->metrics.lineHeight;
            vertices.insert(
                vertices.end(),
                {
                    adv + g->hBearingX + 0.0f,      (float)-g->height + heightBearingDiff + loff,               0.0f,
                    adv + g->hBearingX + g->width,  (float)-g->height + heightBearingDiff + loff,               0.0f,
                    adv + g->hBearingX + g->width,  (float)heightBearingDiff + loff,   0.0f,
                    adv + g->hBearingX + g->width,  (float)heightBearingDiff + loff,   0.0f,
                    adv + g->hBearingX + 0.0f,      (float)heightBearingDiff + loff,   0.0f,
                    adv + g->hBearingX + 0.0f,      (float)-g->height + heightBearingDiff + loff,               0.0f
                }
            );
            uvw.insert(
                uvw.end(),
                {
                    g->u0, g->v0, g->layer,
                    g->u1, g->v0, g->layer,
                    g->u1, g->v1, g->layer,
                    g->u1, g->v1, g->layer,
                    g->u0, g->v1, g->layer,
                    g->u0, g->v0, g->layer
                }
            );
            adv += g->advX;
        }
        
        mesh->SetAttribArray<Au::Position>(vertices);
        mesh->SetAttribArray<Au::UVW>(uvw);
    }
private:
    Mesh* mesh;
    FontData* fontData;
    int charCount;
    std::vector<int> string;
    unsigned size;
    
    FontData* CreateDefaultFontData()
    {
        FontData* fd = new FontData();
        fd->LoadMemory((void*)nimbusmono_bold_otf, sizeof(nimbusmono_bold_otf));
        return fd;
    }
};

#endif
