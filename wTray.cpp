//---------------------------------------------------------------------------
// Copyright (C) 2014 Krzysztof Grochocki
//
// This file is part of wTray
//
// wTray is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// wTray is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#pragma argsused
#include <PluginAPI.h>
#include <IdHashMessageDigest.hpp>

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  return 1;
}
//---------------------------------------------------------------------------

//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//Sciezka-do-katalogu-prywatnego-wtyczek-------------------------------------
UnicodeString PluginUserDir;
//Gdy-zostalo-uruchomione-wyladowanie-wtyczki--------------------------------
bool UnloadExecuted = false;
//Gdy-zostalo-uruchomione-wyladowanie-wtyczki-wraz-z-zamknieciem-komunikatora
bool ForceUnloadExecuted = false;
//IKONY-W-INTERFEJSIE--------------------------------------------------------
int ONLINE;
int OFFLINE;
int AWAY;
int XA;
int DND;
int INVISIBLE;
int FREEFORCHAT;
int NEW_MESSAGE;
int SECURE;
//FORWARD-AQQ-HOOKS----------------------------------------------------------
INT_PTR __stdcall OnBeforeUnload(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnListReady(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnTrayIconChange(WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu prywatnego wtyczek
UnicodeString GetPluginUserDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Hook na wylaczenie komunikatora poprzez usera
INT_PTR __stdcall OnBeforeUnload(WPARAM wParam, LPARAM lParam)
{
  //Info o rozpoczeciu procedury zamykania komunikatora
  ForceUnloadExecuted = true;

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zakonczenie ladowania listy kontaktow przy starcie AQQ
INT_PTR __stdcall OnListReady(WPARAM wParam, LPARAM lParam)
{
  //Odswiezenie ikonki w zasobniku systemowym
  PluginLink.CallService(AQQ_SYSTEM_TRAYICONREFRESH,0,0);

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane ikonki w zasobniku systemowym
INT_PTR __stdcall OnTrayIconChange(WPARAM wParam, LPARAM lParam)
{
  //Pobranie indeksu aktualnej ikonki w tray
  int TrayIcon = (int)wParam;
  //Nie zostala wywolana proceduta wyladowania wtyczki
  if(!UnloadExecuted)
  {
	//Online
	if(TrayIcon==1) return ONLINE;
	//Offline
	else if(TrayIcon==2) return OFFLINE;
	//Away
	else if(TrayIcon==3) return AWAY;
	//XA
	else if(TrayIcon==4) return XA;
	//DND
	else if(TrayIcon==5) return DND;
	//Invisible
	else if(TrayIcon==6) return INVISIBLE;
	//FreeForChat
	else if(TrayIcon==7) return FREEFORCHAT;
	//Message
	else if(TrayIcon==8) return NEW_MESSAGE;
	//Secure
	else if(TrayIcon==73) return SECURE;
  }
  //Zostala wywolana procedura wyladowania wtyczki bez wylaczenia komunikatora
  else if(!ForceUnloadExecuted)
  {
    //Online
	if(TrayIcon==ONLINE) return 1;
	//Offline
	else if(TrayIcon==OFFLINE) return 2;
	//Away
	else if(TrayIcon==AWAY) return 3;
	//XA
	else if(TrayIcon==XA) return 4;
	//DND
	else if(TrayIcon==DND) return 5;
	//Invisible
	else if(TrayIcon==INVISIBLE) return 6;
	//FreeForChat
	else if(TrayIcon==FREEFORCHAT) return 7;
	//Message
	else if(TrayIcon==NEW_MESSAGE) return 8;
	//Secure
	else if(TrayIcon==SECURE) return 73;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Zapisywanie zasobów
void ExtractRes(wchar_t* FileName, wchar_t* ResName, wchar_t* ResType)
{
  TPluginTwoFlagParams PluginTwoFlagParams;
  PluginTwoFlagParams.cbSize = sizeof(TPluginTwoFlagParams);
  PluginTwoFlagParams.Param1 = ResName;
  PluginTwoFlagParams.Param2 = ResType;
  PluginTwoFlagParams.Flag1 = (int)HInstance;
  PluginLink.CallService(AQQ_FUNCTION_SAVERESOURCE,(WPARAM)&PluginTwoFlagParams,(LPARAM)FileName);
}
//---------------------------------------------------------------------------

//Obliczanie sumy kontrolnej pliku
UnicodeString MD5File(UnicodeString FileName)
{
  if(FileExists(FileName))
  {
	UnicodeString Result;
    TFileStream *fs;

	fs = new TFileStream(FileName, fmOpenRead | fmShareDenyWrite);
	try
	{
	  TIdHashMessageDigest5 *idmd5= new TIdHashMessageDigest5();
	  try
	  {
	    Result = idmd5->HashStreamAsHex(fs);
	  }
	  __finally
	  {
	    delete idmd5;
	  }
    }
	__finally
    {
	  delete fs;
    }

    return Result;
  }
  else
   return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
  //Linkowanie wtyczki z komunikatorem
  PluginLink = *Link;
  //Pobranie sciezki do katalogu prywatnego uzytkownika
  PluginUserDir = GetPluginUserDir();
  //Folder wtyczki
  if(!DirectoryExists(PluginUserDir + "\\\\wTray"))
   CreateDir(PluginUserDir + "\\\\wTray");
  //Wypakiwanie pliku Online.png
  //2BD46DE45A5C92DA90295EC0C5A24C94
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\Online.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Online.png").w_str(),L"ONLINE",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\Online.png")!="2BD46DE45A5C92DA90295EC0C5A24C94")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Online.png").w_str(),L"ONLINE",L"DATA");
  //Wypakiwanie pliku Offline.png
  //7BAE9E67C46528FA691A3BBDE55C3BE1
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\Offline.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Offline.png").w_str(),L"OFFLINE",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\Offline.png")!="7BAE9E67C46528FA691A3BBDE55C3BE1")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Offline.png").w_str(),L"OFFLINE",L"DATA");
  //Wypakiwanie pliku Away.png
  //A08AD8C52EA4DBAEC62A482611D00908
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\Away.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Away.png").w_str(),L"AWAY",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\Away.png")!="A08AD8C52EA4DBAEC62A482611D00908")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Away.png").w_str(),L"AWAY",L"DATA");
  //Wypakiwanie pliku XA.png
  //ED59A480A4412159F1E428EA7C811253
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\XA.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\XA.png").w_str(),L"XA",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\XA.png")!="ED59A480A4412159F1E428EA7C811253")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\XA.png").w_str(),L"XA",L"DATA");
  //Wypakiwanie pliku DND.png
  //C3DE0E2059FEECD876FEAED11751256A
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\DND.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\DND.png").w_str(),L"DND",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\DND.png")!="C3DE0E2059FEECD876FEAED11751256A")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\DND.png").w_str(),L"DND",L"DATA");
  //Wypakiwanie pliku Invisible.png
  //F5262A1719F29BAABE6EEA4B5ABDBA53
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\Invisible.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Invisible.png").w_str(),L"INVISIBLE",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\Invisible.png")!="F5262A1719F29BAABE6EEA4B5ABDBA53")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Invisible.png").w_str(),L"INVISIBLE",L"DATA");
  //Wypakiwanie pliku FreeForChat.png
  //63BF317FB4CED422E42D4D6F4C971CBB
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\FreeForChat.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\FreeForChat.png").w_str(),L"FREEFORCHAT",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\FreeForChat.png")!="63BF317FB4CED422E42D4D6F4C971CBB")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\FreeForChat.png").w_str(),L"FREEFORCHAT",L"DATA");
  //Wypakiwanie pliku Message.png
  //2128CBC0D7141F70E5478133C8123A50
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\Message.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Message.png").w_str(),L"MESSAGE",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\Message.png")!="2128CBC0D7141F70E5478133C8123A50")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Message.png").w_str(),L"MESSAGE",L"DATA");
  //Wypakiwanie pliku Secure.png
  //61352B4B83745248D4871DFC63AA114A
  if(!FileExists(PluginUserDir + "\\\\wTray\\\\Secure.png"))
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Secure.png").w_str(),L"SECURE",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\wTray\\\\Secure.png")!="61352B4B83745248D4871DFC63AA114A")
   ExtractRes((PluginUserDir + "\\\\wTray\\\\Secure.png").w_str(),L"SECURE",L"DATA");
  //Przypisanie ikonek do interfejsu AQQ
  ONLINE = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\Online.png").w_str());
  OFFLINE = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\Offline.png").w_str());
  AWAY = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\Away.png").w_str());
  XA = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\XA.png").w_str());
  DND = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\DND.png").w_str());
  INVISIBLE = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\Invisible.png").w_str());
  FREEFORCHAT = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\FreeForChat.png").w_str());
  NEW_MESSAGE = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\Message.png").w_str());
  SECURE = PluginLink.CallService(AQQ_ICONS_LOADPNGICON,0, (LPARAM)(PluginUserDir + "\\\\wTray\\\\Secure.png").w_str());
  //Hook na wylaczenie komunikatora poprzez usera
  PluginLink.HookEvent(AQQ_SYSTEM_BEFOREUNLOAD,OnBeforeUnload);
  //Hook na zakonczenie ladowania listy kontaktow przy starcie AQQ
  PluginLink.HookEvent(AQQ_CONTACTS_LISTREADY,OnListReady);
  //Hook na zmiane ikonki w zasobniku systemowym
  PluginLink.HookEvent(AQQ_SYSTEM_TRAYICONIMAGE,OnTrayIconChange);
  //Wszystkie moduly zostaly zaladowane
  if(PluginLink.CallService(AQQ_SYSTEM_MODULESLOADED,0,0))
   //Odswiezenie ikonki w zasobniku systemowym
   PluginLink.CallService(AQQ_SYSTEM_TRAYICONREFRESH,0,0);

  return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Unload()
{
  //Info o rozpoczeciu procedury wyladowania
  UnloadExecuted = true;
  //Odswiezenie ikonki w zasobniku systemowym
  if(!ForceUnloadExecuted) PluginLink.CallService(AQQ_SYSTEM_TRAYICONREFRESH,0,0);
  //Wyladowanie wszystkich hookow
  PluginLink.UnhookEvent(OnBeforeUnload);
  PluginLink.UnhookEvent(OnListReady);
  PluginLink.UnhookEvent(OnTrayIconChange);
  //Wyladowanie ikonek z intefejsu
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)ONLINE);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)OFFLINE);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)AWAY);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)XA);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)DND);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)INVISIBLE);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)FREEFORCHAT);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)NEW_MESSAGE);
  PluginLink.CallService(AQQ_ICONS_DESTROYPNGICON,0,(LPARAM)SECURE);

  return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" __declspec(dllexport) PPluginInfo __stdcall AQQPluginInfo(DWORD AQQVersion)
{
  PluginInfo.cbSize = sizeof(TPluginInfo);
  PluginInfo.ShortName = L"wTray";
  PluginInfo.Version = PLUGIN_MAKE_VERSION(1,0,0,0);
  PluginInfo.Description = L"Wtyczka zmienia ikonki w zasobniku systemowym tak, aby pasowa³y do systemu Windows od wersji 7 wzwy¿.";
  PluginInfo.Author = L"Krzysztof Grochocki";
  PluginInfo.AuthorMail = L"kontakt@beherit.pl";
  PluginInfo.Copyright = L"Krzysztof Grochocki";
  PluginInfo.Homepage = L"http://beherit.pl";
  PluginInfo.Flag = 0;
  PluginInfo.ReplaceDefaultModule = 0;

  return &PluginInfo;
}
//---------------------------------------------------------------------------
