
#include "Players.h"

#include "H2MOD.h"

/*
	- TO NOTE:
	- This functions work only after the game has started (game life cycle is in_game or after map has been loaded).
	- If you need to do something in the pregame lobby, use the functions available in Network Session (H2MOD/Modules/Networking/NetworkSession)
*/

s_datum_array* Player::getArray()
{
	return *h2mod->GetAddress<s_datum_array**>(0x4A8260, 0x4D64C4);
}

bool Player::indexValid(int playerIndex)
{
	return playerIndex >= 0 && playerIndex < ENGINE_PLAYER_MAX;
}

Player* Player::getPlayer(int playerIndex)
{
	if (!indexValid(playerIndex))
	{
		return nullptr;
	}
	return (Player*)&getArray()->datum[playerIndex * getArray()->datum_element_size];
}

e_object_team Player::getTeam(int playerIndex)
{
	if (!indexValid(playerIndex))
	{
		return (e_object_team)NONE;
	}
	return getPlayer(playerIndex)->properties.player_team;
}

void Player::setTeam(int playerIndex, e_object_team team)
{
	if (!indexValid(playerIndex))
	{
		return;
	}
	getPlayer(playerIndex)->properties.player_team = team;
}

void Player::setUnitBipedType(int playerIndex, Player::Biped bipedType)
{
	if (!indexValid(playerIndex))
	{
		return;
	}
	getPlayer(playerIndex)->properties.profile.player_character_type = bipedType;
}

void Player::setBipedSpeed(int playerIndex, float speed)
{
	if (!indexValid(playerIndex))
	{
		return;
	}
	getPlayer(playerIndex)->unit_speed = speed;
}

wchar_t* Player::getName(int playerIndex)
{
	if (!indexValid(playerIndex))
	{
		return L"";
	}
	return getPlayer(playerIndex)->properties.player_name;
}

datum Player::getPlayerUnitDatumIndex(int playerIndex)
{
	if (!indexValid(playerIndex))
		return datum::Null;

	if (getPlayer(playerIndex)->BipedUnitDatum.IsNull())
		return datum::Null;
		
	return getPlayer(playerIndex)->BipedUnitDatum;
}

XUID Player::getIdentifier(int playerIndex)
{
	if (!indexValid(playerIndex))
	{
		return datum::Null;
	}

	return getPlayer(playerIndex)->xuid;
}

PlayerIterator::PlayerIterator() 
	: DatumIterator(Player::getArray())
{

}

bool PlayerIterator::get_next_player()
{
	m_current_player = get_next_datum();
	if (m_current_player)
	{
		do
		{
			if (!(m_current_player->Flags & 2))
				break;

			m_current_player = get_next_datum();

		} while (m_current_player);
	}

	return m_current_player != nullptr;
}

Player* PlayerIterator::get_current_player_data()
{
	return m_current_player;
}

int PlayerIterator::get_current_player_index()
{
	return get_current_absolute_index();
}

wchar_t* PlayerIterator::get_current_player_name()
{
	return m_current_player->properties.player_name;
}
