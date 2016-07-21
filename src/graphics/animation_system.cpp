#include "animation_system.h"
#include "composite_sprite.h"
#include "../res/texture_resource.h"
#include "../app.h"
#include "../res/resource_manager.h"
#include "../res/spriter_animation_player.h"

namespace arena
{
    struct GladiusRightArm : public IHandAnimation
    {
        GladiusRightArm() :
            m_upperAngle(70.f),
            m_forearmAngle(40.f),
            m_gunAngle(250.f),
            m_flipX(false),
            m_upperArm(nullptr),
            m_foreArm(nullptr),
            m_gun(nullptr)
        {

        }

        ~GladiusRightArm() override {}

        void create() override
        {
            ResourceManager* resources = App::instance().resources();
            m_upperArm.m_texture = resources->get<TextureResource>(ResourceType::Texture, "Characters/arms/1_UpperArm.png");
            m_foreArm.m_texture = resources->get<TextureResource>(ResourceType::Texture, "Characters/arms/1_Forearm.png");
            m_gun.m_texture = resources->get<TextureResource>(ResourceType::Texture, "Characters/guns/GladiusLeft.png");

            // setup children hierarchy
            m_upperArm.m_children.push_back(&m_foreArm);
            m_foreArm.m_children.push_back(&m_gun);

            // upper arm
            m_upperArm.m_position = glm::vec2(16, 10);
            m_upperArm.m_origin = glm::vec2(m_upperArm.m_texture->width / 2.f, 10.f);
            m_upperArm.m_rotation = glm::radians(m_upperAngle);
            m_upperArm.m_depth = 2.f;

            // fore arm
            m_foreArm.m_origin = glm::vec2(m_foreArm.m_texture->width / 2.f, 10.f);;
            m_foreArm.m_position = glm::vec2(5, 30);
            m_foreArm.m_rotation = glm::radians(m_forearmAngle);
            m_foreArm.m_depth = 2.f;

            // gun
            m_gun.m_depth = 1.9f;
            m_gun.m_origin = glm::vec2(m_gun.m_texture->width, m_gun.m_texture->height) / 2.f;
            m_gun.m_rotation = glm::radians(m_gunAngle);
            m_gun.m_position = glm::vec2(5.f, 18.f); // 10.f
        }

        void rotateTo(float radians) override
        {
            if (m_flipX)
            {
                m_upperArm.m_rotation = m_gunAngle + radians;
            }
            else
            {
                m_upperArm.m_rotation = m_gunAngle + radians + glm::radians(-25.f);
            }
        }

        void flip() override
        {
            m_flipX = !m_flipX;

            if (m_flipX)
            {
                m_upperArm.m_rotation = glm::radians(-m_upperAngle);
                m_foreArm.m_rotation = glm::radians(-m_forearmAngle);
                m_gun.m_rotation = glm::radians(-m_gunAngle);
                m_foreArm.m_position.x = 9.f;
            }
            else
            {
                m_upperArm.m_rotation = glm::radians(m_upperAngle);
                m_foreArm.m_rotation = glm::radians(m_forearmAngle);
                m_gun.m_rotation = glm::radians(m_gunAngle);
                m_foreArm.m_position.x = 5.f;
            }
        }

        CompositeSprite* getParent() override
        {
            return &m_upperArm;
        }

        // left angles
        float m_upperAngle;
        float m_forearmAngle;
        float m_gunAngle;

        CompositeSprite m_upperArm;
        CompositeSprite m_foreArm;
        CompositeSprite m_gun;

        bool m_flipX;
    };

    struct GladiusReloadAnimation
    {
        GladiusReloadAnimation() : 
            m_offset(12, 45),
            m_animation(nullptr)
        {

        }

        void update(float dt)
        {
            (void)dt;
        }

        void create()
        {

        }

        glm::vec2 m_offset;
        SpriterAnimationPlayer m_animation;
    };

#define DEFINE_ANIMATION(name)                                                                  \
    {                                                                                           \
        AnimationData* data = new AnimationData { new name##RightArm, nullptr, nullptr };       \
        data->m_rightHand->create();                                                            \
        return data;                                                                            \
    }

    AnimationData* getAnimationDataFor(WeaponAnimationType::Enum type)
    {
        switch (type)
        {
        case WeaponAnimationType::Gladius:
            DEFINE_ANIMATION(Gladius);
        default:
            return nullptr;
        }
    }

#undef DEFINE_ANIMATION
    void animationSystemInit()
    {

    }

    void animationSystemShutdown()
    {

    }
}