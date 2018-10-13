#pragma once
#include "xlivedefs.h"
#include <atomic>

typedef struct _XLOCATOR_SEARCHRESULT {
	ULONGLONG serverID;                     // the ID of dedicated server
	DWORD dwServerType;                     // see XLOCATOR_SERVERTYPE_PUBLIC, etc
	XNADDR serverAddress;                   // the address dedicated server
	XNKID xnkid;
	XNKEY xnkey;
	DWORD dwMaxPublicSlots;
	DWORD dwMaxPrivateSlots;
	DWORD dwFilledPublicSlots;
	DWORD dwFilledPrivateSlots;
	DWORD cProperties;                      // number of custom properties.
	PXUSER_PROPERTY pProperties;            // an array of custom properties.

} XLOCATOR_SEARCHRESULT, *PXLOCATOR_SEARCHRESULT;

class ServerList
{
	std::thread serv_thread;

public:
	_XLOCATOR_SEARCHRESULT *servers;

	std::atomic<bool> running = false;
	bool completed = false;
	int servers_left = 0;
	int total_servers = 0;

	int total_count = 0;
	int total_public = 0;
	int total_public_gold = 0;
	int total_peer = 0;
	int total_peer_gold = 0;

	bool GetRunning();
	void GetServers(PXOVERLAPPED,char*);
	int GetServersLeft();
	int GetTotalServers();
	void RemoveServer();
	bool GetServerCounts();


};

