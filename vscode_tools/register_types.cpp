/* register_types.cpp */
#include "register_types.h"

#ifdef EDITOR_SERVICE
#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_settings.h"
#include <core/os/file_access.h>
#include <core/os/dir_access.h>

namespace gdexplorer {

	class VSCodeToolsPlugin : public EditorPlugin
	{
		OBJ_TYPE(VSCodeToolsPlugin, EditorPlugin);
		EditorNode *editor;
		Vector<Variant> m_notificationParam;
		Variant port;
		Variant problem_max;
	protected:

		void _notification(int p_what) {
			if (p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {
				port = EditorSettings::get_singleton()->get("network/editor_server_port");
				problem_max = EditorSettings::get_singleton()->get("vscode/max_number_of_problems");
				this->_save_settings();
			}
		}

		void _save_settings() {
			Dictionary settings;
			settings["GodotTools.editorServerPort"] = port;
			settings["GodotTools.maxNumberOfProblems"] = problem_max;

			DirAccess* dir = DirAccess::create_for_path("res://");
			if(dir && !dir->dir_exists(".vscode"))
				dir->make_dir(".vscode");
			else if (dir)
				memdelete(dir);

			FileAccess * file = FileAccess::open("res://.vscode/settings.json", FileAccess::WRITE);
			if(file && file->get_error() == OK) {
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
			ObjectTypeDB::bind_method(_MD("_notification","p_what"),&VSCodeToolsPlugin::_notification);
		}

	public:
		VSCodeToolsPlugin(EditorNode* pEditor): editor(pEditor) {
			auto problem_max = EditorSettings::get_singleton()->get("vscode/max_number_of_problems");
			if (problem_max.get_type() == Variant::NIL || !problem_max.is_num())
				problem_max = 100;
			EditorSettings::get_singleton()->set("vscode/max_number_of_problems", problem_max);
			EditorSettings::get_singleton()->connect("settings_changed", this, "_notification", m_notificationParam);
		};
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
