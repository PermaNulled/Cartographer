#include "stdafx.h"
#include "Achievements.h"

#include <string>
#include <curl/curl.h>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "Globals.h"
#include "H2MOD\Modules\Accounts\Accounts.h"

using namespace rapidjson;
using namespace std;
extern XUID xFakeXuid[4];
std::map<DWORD, bool> achievementList;


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void AchievementUnlock(int achievement_id)
{
	TRACE_GAME_N("[H2Mod-Achievement] - Unlocking achievement ID: %i", achievement_id);

	CURL *curl;
	CURLcode res;
	std::string readBuffer;


	curl = curl_easy_init();
	if (curl) {
		rapidjson::Document document;
		document.SetObject();


		Value token(kStringType);
		token.SetString(H2CurrentAccountLoginToken, document.GetAllocator());
		document.AddMember("token", token, document.GetAllocator());
		document.AddMember("id", achievement_id, document.GetAllocator());
		document.AddMember("xuid", Value().SetUint64(xFakeXuid[0]), document.GetAllocator());


		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		document.Accept(writer);

		curl_easy_setopt(curl, CURLOPT_URL, "http://cartographer.online/achievement-api/unlock.php");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer.GetString());
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

	}
}

void GetAchievements()
{
	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl) {

		std::string server_url;
		server_url.append("http://cartographer.online/achievement-api/achievement_list.php?xuid=");
		server_url.append(to_string(xFakeXuid[0]));

		curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		rapidjson::Document document;

		document.Parse(readBuffer.c_str());

		for (auto& achievement : document["achievements"].GetArray())
		{
			int id = (int)std::stoll(achievement.GetString());
			achievementList[id] = 1;
		}
	}

	// enable single player achievements
	if (!h2mod->Server)
		*(BYTE*)(h2mod->GetBase() + 0x518210 + 0x1B41)  = (BYTE)1;
}