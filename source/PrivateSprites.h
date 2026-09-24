#pragma once
#include "Utility.h"
#include "Image.h"
#include "Other.h"
#include <map>
#include <fstream>

// Textures belong to this store, never to the temporary drawing sprites.
class SkyPrivateSprites {
    std::map<std::string, RwTexture*> textures;
    struct Source { std::string directory; const unsigned char* data; size_t size; unsigned retries = 0; };
    std::map<std::string, Source> sources;
    std::map<std::string, bool> loggedDraw;

public:
    void Clear() {
        for (auto& item : textures) if (item.second) RwTextureDestroy(item.second);
        textures.clear();
        sources.clear();
        loggedDraw.clear();
    }
    void Load(const std::string& directory, const char* name,
              const unsigned char* fallback, size_t size) {
        if (!sources.count(name)) sources.emplace(name, Source{directory, fallback, size});
        const auto path = directory + "\\" + name + ".png";
        plugin::Image* img = nullptr;
        const bool external = plugin::CreateImageFromFile(path, img);
        const char* origin = external ? "external" : "embedded";
        if (!external)
            plugin::CreateImageFromMemory(name, const_cast<unsigned char*>(fallback),
                                          static_cast<unsigned long>(size), img);
        skyTextureUploadStatus = "image decode failed";
        RwTexture* texture = img ? CreateRwTexture(img->width, img->height, img->pixels) : nullptr;
        if (img) img->Release();
        // A corrupt override or GPU allocation failure must not become a null dereference.
        if (external && !texture) {
            img = nullptr;
            origin = "embedded after external texture failure";
            if (plugin::CreateImageFromMemory(name, const_cast<unsigned char*>(fallback),
                                             static_cast<unsigned long>(size), img)) {
                texture = CreateRwTexture(img->width, img->height, img->pixels);
                img->Release();
            }
        }
        auto& previous = textures[name];
        if (previous) RwTextureDestroy(previous);
        previous = texture;
        loggedDraw[name] = false;
        if (texture) RwTextureSetAddressing(texture, rwTEXTUREADDRESSCLAMP);
        std::ofstream log(PLUGIN_PATH("SkyUI-assets.log"), std::ios::app);
        log << path << ": " << origin << ", upload=" << skyTextureUploadStatus << ", texture=" << texture << '\n';
    }
    RwTexture* GetTex(const std::string& name) const {
        auto it = textures.find(name);
        return it == textures.end() ? nullptr : it->second;
    }
    void RetryMissing() {
        for (auto& [name, source] : sources) {
            if (!GetTex(name) && source.retries < 2)
                { ++source.retries; Load(source.directory, name.c_str(), source.data, source.size); }
        }
    }
    bool Draw(const std::string& name, const CRect& rect, const CRGBA& color) {
        auto* texture = GetTex(name);
        if (!loggedDraw[name]) {
            std::ofstream log(PLUGIN_PATH("SkyUI-assets.log"), std::ios::app);
            log << "draw " << name << " texture=" << texture
                << " raster=" << (texture ? texture->raster : nullptr) << '\n';
            loggedDraw[name] = true;
        }
        // Native CSprite2d::Draw renders an opaque quad when its texture is null.
        // Missing arrow overlays must never conceal a valid controller underneath.
        if (!texture || !texture->raster) return false;
        CSprite2d sprite;
        sprite.m_pTexture = texture;
        sprite.Draw(rect, color);
        return true;
    }
    bool Draw(const std::string& name, float x, float y, float w, float h, const CRGBA& color) {
        return Draw(name, CRect(x, y, x + w, y + h), color);
    }
};
