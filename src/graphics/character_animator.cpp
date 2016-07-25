#include "character_animator.h"
#include <bx/bx.h>
#include "../utils/math.h"
#include <algorithm>
#include <common/debug.h>

namespace arena
{

    struct AnimationPosition
    {
        enum Enum
        {
            Helmet, 
            RightArm,
            LeftArm,
            
            Count
        };
    };

    float calculateTorsoOffsetY(uint32_t milliseconds)
    {
        static const uint32_t length = 600;
        static const float s_directionTransitionTable[] =
        {
            0.f, // 0
            -1.f, // 50
            1.f, // 100
            2.f, // 150
            1.f, // 200
            0.5f, // 250
            0.f, // 300
            -1.f, // 350
            1.f, // 400
            2.f, // 450
            1.f, // 500
            0.5f // 550
        };

        uint32_t index = uint32_t(milliseconds / 50) % BX_COUNTOF(s_directionTransitionTable);
        uint32_t nextIndex = (index + 1) % BX_COUNTOF(s_directionTransitionTable);
        // hax :D
        float t = (milliseconds % 50) / 50.f;
        return lerp<float>(s_directionTransitionTable[index], s_directionTransitionTable[nextIndex], t);
    }

    void CharacterAnimator::setPosition(const glm::vec2& position)
    {
        ARENA_ASSERT(m_legs.m_animation.m_entity != nullptr, "Leg animation hasnt been set");
        m_position = position;
        m_legs.m_animation.setPosition(m_legs.m_relativeOffset + position);
		m_death.m_animation.setPosition(m_death.m_relativeOffset + position);
    }

    CharacterAnimator::CharacterAnimator() :
        m_weaponAnimType(WeaponAnimationType::Count),
        m_animationData(nullptr)
    {
		fillMap();
		aimAngle = 0;
		m_skin = Bronze;
        m_torso.m_relativeOffset = glm::vec2(-6.f, 37.f);
		// add death relative offset
		m_death.m_relativeOffset = glm::vec2(11, 124);
        m_legs.m_relativeOffset = glm::vec2(11, 124);
        m_head.m_helmet.m_position = glm::vec2(-16, -57);
		m_head.m_crest.m_position = glm::vec2(0,0);
        // build sprite hierarchy
        // assign crest to be child of helmet
        m_head.m_helmet.m_children.push_back(&m_head.m_crest);

        // torso will hold 2 arms and head 
        // TODO only one hand for now
        m_torso.m_sprite.m_children.resize(AnimationPosition::Count);
        // put helmet as first child of torso
        m_torso.m_sprite.m_children[AnimationPosition::Helmet] = &m_head.m_helmet;
        // this is the hand, but we dont have it. The flipX assumes there is allocated space 
        m_torso.m_sprite.m_children[AnimationPosition::RightArm] = nullptr;
        m_torso.m_sprite.m_children[AnimationPosition::LeftArm] = nullptr;
    }

    void CharacterAnimator::setStaticContent(TextureResource* crest, TextureResource* helmet, TextureResource* torso, SpriterEngine::EntityInstance* legs, SpriterEngine::EntityInstance* death)
    {
        m_head.m_crest.m_texture = crest;
        m_head.m_helmet.m_texture = helmet;

        m_torso.m_sprite.m_texture = torso;

        m_legs.m_animation.m_entity = legs;
		m_death.m_animation.m_entity = death;

    }

    void CharacterAnimator::rotateAimTo(float radians)
    {
		aimAngle = radians;
        ARENA_ASSERT(m_weaponAnimType != WeaponAnimationType::Count, "Animation type hasn't been set");
        m_animationData->m_rightHand->rotateTo(radians);
        m_animationData->m_leftHand->rotateTo(radians);

    }

    void CharacterAnimator::update(float dt)
    {
        double inMillis = dt * 1000.0;
		if (m_death.dying)
		{
			m_death.m_animation.setTimeElapsed(inMillis);
		}
		else
		{ 
			m_legs.m_animation.setTimeElapsed(inMillis);
			
			glm::vec2 offset(
			    0.f,
			    calculateTorsoOffsetY(uint32_t(m_legs.m_animation.getCurrentTime()))
			    );

			m_torso.m_sprite.m_position = m_position + m_torso.m_relativeOffset + offset;
		}
    }

    void CharacterAnimator::setWeaponAnimation(WeaponAnimationType::Enum type)
    {
        ARENA_ASSERT(type != WeaponAnimationType::Count, "Invalid type for weapon animation %d", type);

        if (m_weaponAnimType != type)
        {
            if (m_animationData != nullptr)
            {
                delete m_animationData;
            }

            m_animationData = getAnimationDataFor(type);
            m_weaponAnimType = type;

            auto& childrens = m_torso.m_sprite.m_children;
            // replace the hand
            // TODO other hands too
            childrens[AnimationPosition::RightArm] = m_animationData->m_rightHand->getParent();
            childrens[AnimationPosition::LeftArm] = m_animationData->m_leftHand->getParent();
        }
    }

    void CharacterAnimator::setFlipX(bool flip)
    {
        if (flip != m_flipX)
        {
            ARENA_ASSERT(m_weaponAnimType != WeaponAnimationType::Count, "Weapon animation type hasnt been set");

            m_flipX = flip;

            if (flip)
            {
                m_legs.m_animation.setCurrentAnimation("1_Right_Running");
                m_torso.m_relativeOffset.x = -8.f;
            }
            else
            {
                m_legs.m_animation.setCurrentAnimation("1_Left_Running");
                m_torso.m_relativeOffset.x = -4.f;
            }

            m_animationData->m_rightHand->flip();
            m_animationData->m_leftHand->flip();
        }
    }

	void CharacterAnimator::playDeathAnimation(bool hitDirection, float hitPositionY)
	{
		int bodyArea, gladiator, hitDirectionInt, upperBodyDirection, lowerBodyDirection;

		gladiator = m_skin; //either 0 (bronze) or 1 (gold), unless more skins are added
		hitDirectionInt = (int)hitDirection; // either 0 (left) or 1 (right)
		lowerBodyDirection = (int)m_flipX; // either 0 or 1, used for cases when upper body direction is different than lower body direction and legshot triggers the animation

		//0 = head, 1 = body, 2 = legs
		if (hitPositionY < 10)
			bodyArea = 0; 
		else if (hitPositionY < 60)
			bodyArea = 1;
		else
			bodyArea = 2;

		//pi/2 = 1.5707...
		if (aimAngle < 1.571 && aimAngle > -1.571) //if aiming right
			upperBodyDirection = 1;
		else //if aiming left
			upperBodyDirection = 0;

		m_death.dying = true;
		
		//there are currently 24 differenct dying animations combined for both characters
		/*int animation = hitDirection + (2 * upperBodyDirection) + (4 * bodyArea) + (12 * gladiator);
		m_death.m_animation.setCurrentAnimation(enumToFileName[(DyingAnimations)animation]);
		std::cout << std::to_string(animation) << std::endl;*/
		DyingAnimations dyingAnimation = DyingAnimations(hitDirection + (2 * upperBodyDirection) + (4 * bodyArea) + (12 * gladiator));
		m_death.m_animation.setCurrentAnimation(enumToFileName[dyingAnimation]);
		printf("animation enum: %d, string : %s\n", dyingAnimation, enumToFileName[dyingAnimation].c_str());
	}

    const glm::vec2& CharacterAnimator::getPosition() const
    {
        return m_position;
    }

    glm::vec2& CharacterAnimator::getPosition()
    {
        return m_position;
    }

	void CharacterAnimator::setCharacterSkin(CharacterSkin skin)
	{
		m_skin = skin;
	}

    void CharacterAnimator::render()
    {
        SpriteEffects::Enum effects = m_flipX ? SpriteEffects::FlipHorizontally : SpriteEffects::None;

        if (!m_death.dying)
        {

            m_legs.m_animation.render();
            // torso will render head and hands
            m_torso.m_sprite.render(effects);
        }
        else
        {
            m_death.m_animation.render();
        }
    }
	void CharacterAnimator::fillMap()
	{
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadLeft1, "1_toForehead_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadRight1, "1_toForehead_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadLeft1, "1_toBackOfTheHead_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadRight1, "1_toBackOfTheHead_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestLeft1, "1_toChest_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestRight1, "1_toChest_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLeft1, "1_toBack_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackRight1, "1_toBack_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsLeft1, "1_FrontLegs_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsRight1, "1_FrontLegs_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsLeft1, "1_BackLegs_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsRight1, "1_BackLegs_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadLeft2, "2_toForehead_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadRight2, "2_toForehead_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadLeft2, "2_toBackOfTheHead_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadRight2, "2_toBackOfTheHead_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestLeft2, "2_toChest_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestRight2, "2_toChest_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLeft2, "2_toBack_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackRight2, "2_toBack_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsLeft2, "2_FrontLegs_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsRight2, "2_FrontLegs_right"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsLeft2, "2_BackLegs_left"));
		enumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsRight2, "2_BackLegs_right"));
	}
}