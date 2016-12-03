#include "editor_service_plugin.h"
#include "services/service.h"

namespace gdexplorer {

	EditorServicePlugin::EditorServicePlugin(EditorNode* pEditor): editor(pEditor) {
		server = memnew(EditorServer);
		server->register_service("echo", Service());
	}

	EditorServicePlugin::~EditorServicePlugin() {
		memdelete(server);
	}

	void EditorServicePlugin::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			server->start();
			break;
		case NOTIFICATION_EXIT_TREE:
			server->stop();
			break;
		default:
			break;
		}
	}
}

