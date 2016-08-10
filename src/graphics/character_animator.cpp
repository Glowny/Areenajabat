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

	float CharacterAnimator::calculateTorsoRotation(float radians, bool direction)
	{
		if (direction) {
			if (radians < m_torsoMinAngle) {
				return radians;
			}
			else return m_torsoMinAngle;
		
			if (radians > m_torsoMaxAngle) {
				return radians;
			}
			else return m_torsoMaxAngle;
		} else
		{
			if (radians < m_torsoMaxAngle) {
			return radians;
		}
		else return m_torsoMaxAngle;

		if (radians > m_torsoMinAngle) {
			return radians;
		}
		else return m_torsoMinAngle;
	}
	}

    void CharacterAnimator::setPosition(const glm::vec2& position)
    {
        ARENA_ASSERT(m_legs.m_animation.m_entity != nullptr, "Leg animation hasnt been set");
        m_position = position;
        m_legs.m_animation.setPosition(m_legs.m_relativeOffset + position);
		m_death.m_animation.setPosition(m_death.m_relativeOffset + position);
		if (m_climb.m_climbing == 1) // Left
		{
			m_climb.m_animation.setPosition(m_climb.m_relativeOffsetLeft + position);
		}
		else if (m_climb.m_climbing == 2) // Right
		{
			m_climb.m_animation.setPosition(m_climb.m_relativeOffsetRight + position);
		}
    }

    CharacterAnimator::CharacterAnimator() :
        m_weaponAnimType(WeaponAnimationType::Count),
        m_animationData(nullptr)
    {
		m_torsoMaxAngle = 0.524f;
		m_torsoMinAngle = 5.760f;
		fillMap();
		m_aimAngle = 0;
		m_skin = Bronze;
		m_torso.m_sprite.m_origin = glm::vec2(18.f, 32.f);
        m_torso.m_relativeOffset = glm::vec2(-6.f + 18.f, 37.f + 32.f);
		// add death relative offset
		m_death.m_relativeOffset = glm::vec2(11, 124);
		m_gladiusReload.m_relativeOffset = glm::vec2(10, 45);
		m_axeReload.m_relativeOffset = glm::vec2(10, 45);
		m_throw.m_relativeOffset = glm::vec2(10, 45);
		m_climb.m_relativeOffsetLeft = glm::vec2(40, 124);
		m_climb.m_relativeOffsetRight = glm::vec2(-40, 124);
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

    void CharacterAnimator::setStaticContent(TextureResource* crest, TextureResource* helmet, TextureResource* torso,
		SpriterEngine::EntityInstance* legs, SpriterEngine::EntityInstance* death, SpriterEngine::EntityInstance* throwing,
		SpriterEngine::EntityInstance* gladiusReload, SpriterEngine::EntityInstance* axeReload, SpriterEngine::EntityInstance* climb)
	{
        m_head.m_crest.m_texture = crest;
        m_head.m_helmet.m_texture = helmet;

        m_torso.m_sprite.m_texture = torso;

        m_legs.m_animation.m_entity = legs;
		m_death.m_animation.m_entity = death;
		m_climb.m_animation.m_entity = climb;
		m_throw.m_animation.m_entity = throwing;
		m_gladiusReload.m_animation.m_entity = gladiusReload;
		m_axeReload.m_animation.m_entity = axeReload;
    }

    void CharacterAnimator::rotateAimTo(float radians)
    {
		m_aimAngle = radians;
		float weaponAim = m_aimAngle;

		//pi/2 = 1.5707...
		if (m_aimAngle < 1.571 && m_aimAngle > -1.571) //if aiming right
		{
			aimOffSetLeft = 0.25f;
			weaponAim += aimOffSetLeft;
			m_upperBodyDirection = 1;
			m_animationData->m_rightHand->setDirection(1);
			m_animationData->m_leftHand->setDirection(1);
		}
		else //if aiming left
		{ 
			aimOffSetRight = -0.25f;
			weaponAim += aimOffSetRight;
			m_upperBodyDirection = 0;
			m_animationData->m_rightHand->setDirection(0);
			m_animationData->m_leftHand->setDirection(0);
		}

        ARENA_ASSERT(m_weaponAnimType != WeaponAnimationType::Count, "Animation type hasn't been set");
		m_animationData->m_rightHand->rotateTo(weaponAim);
        m_animationData->m_leftHand->rotateTo(weaponAim);

		//calculate torso rotation
		m_torso.m_sprite.m_rotation = 0.0f;//calculateTorsoRotation(m_aimAngle, m_upperBodyDirection);

    }

    void CharacterAnimator::update(float dt)
    {
        double inMillis = dt * 1000.0;
		//calculate torso offset
		glm::vec2 offset(
			0.f,
			calculateTorsoOffsetY(uint32_t(m_legs.m_animation.getCurrentTime()))
		);
		

		if (m_death.dying)
		{
			m_death.m_animation.setTimeElapsed(inMillis);
		}
		else if (m_climb.m_climbing)
		{
			m_climb.m_animation.setTimeElapsed(inMillis/2);
			if (m_climb.m_animation.getCurrentTime() == 300.0f) // this is animation specific.
			{ 
				m_climb.m_animation.setCurrentTime(0.0f);
			}
		}
		
		else
		{ 
			m_legs.m_animation.setTimeElapsed(inMillis * m_legs.m_playSpeedMultiplier);
			//move torso according to leg movement
			m_torso.m_sprite.m_position = m_position + m_torso.m_relativeOffset + offset;
		}
		if (m_throw.m_throwing)
		{
			m_throw.m_animation.setTimeElapsed(inMillis);
			//move throwing arms according to leg movement
			m_throw.m_animation.setPosition(m_position + m_throw.m_relativeOffset + offset);
			if (m_throw.m_animation.isFinished())
			{
				auto& childrens = m_torso.m_sprite.m_children;
				childrens[AnimationPosition::RightArm]->m_hide = false;
				m_throw.m_throwing = false;
			}
		}
		else if (m_axeReload.m_reload)
		{
			m_axeReload.m_animation.setTimeElapsed(inMillis);
			//move reloading arms according to leg movement
			m_axeReload.m_animation.setPosition(m_position + m_axeReload.m_relativeOffset + offset);
			if (m_axeReload.m_animation.isFinished())
			{
				auto& childrens = m_torso.m_sprite.m_children;
				childrens[AnimationPosition::RightArm]->m_hide = false;
				m_axeReload.m_reload = false;
			}
		}
		else if (m_gladiusReload.m_reload)
		{
			m_gladiusReload.m_animation.setTimeElapsed(inMillis);
			//move reloading arms according to leg movement
			m_gladiusReload.m_animation.setPosition( m_position + m_gladiusReload.m_relativeOffset + offset);
			if (m_gladiusReload.m_animation.isFinished())
			{
				auto& childrens = m_torso.m_sprite.m_children;
				childrens[AnimationPosition::RightArm]->m_hide = false;
				m_gladiusReload.m_reload = false;
			}
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
     
        ARENA_ASSERT(m_weaponAnimType != WeaponAnimationType::Count, "Weapon animation type hasnt been set");

        m_flipX = flip;

        if (flip)
        {
            m_legs.m_animation.setCurrentAnimation("1_Right_Running");
            m_torso.m_relativeOffset.x = -8.f + 18.f;
        }
        else
        {
            m_legs.m_animation.setCurrentAnimation("1_Left_Running");
            m_torso.m_relativeOffset.x = -4.f + 17.f;
        }

    }
	void CharacterAnimator::stopRunningAnimation()
	{
		if (m_flipX)
		{ 
			m_legs.m_animation.setCurrentAnimation("1_Right_Standing");
			m_torso.m_relativeOffset.x = -5.f + 18.f;
		}
		else
		{ 
			m_legs.m_animation.setCurrentAnimation("1_Left_Standing");
			m_torso.m_relativeOffset.x = -4.f + 17.f;
		}
		m_legs.m_animation.pausePlayback();
		m_legs.running = false;
	}
	void CharacterAnimator::startRunningAnimation(float playSpeedMultiplier)
	{
		m_legs.m_playSpeedMultiplier = playSpeedMultiplier;
		if (m_legs.running == true)
			return;
		m_legs.m_animation.startResumePlayback();
		m_legs.running = true;
	}

	void CharacterAnimator::playDeathAnimation(bool hitDirection, float hitPositionY)
	{
		int bodyArea, gladiator, hitDirectionInt, upperBodyDirection, lowerBodyDirection;

		gladiator = m_skin; //either 0 (bronze) or 1 (gold), unless more skins are added
		hitDirectionInt = (int)hitDirection; // either 0 (left) or 1 (right)
		lowerBodyDirection = (int)m_flipX; // either 0 or 1, used for cases when upper body direction is different than lower body direction and legshot triggers the animation
		
		//0 = legs, 1 = body, 2 = head
		if (hitPositionY < 10)
			bodyArea = 2; 
		else if (hitPositionY < 60)
			bodyArea = 1;
		else
			bodyArea = 0;
		
		upperBodyDirection = (int)m_upperBodyDirection;

		//if the character is shot to legs, the direction of the character's upper body is the same as the direction of it's legs when dying
		if (bodyArea == 0 && upperBodyDirection != lowerBodyDirection)
			upperBodyDirection = abs(upperBodyDirection - 1); // so 1 = 0 and 0 = 1

		//there are currently 24 differenct dying animations combined for both characters
		//with the following calculation we can get 24 different animations with the information provided
		//hitdir + (2 * uppbodydir) + (4 * hitarea) + ( 12 * gladiator number )
		DyingAnimations dyingAnimation = DyingAnimations(hitDirection + (2 * upperBodyDirection) + (4 * bodyArea) + (12 * gladiator));
		m_death.m_animation.setCurrentAnimation(DyingEnumToFileName[dyingAnimation]);
		m_death.m_animation.setCurrentTime(0);
		//printf("animation enum: %d, string : %s\n", dyingAnimation, dyingEnumToFileName[dyingAnimation].c_str());

		//start updating the animation
		m_death.dying = true;
	}

	void CharacterAnimator::playClimbAnimation(int direction) {
		
		if (m_climb.m_climbing != 0)
			return;
		int gladiator;
		gladiator = m_skin;
	

		// Get the correct animation, climb direction can be 1 (left) or 2 (right).
		ClimbingAnimations animation = ClimbingAnimations(direction-1 + 2 * gladiator);
		m_climb.m_animation.setCurrentAnimation(ClimbingEnumToFileName[animation]);
		m_climb.m_animation.setCurrentTime(0);
		m_climb.m_climbing = direction;
	}

	void CharacterAnimator::endClimbAnimation()
	{
		m_climb.m_climbing = 0;
	}

	void CharacterAnimator::playReloadAnimation(int weapon) {

		int gladiator, direction;

		gladiator = m_skin;
		direction = (int)m_upperBodyDirection;
		//weapons: 0 = gladius (ump45), 1 = axe (shotgun)

		ReloadingAnimations animation = ReloadingAnimations(direction + 2 * gladiator);
		if (weapon == 0)
		{
			m_gladiusReload.m_animation.setCurrentAnimation(ReloadingEnumToFileName[animation]);
			m_gladiusReload.m_reload = true;
			m_axeReload.m_reload = false;
			m_gladiusReload.m_animation.setCurrentTime(0);
		}
		else
		{
			m_axeReload.m_animation.setCurrentAnimation(ReloadingEnumToFileName[animation]);
			m_axeReload.m_reload = true;
			m_gladiusReload.m_reload = false;
			m_axeReload.m_animation.setCurrentTime(0);
		}
		// TODO: The secret for hiding the other hand lies around here.
		auto& childrens = m_torso.m_sprite.m_children;
		childrens[AnimationPosition::RightArm]->m_hide = true;
		
		
	}

	void CharacterAnimator::playThrowAnimation(int weapon, int weaponSkin) {

		int gladiator, direction;

		gladiator = m_skin;
		direction = (int)m_upperBodyDirection;
		//26.7.2016 currently there is only 1 grenade
		//26.7.2016 currently weaponSkin is only used for Gladius (0 = with clip, 1 = without clip), so weapon 1 with weaponSkin 1 should not be used.
		ThrowingAnimations animation = (ThrowingAnimations)(direction + 2 * weaponSkin + 4 * weapon + 6 * gladiator);
		m_throw.m_animation.setCurrentAnimation(ThrowingEnumToFileName[animation]);
		m_throw.m_throwing = true;
		m_throw.m_animation.setCurrentTime(0);
		
		// TODO: fix other hand.
		auto& childrens = m_torso.m_sprite.m_children;
		childrens[AnimationPosition::RightArm]->m_hide = true;
	
	}

	void  CharacterAnimator::resetAnimation()
	{
		m_death.dying = false;
		m_climb.m_climbing = 0;
		m_gladiusReload.m_reload = false;
		m_axeReload.m_reload = false;
		m_throw.m_throwing = false;
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
		SpriteEffects::Enum effects = m_upperBodyDirection ? SpriteEffects::FlipHorizontally : SpriteEffects::None;

		// Render full body animation

		if (m_death.dying)
		{
			m_death.m_animation.render();
			return;
		}
		else if (m_climb.m_climbing != 0) 
		{
			m_climb.m_animation.render();
			return;
		}
		// If no full body animation, render legs.
        else
        {

            if (m_gladiusReload.m_reload)
            {
                m_gladiusReload.m_animation.render();
            }
            else if (m_axeReload.m_reload)
            {
                m_axeReload.m_animation.render();
            }
            else if (m_throw.m_throwing)
            {
                m_throw.m_animation.render();
            }
           
			
			m_torso.m_sprite.render(effects);


			// render legs
			m_legs.m_animation.render();
		}
		
	
	}
	void CharacterAnimator::fillMap()
	{
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadLeft1, "1_toForehead_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadRight1, "1_toForehead_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadLeft1, "1_toBackOfTheHead_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadRight1, "1_toBackOfTheHead_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestLeft1, "1_toChest_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestRight1, "1_toChest_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLeft1, "1_toBack_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackRight1, "1_toBack_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsLeft1, "1_toFrontLegs_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsRight1, "1_toFrontLegs_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsLeft1, "1_toBackLegs_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsRight1, "1_toBackLegs_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadLeft2, "2_toForehead_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToForeheadRight2, "2_toForehead_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadLeft2, "2_toBackOfTheHead_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackOfTheHeadRight2, "2_toBackOfTheHead_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestLeft2, "2_toChest_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToChestRight2, "2_toChest_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLeft2, "2_toBack_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackRight2, "2_toBack_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsLeft2, "2_toFrontLegs_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToFrontLegsRight2, "2_toFrontLegs_right"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsLeft2, "2_toBackLegs_left"));
		DyingEnumToFileName.insert(std::pair<DyingAnimations, std::string>(ToBackLegsRight2, "2_toBackLegs_right"));
		ClimbingEnumToFileName.insert(std::pair<ClimbingAnimations, std::string>(ClimbingLeft1, "1_Climbing_left"));
		ClimbingEnumToFileName.insert(std::pair<ClimbingAnimations, std::string>(ClimbingRight1, "1_Climbing_right"));
		ClimbingEnumToFileName.insert(std::pair<ClimbingAnimations, std::string>(ClimbingLeft2, "2_Climbing_left"));
		ClimbingEnumToFileName.insert(std::pair<ClimbingAnimations, std::string>(ClimbingRight2, "2_Climbing_right"));
		ReloadingEnumToFileName.insert(std::pair<ReloadingAnimations, std::string>(Left1, "1_Left"));
		ReloadingEnumToFileName.insert(std::pair<ReloadingAnimations, std::string>(Right1, "1_Right"));
		ReloadingEnumToFileName.insert(std::pair<ReloadingAnimations, std::string>(Left2, "2_Left"));
		ReloadingEnumToFileName.insert(std::pair<ReloadingAnimations, std::string>(Right2, "2_Right"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusLeft1, "1_Gladius_left"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusRight1, "1_Gladius_right"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusLeft2, "2_Gladius_left"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusRight2, "2_Gladius_right"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusNoClipLeft1, "1_Gladius_left_noclip"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusNoClipRight1, "1_Gladius_right_noclip"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusNoClipLeft2, "2_Gladius_left_noclip"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeGladiusNoClipRight2, "2_Gladius_right_noclip"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeAxeLeft1, "1_Axe_left"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeAxeRight1, "1_Axe_right"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeAxeLeft2, "2_Axe_left"));
		ThrowingEnumToFileName.insert(std::pair<ThrowingAnimations, std::string>(GrenadeAxeRight2, "2_Axe_right"));
	}
}