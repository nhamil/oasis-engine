#include "Oasis/Graphics/Texture.h" 

#include "Oasis/Graphics/Texture2D.h" 
#include <iostream> 
namespace Oasis 
{

void Texture::FlushToGPU() 
{
    if (dirty_ || dirtyParams_) 
    {
        UploadToGPU(); 
    }

    dirty_ = false; 
    dirtyParams_ = false; 
}

Texture2D* Texture::GetTexture2D() 
{
    if (type_ == TextureType::TEXTURE_2D) 
    {
        return (Texture2D*) this; 
    }
    else 
    {
        return nullptr; 
    }
}

}