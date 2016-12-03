#include "editor_service_plugin.h"

namespace gdexplorer {

	void EditorServicePlugin::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			print_line("EditorServicePlugin: enter tree");
			server->start();
			break;
		case NOTIFICATION_EXIT_TREE:
			print_line("EditorServicePlugin: exit tree");
			break;
		default:
			break;
		}
	}

	EditorServicePlugin::EditorServicePlugin(EditorNode* pEditor): editor(pEditor) {
		server = memnew(EditorServer);
	}

	EditorServicePlugin::~EditorServicePlugin() {
		memdelete(server);
	}
}

