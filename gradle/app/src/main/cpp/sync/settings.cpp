/*
 Copyright (©) 2003-2024 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <sync/settings.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
using namespace std;


string sync_settings_url ()
{
  return "sync/settings";
}


string sync_settings (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return "";
  }

  // Check on the credentials.
  if (!sync_logic.credentials_okay ()) return "";
  
  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();

  // Get the relevant parameters the client POSTed to us, the server.
  int action = filter::strings::convert_to_int (webserver_request.post ["a"]);
  string value = webserver_request.post ["v"];
  // The value can be all Bibles, or one Bible.
  string bible_s = webserver_request.post ["b"];

  switch (action) {
    case Sync_Logic::settings_get_total_checksum:
    {
      return sync_logic.settings_checksum (filter::strings::explode (bible_s, '\n'));
    }
    case Sync_Logic::settings_send_workspace_urls:
    {
      webserver_request.database_config_user()->setWorkspaceURLs (value);
      return string();
    }
    case Sync_Logic::settings_get_workspace_urls:
    {
      return webserver_request.database_config_user()->getWorkspaceURLs ();
    }
    case Sync_Logic::settings_send_workspace_widths:
    {
      webserver_request.database_config_user()->setWorkspaceWidths (value);
      return string();
    }
    case Sync_Logic::settings_get_workspace_widths:
    {
      return webserver_request.database_config_user()->getWorkspaceWidths ();
    }
    case Sync_Logic::settings_send_workspace_heights:
    {
      webserver_request.database_config_user()->setWorkspaceHeights (value);
      return string();
    }
    case Sync_Logic::settings_get_workspace_heights:
    {
      return webserver_request.database_config_user()->getWorkspaceHeights ();
    }
    case Sync_Logic::settings_send_resources_organization:
    {
      vector <string> resources = filter::strings::explode (value, '\n');
      webserver_request.database_config_user()->setActiveResources (resources);
      return string();
    }
    case Sync_Logic::settings_get_resources_organization:
    {
      vector <string> resources = webserver_request.database_config_user()->getActiveResources ();
      return filter::strings::implode (resources, "\n");
    }
    case Sync_Logic::settings_get_bible_id:
    {
      // No longer in use since June 2016.
      return "1";
    }
    case Sync_Logic::settings_get_bible_font:
    {
      return Database_Config_Bible::getTextFont (bible_s);
    }
    case Sync_Logic::settings_send_platform:
    {
      // No longer in use, just discard this.
      return string();
    }
    case Sync_Logic::settings_get_privilege_delete_consultation_notes:
    {
      return filter::strings::convert_to_string (webserver_request.database_config_user()->getPrivilegeDeleteConsultationNotes ());
    }
    default:
    {
    }
  }

  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  webserver_request.response_code = 400;
  return "";
}
