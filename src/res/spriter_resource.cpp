#include "spriter_resource.h"
#include <bx/macros.h>
#include <rapidxml/rapidxml.hpp>
#include "../io/io.h"
#include <bx/readerwriter.h>
#include <vector>

BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4263) // 'function' : member function does not override any base class virtual member function
#include <spriterengine/spriterengine.h>
#include "spriterengine/override/filefactory.h"
#include "spriterengine/override/spriterfileelementwrapper.h"
BX_PRAGMA_DIAGNOSTIC_POP_MSVC()

namespace arena
{
    using namespace SpriterEngine;

    class RapidXmlFileElementWrapper : public SpriterFileElementWrapper
    {
    public:
        RapidXmlFileElementWrapper(rapidxml::xml_node<char>* node)
        {

        }

        std::string getName() override
        {
            return std::string();
        }

        bool isValid() override
        {
            return false;
        }

        void advanceToNextSiblingElement() override
        {

        }
        void advanceToNextSiblingElementOfSameName() override
        {

        }

    private:

        SpriterFileAttributeWrapper *newAttributeWrapperFromFirstAttribute() override
        {
            return NULL;
        }
        SpriterFileAttributeWrapper *newAttributeWrapperFromFirstAttribute(const std::string & attributeName) override
        {
            return NULL;
        }

        SpriterFileElementWrapper *newElementWrapperFromFirstElement() override
        {
            return NULL;
        }
        SpriterFileElementWrapper *newElementWrapperFromFirstElement(const std::string & elementName) override
        {
            return NULL;
        }

        SpriterFileElementWrapper *newElementWrapperFromNextSiblingElement() override
        {
            return NULL;
        }

        SpriterFileElementWrapper *newElementClone() override
        {
            return NULL;
        }

    };

    class RapidxmlDocumentWrapper : public SpriterFileDocumentWrapper
    {
    public:
        void loadFile(std::string fileName) override
        {
            FileReader reader;
            if (!bx::open(&reader, fileName.c_str())) return;

            
            int64_t size = bx::getSize(&reader);
            m_buffer.resize((uint32_t)size);
            bx::read(&reader, m_buffer.data(), (int32_t)size);
            m_doc.parse<0>(m_buffer.data());
            
        }
    private:
        SpriterFileElementWrapper* newElementWrapperFromFirstElement() override
        {
            return new RapidXmlFileElementWrapper(m_doc.first_node());
        }
        SpriterFileElementWrapper* newElementWrapperFromFirstElement(const std::string& elementName) override
        {
            return nullptr;
        }

        rapidxml::xml_document<> m_doc;
        std::vector<char> m_buffer;
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

