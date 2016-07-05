#pragma once
enum MessageIdentifier
{
	Start,
	PlatformData,
	Update,
	ClientMove,
	ClientShoot,
	CreateBullet,
	Hit,
	BulletUpdate,
	PlayerDamage,
	PlayerKill,
	PlayerRespawn,
};

enum BulletType
{
	UMP45,
	Shotgun
};