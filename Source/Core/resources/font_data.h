#ifndef FONT_DATA_H
#define FONT_DATA_H

#include "../game_state.h"
#include <resource.h>

#include "texture2d.h"
#include "texture3d.h"

#include "../lib/font_rasterizer.h"

#define GLYPH_PAGE_SIZE 256

class FontData
{
public:
    struct Glyph
    {
        int width, height;
        int advX;
        int hBearingY;
        int hBearingX;
        float u0, v0, u1, v1;
        float layer;
    };
    struct GlyphAtlas
    {
        GlyphAtlas()
        : slot(0, 0, 0), pages(1) 
        {
            
        }

        FontRasterizer::GlobalMetrics metrics;
        gfxm::ivec3 slot;
        int lastRowHeight = 0;
        Texture3D* texture;
        unsigned char* data;
        int pages;
        std::map<unsigned, Glyph> glyphs;
        bool Exists(unsigned charCode)
        {
            return glyphs.count(charCode) != 0;
        }
        void AddPage()
        {
            pages++;
            delete[] data;
            data = new unsigned char[GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE * pages];
            memset(data, 0, GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE * pages);
            unsigned char* old = texture->GetData();
            memcpy(data, old, GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE * (pages - 1));
            texture->Data(data, GLYPH_PAGE_SIZE, GLYPH_PAGE_SIZE, pages, 1);
        }
    };
    FontData()
    {
        rasterizer.Init();
    }
    ~FontData()
    {
        rasterizer.Cleanup();
    }
    void Load(const std::string& filename)
    {
        rasterizer.ReadFile(filename);
    }
    void LoadMemory(void* data, size_t sz)
    {
        rasterizer.ReadMemory(data, sz);
    }
    void RebuildTexture()
    {
        FontRasterizer::Bitmap bmp;
        rasterizer.Rasterize(bmp);
        texture.Data((unsigned char *)bmp.data, bmp.width, bmp.height, bmp.bpp);
        bmp.Free();
    }
    void RebuildMesh()
    {

    }
    GlyphAtlas* GetGlyphAtlas(unsigned size) 
    {
        if(atlases.count(size) == 0)
        {
            atlases[size].texture = new Texture3D();
            atlases[size].data = new unsigned char[GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE];
            memset(atlases[size].data, 0, GLYPH_PAGE_SIZE * GLYPH_PAGE_SIZE);
            atlases[size].texture->Data(atlases[size].data, GLYPH_PAGE_SIZE, GLYPH_PAGE_SIZE, 1, 1);
        }
        GlyphAtlas& a = atlases[size];
        a.metrics = rasterizer.GetGlobalMetrics(size);
        return &atlases[size]; 
    }
    Texture2D* GetTexture() { return &texture; }
    Glyph* GetGlyph(unsigned charCode, unsigned size) 
    {
        GlyphAtlas* atlas = GetGlyphAtlas(size);
        FontRasterizer::GlyphInfo* g = 
            rasterizer.GetGlyph(charCode, size);
        if(!atlas->Exists(charCode))
        {
            if(GLYPH_PAGE_SIZE - atlas->slot.x < (int)g->bitmap.width)
            {
                atlas->slot.x = 0;
                atlas->slot.y += atlas->lastRowHeight;
                atlas->lastRowHeight = 0;
            }
            if(GLYPH_PAGE_SIZE - atlas->slot.y < (int)g->bitmap.height)
            {
                atlas->slot.x = 0;
                atlas->slot.y = 0;
                atlas->slot.z += 1;
                atlas->lastRowHeight = 0;
                atlas->AddPage();
            }

            atlas->texture->Blit2d(
                (unsigned char*)g->bitmap.data,
                g->bitmap.width, g->bitmap.height, 1,
                atlas->slot.x, 
                atlas->slot.y,
                atlas->slot.z
            );
            atlas->glyphs[charCode] = 
                Glyph{
                    (int)g->width, (int)g->height,
                    (int)g->advX,
                    (int)g->hBearingY,
                    (int)g->hBearingX,
                    (float)(atlas->slot.x) / (float)GLYPH_PAGE_SIZE, 
                    (float)(atlas->slot.y) / (float)GLYPH_PAGE_SIZE, 
                    (float)(atlas->slot.x) / (float)GLYPH_PAGE_SIZE + 
                    (float)g->bitmap.width / (float)GLYPH_PAGE_SIZE, 
                    (float)(atlas->slot.y) / (float)GLYPH_PAGE_SIZE + 
                    (float)g->bitmap.height / (float)GLYPH_PAGE_SIZE,
                    (float)atlas->slot.z
                };
            
            atlas->slot.x += (int)g->width;
            if(atlas->lastRowHeight < g->height)
                atlas->lastRowHeight = (int)g->height;
        }
        return &atlas->glyphs[charCode];
    }
private:
    std::map<unsigned, GlyphAtlas> atlases;
    Texture2D texture;
    FontRasterizer rasterizer;
};

template<>
inline bool LoadAsset<FontData, TTF>(FontData* fontData, const std::string& filename)
{
    fontData->Load(filename);
    return true;
}

#endif
