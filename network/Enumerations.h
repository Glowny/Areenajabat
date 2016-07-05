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
};

enum BulletType
{
	UMP45,
	Shotgun
};