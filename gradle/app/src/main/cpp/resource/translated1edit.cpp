/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <resource/translated1edit.h>
#include <resource/img.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/google.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/config/general.h>
#include <database/books.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <tasks/logic.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <client/logic.h>
using namespace std;


string resource_translated1edit_url ()
{
  return "resource/translated1edit";
}


bool resource_translated1edit_acl (void * webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


string resource_translated1edit (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  string page {};
  Assets_Header header = Assets_Header (translate("Translated resource"), request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view {};
  string error {};
  string success {};
  
  
  string name = request->query ["name"];
  if (name.empty()) name = request->post ["val1"];
  view.set_variable ("name", name);

  
  string checkbox = request->post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (request->post ["checked"]);

  
  bool resource_edited {false};


  string title {};
  string original_resource {};
  string source_language {};
  string target_language {};
  bool cache {false};
  {
    vector <string> resources = Database_Config_General::getTranslatedResources ();
    for (const auto & resource : resources) {
      resource_logic_parse_translated_resource (resource, &title, &original_resource, &source_language, &target_language, &cache);
      if (title == name) break;
    }
  }

  
  // The translated resource's original resource.
  if (request->query.count ("original")) {
    string value = request->query["original"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("translated1edit", translate("Select a resource to be used as the original resource"), translate ("The original resource will be translated from the source language to the target language."), string());
      dialog_list.add_query ("name", name);
      vector <string> resources = resource_logic_get_names (webserver_request, true);
      for (const auto & resource : resources) {
        dialog_list.add_row (resource, "original", resource);
      }
      page += dialog_list.run ();
      return page;
    } else {
      original_resource = value;
      resource_edited = true;
    }
  }

  
  // The language of the original resource.
  if (request->query.count ("source")) {
    string value = request->query["source"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("translated1edit", translate("Select the language of the original resource"), translate ("The language the original resource is written in."), string());
      dialog_list.add_query ("name", name);
      vector <pair <string, string> > languages = filter::google::get_languages ("en");
      for (const auto & language : languages) {
        dialog_list.add_row (language.second, "source", language.first);
      }
      page += dialog_list.run ();
      return page;
    } else {
      source_language = value;
      resource_edited = true;
    }
  }
  
  
  // The language to translate the resource into.
  if (request->query.count ("target")) {
    string value = request->query["target"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("translated1edit", translate("Select the language to translate the resource into"), translate ("The language the resource will be translated into."), string());
      dialog_list.add_query ("name", name);
      vector <pair <string, string> > languages = filter::google::get_languages ("en");
      for (const auto & language : languages) {
        dialog_list.add_row (language.second, "target", language.first);
      }
      page += dialog_list.run ();
      return page;
    } else {
      target_language = value;
      resource_edited = true;
    }
  }
  
  
  // Whether to cache the resource on client devices.
  if (checkbox == "cache") {
    cache = checked;
    resource_edited = true;
  }


  // If the resource was edited, then take some steps.
  if (resource_edited) {
    // Save the translated resource.
    vector <string> resources = Database_Config_General::getTranslatedResources ();
    error = translate ("Could not save");
    for (size_t i = 0; i < resources.size(); i++) {
      string title2 {};
      resource_logic_parse_translated_resource (resources[i], &title2);
      if (title2 == title) {
        string resource = resource_logic_assemble_translated_resource (title, original_resource, source_language, target_language, cache);
        resources[i] = resource;
        success = translate ("Saved");
        error.clear();
      }
    }
    Database_Config_General::setTranslatedResources (resources);
    // Update the list of resources not to be cached on the client devices.
    if (cache) client_logic_no_cache_resource_remove(title);
    else client_logic_no_cache_resource_add(title);
    // Store the list of translated resources for download by the client devices.
    {
      string path = resource_logic_translated_resources_list_path ();
      filter_url_file_put_contents (path, filter::strings::implode (resources, "\n"));
    }
  }
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  view.set_variable ("title", title);
  view.set_variable ("original", original_resource);
  view.set_variable ("source", source_language);
  view.set_variable ("target", target_language);
  view.set_variable ("cache", filter::strings::get_checkbox_status (cache));
  page += view.render ("resource", "translated1edit");
  page += assets_page::footer ();
  return page;
}
