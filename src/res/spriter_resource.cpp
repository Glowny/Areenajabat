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
#include "spriterengine/override/spriterfileattributewrapper.h"
BX_PRAGMA_DIAGNOSTIC_POP_MSVC()

namespace arena
{
    using namespace SpriterEngine;

    class RapidXmlFileAttributeWrapper : public SpriterFileAttributeWrapper
    {
    public:
        RapidXmlFileAttributeWrapper(rapidxml::xml_attribute<char>* attrib)
            : m_attribute(attrib)
        {

        }

        std::string getName() override
        {
            return m_attribute->name();
        }

        bool isValid() override
        {
            return m_attribute != NULL;
        }

        real getRealValue() override
        {
            return atof(m_attribute->value());
        }

        int getIntValue() override
        {
            return atoi(m_attribute->value());
        }

        std::string getStringValue() override
        {
            return m_attribute->value();
        }

        void advanceToNextAttribute() override;
    private:
        rapidxml::xml_attribute<char>* m_attribute;
    };

    class RapidXmlFileElementWrapper : public SpriterFileElementWrapper
    {
    public:
        RapidXmlFileElementWrapper(rapidxml::xml_node<char>* node)
            : m_node(node)
        {
            
        }

        std::string getName() override
        {
            return m_node->name();
        }

        bool isValid() override
        {
            return m_node != 0;
        }

        void advanceToNextSiblingElement() override
        {
            m_node = m_node->next_sibling();
        }
        void advanceToNextSiblingElementOfSameName() override
        {
            m_node = m_node->next_sibling(m_node->name(), m_node->name_size());
        }

    private:

        SpriterFileAttributeWrapper *newAttributeWrapperFromFirstAttribute() override
        {
            return new RapidXmlFileAttributeWrapper(m_node->first_attribute());
        }
        SpriterFileAttributeWrapper *newAttributeWrapperFromFirstAttribute(const std::string & attributeName) override
        {
            return new RapidXmlFileAttributeWrapper(m_node->first_attribute(attributeName.c_str()));
        }

        SpriterFileElementWrapper *newElementWrapperFromFirstElement() override
        {
            return new RapidXmlFileElementWrapper(m_node->first_node());
        }
        SpriterFileElementWrapper *newElementWrapperFromFirstElement(const std::string & elementName) override
        {
            return new RapidXmlFileElementWrapper(m_node->first_node(elementName.c_str()));
        }

        SpriterFileElementWrapper *newElementWrapperFromNextSiblingElement() override
        {
            return new RapidXmlFileElementWrapper(m_node->next_sibling());
        }

        SpriterFileElementWrapper *newElementClone() override
        {
            return new RapidXmlFileElementWrapper(m_node);
        }

        rapidxml::xml_node<char>* m_node;
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
            return new RapidXmlFileElementWrapper(m_doc.first_node(elementName.c_str()));
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

