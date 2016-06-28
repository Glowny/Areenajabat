#pragma once
enum MessageIdentifier
{
	Start,
	Update,
	ClientMove,
	ClientShoot

};

enum DataType
{
	messageID,
	unsignedInt,
	Int,
	Float,
	Char
};

enum BulletType
{
	UMP45,
	Shotgun
};