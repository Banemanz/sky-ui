#pragma once
inline const char* skyTextureUploadStatus = "not attempted";
// RwImage is RGBA. Let RenderWare convert pixels and initialize the native
// raster extension (including the D3D8 alpha flag); locking alone does not.
static RwTexture* CreateRwTexture(int32_t w, int32_t h, uint8_t* p) {
    skyTextureUploadStatus = "invalid input";
    if (!p || w <= 0 || h <= 0 || w > 8192 || h > 8192) return nullptr;
    skyTextureUploadStatus = "image create failed";
    RwImage* image = RwImageCreate(w, h, 32);
    if (!image) return nullptr;
    skyTextureUploadStatus = "image allocation failed";
    if (!RwImageAllocatePixels(image)) { RwImageDestroy(image); return nullptr; }
    skyTextureUploadStatus = "invalid image stride/pixels";
    if (!image->cpPixels || image->stride < w * 4) { RwImageDestroy(image); return nullptr; }
    for (int y = 0; y < h; ++y)
        memcpy(image->cpPixels + static_cast<size_t>(y) * image->stride,
               p + static_cast<size_t>(y) * w * 4, static_cast<size_t>(w) * 4);
    skyTextureUploadStatus = "raster format negotiation failed";
    RwInt32 width, height, depth, format;
    if (!RwImageFindRasterFormat(image, rwRASTERTYPETEXTURE, &width, &height, &depth, &format)) {
        RwImageDestroy(image); return nullptr;
    }
    skyTextureUploadStatus = "raster allocation failed";
    RwRaster* raster = RwRasterCreate(width, height, depth, format);
    if (!raster) { RwImageDestroy(image); return nullptr; }
    skyTextureUploadStatus = "native upload failed";
    const bool uploaded = RwRasterSetFromImage(raster, image) != nullptr;
    RwImageDestroy(image);
    if (!uploaded) { RwRasterDestroy(raster); return nullptr; }
    skyTextureUploadStatus = "texture creation failed";
    RwTexture* texture = RwTextureCreate(raster);
    if (!texture) { RwRasterDestroy(raster); return nullptr; }
    RwTextureSetFilterMode(texture, rwFILTERLINEAR);
    skyTextureUploadStatus = "ok";
    return texture;
}
