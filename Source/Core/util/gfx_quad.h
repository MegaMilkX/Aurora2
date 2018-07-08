#ifndef GFX_QUAD_H
#define GFX_QUAD_H

#include <asset.h>
#include <texture2d.h>

class GfxQuad
{
public:
    float width = 100, height = 100;
    gfxm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    asset<Texture2D> image;

    void Draw(
        int modelUniform, 
        int quadSizeUniform, 
        int colorUniform,
        const gfxm::mat4& model)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image->GetGlName());
        glUniformMatrix4fv(
            modelUniform, 1, GL_FALSE, (float*)&model
        );
        glUniform2f(
            quadSizeUniform, width, height
        );
        glUniform4f(
            colorUniform, color.r, color.g, color.b, color.a
        );
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif
