#include "spriter_resource.h"
#include <bx/macros.h>
#include <rapidxml/rapidxml.hpp>
#include "../io/io.h"
#include <bx/readerwriter.h>
#include <vector>
#include "../app.h"
#include "resource_manager.h"
#include "../utils/color.h"
#include "../render.h"
#include "spriter_animation_player.h"

BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4263) // 'function' : member function does not override any base class virtual member function
#include <spriterengine/spriterengine.h>
#include "spriterengine/override/filefactory.h"
#include "spriterengine/override/spriterfileelementwrapper.h"
#include "spriterengine/override/spriterfileattributewrapper.h"
#include "spriterengine/override/imagefile.h"
#include "texture_resource.h"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "spriterengine/override/objectfactory.h"
#include "spriterengine/global/settings.h"
#include "spriterengine/objectinfo/boneinstanceinfo.h"
#include "spriterengine/objectinfo/boxinstanceinfo.h"
BX_PRAGMA_DIAGNOSTIC_POP_MSVC()
void rapidxml::parse_error_handler(const char * /*what*/, void * /*where*/) {
}
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

        void advanceToNextAttribute() override
        {
            m_attribute = m_attribute->next_attribute();
        }
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
            return m_node != NULL;
        }

        void advanceToNextSiblingElement() override
        {
            m_node = m_node->next_sibling();
        }
        void advanceToNextSiblingElementOfSameName() override
        {
            m_node = m_node->next_sibling(m_node->name());
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

        // same name must
        SpriterFileElementWrapper *newElementWrapperFromNextSiblingElement() override 
        {
            return new RapidXmlFileElementWrapper(m_node->next_sibling(m_node->name()));
        }

        // same name must
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
            m_buffer.resize((uint32_t)size + 1); // null terminator
            bx::read(&reader, m_buffer.data(), (int32_t)size);
            m_buffer[uint32_t(size)] = '\0';
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

    class SpriterImageFile : public ImageFile
    {
    public:
        SpriterImageFile(std::string initialFilePath, point initialDefaultPivot)
            : ImageFile(initialFilePath, initialDefaultPivot),
            m_texture(nullptr)
        {
            m_texture = App::instance().resources()->get<TextureResource>(ResourceType::Texture, initialFilePath);
        }

        void renderSprite(UniversalObjectInterface * spriteInfo) override
        {
            uint32_t color = color::toABGR(255, 255, 255, uint8_t(255 * spriteInfo->getAlpha()));
            glm::vec2 position(spriteInfo->getPosition().x, spriteInfo->getPosition().y);
            float rotation = float(spriteInfo->getAngle()); // in radians
            glm::vec2 scale(spriteInfo->getScale().x, spriteInfo->getScale().y);
            point pivot = spriteInfo->getPivot();
            glm::vec2 origin(pivot.x * m_texture->width, pivot.y * m_texture->height);
            arena::draw(m_texture, nullptr, color, position - origin, origin, scale, SpriteEffects::None, rotation, 1.f);
        }

    private:
        TextureResource* m_texture;
    };

    class SpriterFileFactory : public SpriterEngine::FileFactory
    {
    public:

        ImageFile* newImageFile(const std::string& initialFilePath, point initialDefaultPivot) override
        {
            return new SpriterImageFile(initialFilePath, initialDefaultPivot);
        }

        SoundFile* newSoundFile(const std::string&) override
        {
            // we dont need sounds
            return nullptr;
        }

        SpriterFileDocumentWrapper *newScmlDocumentWrapper() override
        {
            return new RapidxmlDocumentWrapper();
        }
    private:
    };

    // dummy, just so this fucking spriter works....
    class SpriterBoneInstanceInfo : public BoneInstanceInfo
    {
    public:
        SpriterBoneInstanceInfo(point initialSize):BoneInstanceInfo(initialSize){}
        void render() override {}
    };

    class SpriterObjectFactory : public ObjectFactory
    {
    public:
        SpriterObjectFactory()
        {

        }

        PointInstanceInfo *newPointInstanceInfo() override
        {
            return nullptr;
        }

        BoxInstanceInfo *newBoxInstanceInfo(point size) override
        {
            return new BoxInstanceInfo(size);
        }

        BoneInstanceInfo *newBoneInstanceInfo(point size) override
        {
            return new SpriterBoneInstanceInfo(size);
        }
    };

    SpriterResource::SpriterResource(const std::string& name)
        : m_model(name, new SpriterFileFactory, /*new SpriterObjectFactory*/nullptr)
    {

    }

    SpriterEngine::EntityInstance* SpriterResource::getNewEntityInstance(int entityId)
    {
        return m_model.getNewEntityInstance(entityId);
    }


    SpriterEngine::EntityInstance* SpriterResource::getNewEntityInstance(std::string entityName)
    {
        return m_model.getNewEntityInstance(entityName);
    }

    SpriterAnimationPlayer::SpriterAnimationPlayer(SpriterEngine::EntityInstance* instance)
        : m_entity(instance)
    {

    }

    void SpriterAnimationPlayer::setPosition(const glm::vec2& position)
    {
        m_entity->setPosition(SpriterEngine::point(position.x, position.y));
    }


    void SpriterAnimationPlayer::setAngle(float radians)
    {
        m_entity->setAngle(radians);
    }

    void SpriterAnimationPlayer::setScale(const glm::vec2& scale)
    {
        m_entity->setScale(SpriterEngine::point(scale.x, scale.y));
    }

    void SpriterAnimationPlayer::setAlpha(float alpha)
    {
        m_entity->setAlpha(alpha);
    }

    glm::vec2 SpriterAnimationPlayer::getPosition() const
    {
        auto pos = m_entity->getPosition();
        return glm::vec2(pos.x, pos.y);
    }

    glm::vec2 SpriterAnimationPlayer::getScale() const
    {
        auto s = m_entity->getScale();
        return glm::vec2(s.x, s.y);
    }

    float SpriterAnimationPlayer::getAngle() const
    {
        return float(m_entity->getAngle());
    }

    float SpriterAnimationPlayer::getAlpha() const
    {
        return float(m_entity->getAlpha());
    }

	void SpriterAnimationPlayer::startResumePlayback()
	{
		m_entity->startResumePlayback();
	}
	void SpriterAnimationPlayer::pausePlayback()
	{
		m_entity->pausePlayback();
	}

    void SpriterAnimationPlayer::setTimeElapsed(double milliseconds)
    {
        m_entity->setTimeElapsed(milliseconds);
    }


    void SpriterAnimationPlayer::render()
    {
        m_entity->render();
    }

    void SpriterAnimationPlayer::setCurrentAnimation(const std::string& anim)
    {
        m_entity->setCurrentAnimation(anim);
    }

    std::string SpriterAnimationPlayer::getCurrentAnimationName()
    {
        return m_entity->currentAnimationName();
    }

    void SpriterAnimationPlayer::setCurrentAnimation(int anim)
    {
        m_entity->setCurrentAnimation(anim);
    }


	void SpriterAnimationPlayer::setCurrentTime(double milliseconds)
	{
		m_entity->setCurrentTime(milliseconds);
		m_entity->startResumePlayback();
	}
    double SpriterAnimationPlayer::getCurrentTime() const
    {
        return m_entity->getCurrentTime();
    }

    bool SpriterAnimationPlayer::isFinished() const
    {
        return m_entity->animationJustFinished();
    }

    namespace spriter
    {
        void* load(const std::string name)
        {
            SpriterEngine::Settings::setErrorFunction(SpriterEngine::Settings::nullError);
            //Settings::reversePivotYOnLoad = false;
            //Settings::reverseYOnLoad = false;
            //Settings::reverseAngleOnLoad = false;
            SpriterResource* resource = new SpriterResource(name);
            return resource;
        }

        void unload(void*)
        {

        }
    }
}

