#include "editor_server_plugin.h"
#include "services/service.h"
#include <tools/editor/editor_settings.h>

namespace gdexplorer {
	EditorServerPlugin::EditorServerPlugin(EditorNode* pEditor): editor(pEditor) {
		server = memnew(EditorServer);
		server->register_service("echo", memnew(EditorServerService));
		auto port = EditorSettings::get_singleton()->get("network/editor_server_port");
		if (port.get_type() == Variant::NIL || !port.is_num())
			port = 6570;
		EditorSettings::get_singleton()->set("network/editor_server_port", port);
		m_notificationParam.push_back(EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED);
		EditorSettings::get_singleton()->connect("settings_changed", this, "_notification", m_notificationParam);
		Globals::get_singleton()->add_singleton( Globals::Singleton("EditorServer", server));
	}

	EditorServerPlugin::~EditorServerPlugin() {
		memdelete(server);
	}

	void EditorServerPlugin::_notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_ENTER_TREE: {
					auto port = EditorSettings::get_singleton()->get("network/editor_server_port");
					server->start(port);
				}
				break;
			case NOTIFICATION_EXIT_TREE:
				server->stop();
				break;
			case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED:{
					auto port = EditorSettings::get_singleton()->get("network/editor_server_port");
					if(int(port) != server->get_port()) {
						server->start(port);
					}
				}
				break;
			default:
				break;
		}
	}

	void EditorServerPlugin::_bind_methods() {
		ObjectTypeDB::bind_method(_MD("_notification","p_what"),&EditorServerPlugin::_notification);
	}
}

