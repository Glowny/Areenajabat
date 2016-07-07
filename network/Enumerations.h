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
	ScoreBoardUpdate,
};

enum BulletType
{
	UMP45,
	Shotgun
};