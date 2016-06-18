#include "resource_manager.h"
#include "../utils/bgfx_utils.h"
#include "../io/io.h"

namespace arena
{
    static ResourceEntry s_invalid = { 0xFFFFFFFF, NULL };

    void ResourceManager::registerLoader(ResourceType::Enum type, LoadFunction load, UnloadFunction unload)
    {
        types[type] = { load, unload };
    }

    void* loadShader(const std::string path)
    {
        size_t index = path.rfind('/');
        std::string root(path.substr(0, index));
        std::string name(path.substr(index + 1) + std::string(".bin"));
        
        arena::FileReader reader;
        std::string vsh(root + std::string("/vs_") + name);
        std::string fsh(root + std::string("/fs_") + name);
        bgfx::ProgramHandle handle = arena::utils::loadProgram(&reader, vsh.c_str(), fsh.c_str());

        if (handle.idx == bgfx::invalidHandle)
        {
            return nullptr;
        }
        ProgramResource* r = new ProgramResource;
        r->handle = handle;

        return r;
    }

    void* loadTexture(const std::string path)
    {
        FileReader reader;
        bgfx::TextureInfo info;
        bgfx::TextureHandle handle = utils::loadTexture(&reader, path.c_str(), BGFX_TEXTURE_U_CLAMP| BGFX_TEXTURE_V_CLAMP, &info);
        if (handle.idx == bgfx::invalidHandle)
        {
            return nullptr;
        }
        TextureResource* r = new TextureResource;
        r->handle = handle;
        r->width = info.width;
        r->height = info.height;
        return r;
    }

    ResourceManager::ResourceManager(const char* root)
        : root(root)
    {
        registerLoader(ResourceType::Shader, loadShader, NULL);
        registerLoader(ResourceType::Texture, loadTexture, NULL);
    }

    ResourceManager::~ResourceManager()
    {

    }

    void ResourceManager::load(ResourceType::Enum type, const std::string& name)
    {
        ResourcePair id = { type, name };
        ResourceEntry& entry = resources.count(id) == 1 ? resources[id] : s_invalid;

        if (entry == s_invalid)
        {
            std::string path;

            if (type == ResourceType::Shader)
            {
                const char* shaderPath =
                    bgfx::getRendererType() == bgfx::RendererType::Direct3D11 ?
                    "shaders/dx11/" : "shaders/gl/";
                path = root + std::string(shaderPath) + name;
            } 
            else
            {
                path = root + name;
            }

            entry.references = 1;
            entry.data = types[type].load(path);
            
            resources[id] = entry;
            return;
        }

        ++entry.references;
    }

    void* ResourceManager::get(ResourceType::Enum type, const std::string& name)
    {
        ResourcePair id = { type, name };
        if (resources.count(id) == 0)
        {
            load(type, name);
        }

        ResourceEntry& entry = resources[id];
        return entry.data;
    }
}