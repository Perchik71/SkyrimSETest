#include "../common.h"

typedef void *ISteamClient;
typedef void *ISteamUser;
typedef void *ISteamFriends;
typedef void *ISteamUserStats;
typedef void *ISteamRemoteStorage;

bool SteamAPI_Init()
{
	// Goal: Prevent Steam from spamming my friends with "X is now playing" when I restart
	// the game every 5 seconds
	return true;
}

ISteamClient *SteamClient()
{
	return nullptr;
}

ISteamUser *SteamUser()
{
	return nullptr;
}

ISteamFriends *SteamFriends()
{
	return nullptr;
}

ISteamUserStats *SteamUserStats()
{
	return nullptr;
}

ISteamRemoteStorage *SteamRemoteStorage()
{
	return nullptr;
}

void SteamAPI_RegisterCallResult(class CCallbackBase *pCallback, void *hAPICall)
{
}

void SteamAPI_UnregisterCallResult(class CCallbackBase *pCallback, void *hAPICall)
{
}

void SteamAPI_RegisterCallback(class CCallbackBase *pCallback, int iCallback)
{
}

void SteamAPI_UnregisterCallback(class CCallbackBase *pCallback)
{
}

void SteamAPI_RunCallbacks()
{
}

void PatchSteam()
{
	PatchIAT(SteamAPI_Init, "steam_api64.dll", "SteamAPI_Init");
	PatchIAT(SteamClient, "steam_api64.dll", "SteamClient");
	PatchIAT(SteamUser, "steam_api64.dll", "SteamUser");
	PatchIAT(SteamFriends, "steam_api64.dll", "SteamFriends");
	PatchIAT(SteamUserStats, "steam_api64.dll", "SteamUserStats");
	PatchIAT(SteamRemoteStorage, "steam_api64.dll", "SteamRemoteStorage");
	PatchIAT(SteamAPI_RegisterCallResult, "steam_api64.dll", "SteamAPI_RegisterCallResult");
	PatchIAT(SteamAPI_UnregisterCallResult, "steam_api64.dll", "SteamAPI_UnregisterCallResult");
	PatchIAT(SteamAPI_RegisterCallback, "steam_api64.dll", "SteamAPI_RegisterCallback");
	PatchIAT(SteamAPI_UnregisterCallback, "steam_api64.dll", "SteamAPI_UnregisterCallback");
	PatchIAT(SteamAPI_RunCallbacks, "steam_api64.dll", "SteamAPI_RunCallbacks");
}