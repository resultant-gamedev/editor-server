/* register_types.cpp */
#include "register_types.h"

#ifdef EDITOR_SERVICE
#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_settings.h"
#include <core/os/file_access.h>
#include <core/os/dir_access.h>
#include <core/globals.h>
#include <tools/editor/editor_node.h>

namespace gdexplorer {

	class VSCodeToolsPlugin : public EditorPlugin
	{
		OBJ_TYPE(VSCodeToolsPlugin, EditorPlugin);
		EditorNode *editor;
		Vector<Variant> m_notificationParam;
		Variant port = 6570;
		Variant problem_max = 100;
		Variant highlight_res = true;
		String reslang = "toml";
		Vector<String> text_res_exts;
	protected:

		void _notification(int p_what) {
			if (p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {
				Variant port = EditorSettings::get_singleton()->get("network/editor_server_port");
				Variant problem_max = EditorSettings::get_singleton()->get("vscode/max_number_of_problems");
				Variant highlight_res = EditorSettings::get_singleton()->get("vscode/highlight_resources");
				bool changed = this->port != port || this->problem_max != problem_max || this->highlight_res != highlight_res;
				this->port = port;
				this->problem_max = problem_max;
				this->highlight_res = highlight_res;
				if(changed) {
					this->_save_settings();
				}
			}
		}

		void _save_settings() {
			DirAccess* dir = DirAccess::create_for_path("res://");
			if(dir && !dir->dir_exists(".vscode"))
				dir->make_dir(".vscode");
			if(dir)
				memdelete(dir);
			String configfile = "res://.vscode/settings.json";
			FileAccess * file  = FileAccess::create_for_path(configfile);
			int mode = 0;
			if(file->file_exists(configfile)) {
				memdelete(file);
				mode = FileAccess::READ_WRITE;
				file = FileAccess::open(configfile, FileAccess::READ_WRITE);
			}
			else {
				memdelete(file);
				mode = FileAccess::WRITE;
				file = FileAccess::open(configfile, FileAccess::WRITE);
			}
			if(file && file->get_error() == OK) {
				String content;

				Vector<uint8_t> data;
				int len = int(file->get_len());
				data.resize(len);
				file->get_buffer(data.ptr(), len);
				content.parse_utf8((const char*)data.ptr(), len);

				Dictionary settings;
				settings.parse_json(content);
				settings["GodotTools.editorServerPort"] = port;
				settings["GodotTools.maxNumberOfProblems"] = problem_max;
				Dictionary associations;
				if(settings.has("files.associations"))
					associations = settings["files.associations"];
				String text_res_lang = highlight_res? reslang : "plaintext";
				for(int i= 0; i<text_res_exts.size(); ++i)
					associations[text_res_exts[i]] = text_res_lang;
				settings["files.associations"] = associations;

				if(mode == FileAccess::READ_WRITE) {
					file->close();
					memdelete(file);
					file = FileAccess::open(configfile, FileAccess::WRITE);
				}
				file->store_string(settings.to_json());
				file->close();
				memdelete(file);
			}
			else {
				if(file)
					memdelete(file);
				ERR_PRINTS("[VSCodeTools]: Generate setting.json failed");
			}
		}

		static void _bind_methods() {
			ObjectTypeDB::bind_method(_MD("_notification", "p_wath"),&VSCodeToolsPlugin::_notification);
		}

	public:
		VSCodeToolsPlugin(EditorNode* pEditor): editor(pEditor) {
			text_res_exts.push_back("*.tres");
			text_res_exts.push_back("*.tscn");
			text_res_exts.push_back("*.cfg");
			if(!EditorSettings::get_singleton()->has("vscode/max_number_of_problems"))
				EditorSettings::get_singleton()->set("vscode/max_number_of_problems", problem_max);
			if(!EditorSettings::get_singleton()->has("vscode/highlight_resources"))
				EditorSettings::get_singleton()->set("vscode/highlight_resources", highlight_res);
			m_notificationParam.push_back(EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED);
			EditorSettings::get_singleton()->connect("settings_changed", this, "_notification", m_notificationParam);
		}
		~VSCodeToolsPlugin() = default;
	};
}

#endif

void register_vscode_tools_types() {
#ifdef EDITOR_SERVICE
	EditorPlugins::add_by_type<gdexplorer::VSCodeToolsPlugin>();
#endif
}

void unregister_vscode_tools_types() {

}
