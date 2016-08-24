#pragma once

#include "../res/spriter_animation_player.h"
#include "composite_sprite.h"
#include "animation_system.h"
#include <map>
#include "../arena_types.h"

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
		ClimbingRight1,
		ClimbingLeft1,
		ClimbingRight2,
		ClimbingLeft2
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
		Legs() : m_animation(nullptr) { running = false; m_playSpeedMultiplier = 1.0f; }

        SpriterAnimationPlayer m_animation;
		bool running;
        glm::vec2 m_relativeOffset;
		float m_playSpeedMultiplier;
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
			m_climbing = 0;
			m_relativeOffsetLeft = glm::vec2(0, 0);
			m_relativeOffsetRight = glm::vec2(0, 0);
		}
		SpriterAnimationPlayer m_animation;
		glm::vec2 m_relativeOffsetLeft;
		glm::vec2 m_relativeOffsetRight;
		int m_climbing;
		// TODO: remove direction as speed can be used.
		int m_climbVerticalDirection = 1;
		float m_climbSpeed;
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
		glm::vec2 m_shoulderPoint;

		CharacterAnimator();

		float calculateTorsoRotation(float radians, bool direction);

		float calculateHeadRotation(float radians, bool direction);

		void update(float64 dt);

		void setPosition(const glm::vec2& position);

		glm::vec2& getPosition();

		const glm::vec2& getPosition() const;

		void setCharacterSkin(CharacterSkin skin);

		void setFlipX(bool flip);
		bool getFlipX() { return m_flipX; };
		bool getUpperBodyDirection() { return m_upperBodyDirection; };

        void setWeaponAnimation(WeaponAnimationType::Enum type);

		void playDeathAnimation(bool hitDirection, float hitPositionY);

		void playClimbAnimation(int direction);
		bool isClimbing();
		void pauseClimbAnimation();
		void continueClimbAnimation(int verticalDirection, float speed);
		void endClimbAnimation();

		void stopRunningAnimation();
		void startRunningAnimation(float playSpeedMultiplier);

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

		float m_torsoRotation = 0;
		float m_angleLimit1 = 0; //used for head and torso rotation
		float m_angleLimit2 = 0; //used for head and torso rotation
		float m_headMaxAngle = 0;
		float m_headMinAngle = 0;
        bool m_flipX;
		float m_aimAngle;
		void fillMap();
		bool m_upperBodyDirection;
	

		std::map<unsigned, std::string> ClimbingEnumToFileName;
		std::map<unsigned, std::string> DyingEnumToFileName;
		std::map<unsigned, std::string> ReloadingEnumToFileName;
		std::map<unsigned, std::string> ThrowingEnumToFileName;

    };
}
