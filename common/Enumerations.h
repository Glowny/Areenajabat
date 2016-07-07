#pragma once
enum MessageIdentifier
{
	Restart,
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
	PlayerAmount,
};

enum BulletType
{
	UMP45,
	Shotgun
};