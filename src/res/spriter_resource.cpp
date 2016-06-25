#include "spriter_resource.h"
#include <bx/macros.h>
#include <rapidxml/rapidxml.hpp>

BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4263) // 'function' : member function does not override any base class virtual member function
#include <spriterengine/spriterengine.h>
#include "spriterengine/override/filefactory.h"
BX_PRAGMA_DIAGNOSTIC_POP_MSVC()

namespace arena
{
    using namespace SpriterEngine;

    class RapidxmlDocumentWrapper : public SpriterFileDocumentWrapper
    {
    public:
        void loadFile(std::string fileName) override;
    private:
        SpriterFileElementWrapper* newElementWrapperFromFirstElement() override
        {
            return nullptr;
        }
        SpriterFileElementWrapper* newElementWrapperFromFirstElement(const std::string& elementName) override
        {
            return nullptr;
        }
    };

    class SpriterFileFactory : public SpriterEngine::FileFactory
    {
    public:

        ImageFile* newImageFile(const std::string& initialFilePath, point initialDefaultPivot) override
        {

        }

        SoundFile* newSoundFile(const std::string&) override
        {
            // we dont need sounds
            return nullptr;
        }

        SpriterFileDocumentWrapper *newScmlDocumentWrapper() override
        {
            //rapidxml
            return nullptr;
        }
    private:
    };

    namespace spriter
    {
        void* load(const std::string name)
        {
            return NULL;
        }

        void unload(void*)
        {

        }
    }
}

