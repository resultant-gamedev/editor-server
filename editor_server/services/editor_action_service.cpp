#include "editor_action_service.h"
#include <core/os/os.h>

namespace gdexplorer {

	Dictionary EditorActionService::resolve(const Dictionary &_data) const {
		Dictionary data = _data;
		if(data.has("command")) {
			const String& command = data["command"];
			if(command == "alter") {
				String content = data.has("content")? data["content"] : "";
				String title = data.has("title")? data["title"] : "";
				OS::get_singleton()->alert(content, title);
			}
			else if (command == "fullscreen") {
				OS::get_singleton()->set_window_fullscreen(true);
				data["fullscreen"] = true;
			}
			else if (command == "unfullscreen") {
				OS::get_singleton()->set_window_fullscreen(false);
				data["fullscreen"] = false;
			}
		}
		return super::resolve(data);
	}

}
