#pragma once
#include <string>
#include <map>
#include <bx/readerwriter.h>
#include <bgfx/bgfx.h>

namespace arena
{
    struct ResourceType
    {
        enum Enum
        {
            Shader,
            Texture,
            Spriter,
            Count
        };
    };

    struct ResourcePair
    {
        ResourceType::Enum type;
        std::string name;

        bool operator<(const ResourcePair& a) const
        {
            return type < a.type || (type == a.type && name < a.name);
        }
    };

    struct ResourceEntry
    {
        uint32_t references;
        void* data;

        bool operator==(const ResourceEntry& e)
        {
            return references == e.references && data == e.data;
        }
    };

    typedef void* (*LoadFunction)(const std::string name);
    typedef void (*UnloadFunction)(void* data);

    struct TypeData
    {
        LoadFunction load;
        UnloadFunction unload;
    };

    class ResourceManager
    {
    public:
        using ResourceMap = std::map<ResourcePair, ResourceEntry>;
        using TypeMap = std::map<ResourceType::Enum, TypeData>;
        ResourceManager(const char* root);
        ~ResourceManager();

        void load(ResourceType::Enum type, const std::string& name);

        void* get(ResourceType::Enum type, const std::string& name);


        template <typename T>
        T* get(ResourceType::Enum type, const std::string& name) 
        {
            return (T*)get(type, name);
        }

    private:
        void registerLoader(ResourceType::Enum type, LoadFunction load, UnloadFunction unload);

        ResourceMap resources;
        TypeMap types;
        std::string root;
    };
}