#pragma once

#include "../res/spriter_animation_player.h"
#include "composite_sprite.h"
#include "animation_system.h"
#include <map>

namespace arena
{

	enum CharacterSkin
	{
		Bronze,
		Gold,

	};

	enum DyingAnimations
	{
		ToFrontLegsLeft1,
		ToBackLegsLeft1,
		ToBackLegsRight1,
		ToFrontLegsRight1,		
		ToChestLeft1,
		ToBackLeft1,
		ToBackRight1,
		ToChestRight1,		
		ToForeheadLeft1,
		ToBackOfTheHeadLeft1,
		ToBackOfTheHeadRight1,
		ToForeheadRight1,
		ToFrontLegsLeft2,
		ToBackLegsLeft2,
		ToBackLegsRight2,
		ToFrontLegsRight2,
		ToChestLeft2,
		ToBackLeft2,
		ToBackRight2,
		ToChestRight2,
		ToForeheadLeft2,
		ToBackOfTheHeadLeft2,
		ToBackOfTheHeadRight2,
		ToForeheadRight2,		
	};

	enum ClimbingAnimations
	{
		ClimbingLeft1,
		ClimbingRight1,
		ClimbingLeft2,
		ClimbingRight2
	};

	enum ReloadingAnimations
	{
		Left1,
		Right1,
		Left2,
		Right2,
	};
	
	enum ThrowingAnimations
	{
		GrenadeGladiusLeft1,
		GrenadeGladiusRight1,
		GrenadeGladiusNoClipLeft1,
		GrenadeGladiusNoClipRight1,
		GrenadeAxeLeft1,
		GrenadeAxeRight1,
		GrenadeGladiusLeft2,
		GrenadeGladiusRight2,
		GrenadeGladiusNoClipLeft2,
		GrenadeGladiusNoClipRight2,
		GrenadeAxeLeft2,
		GrenadeAxeRight2
	};

    struct Head
    {
        Head() :
            m_helmet(nullptr),
            m_crest(nullptr)
        {
            
        }

        CompositeSprite m_helmet;
        CompositeSprite m_crest;
    };

    struct Legs
    {
        Legs() : m_animation(nullptr) {}

        SpriterAnimationPlayer m_animation;
        glm::vec2 m_relativeOffset;
    };

	struct Death
	{
		Death() : m_animation(nullptr) 
		{
			dying = false;
			m_relativeOffset = glm::vec2(0, 0);
		}

		SpriterAnimationPlayer m_animation;
		glm::vec2 m_relativeOffset;
		bool dying;
	};

	struct Climb
	{
		Climb() : m_animation(nullptr)
		{
			m_climbing = false;
			m_relativeOffset = glm::vec2(0, 0);
		}
		SpriterAnimationPlayer m_animation;
		glm::vec2 m_relativeOffset;
		bool m_climbing;
	};

	struct Reload
	{
		Reload() : m_animation(nullptr)
		{
			m_reload = false;
			m_relativeOffset = glm::vec2(0, 0);
		}
		SpriterAnimationPlayer m_animation;
		glm::vec2 m_relativeOffset;
		bool m_reload;
	};

	struct Throw
	{
		Throw() : m_animation(nullptr)
		{
			m_throwing = false;
			m_relativeOffset = glm::vec2(0, 0);
		}
		SpriterAnimationPlayer m_animation;
		glm::vec2 m_relativeOffset;
		bool m_throwing;
	
	};

    struct Torso
    {
        Torso() : m_sprite(nullptr) {}

        CompositeSprite m_sprite;
        glm::vec2 m_relativeOffset;
    };

	
    class CharacterAnimator
    {
    public:
        CharacterAnimator();
        
        void update(float dt);

        void setPosition(const glm::vec2& position);

        glm::vec2& getPosition();

        const glm::vec2& getPosition() const;

		void setCharacterSkin(CharacterSkin skin);

        void setFlipX(bool flip);


        void setWeaponAnimation(WeaponAnimationType::Enum type);

		void playDeathAnimation(bool hitDirection, float hitPositionY);

		void playClimbAnimation(bool direction);

		void playReloadAnimation(int weapon);

		void playThrowAnimation(int weapon, int weaponSkin);

		void resetAnimation();

        void setStaticContent(TextureResource* crest, TextureResource* helmet, TextureResource* torso,
			SpriterEngine::EntityInstance* legs, SpriterEngine::EntityInstance* death, SpriterEngine::EntityInstance* throwing,
			SpriterEngine::EntityInstance* gladiusReload, SpriterEngine::EntityInstance* axeReload, SpriterEngine::EntityInstance* climb);

        void render();

        void rotateAimTo(float radians);
    private:
		CharacterSkin m_skin;
        Head m_head;
        Torso m_torso;
        Legs m_legs;
		Death m_death;
		Climb m_climb;
		Reload m_gladiusReload;
		Reload m_axeReload;
		Throw m_throw;
        AnimationData* m_animationData;

        WeaponAnimationType::Enum m_weaponAnimType;

        glm::vec2 m_position;

        bool m_flipX;
		float aimAngle;
		void fillMap();
		bool m_upperBodyDirection;

		std::map<unsigned, std::string> ClimbingEnumToFileName;
		std::map<unsigned, std::string> DyingEnumToFileName;
		std::map<unsigned, std::string> ReloadingEnumToFileName;
		std::map<unsigned, std::string> ThrowingEnumToFileName;

    };
}
