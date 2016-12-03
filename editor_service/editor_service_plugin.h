#ifndef GD_EXPLORER_EDITORSERVICEPLUGIN_H
#define GD_EXPLORER_EDITORSERVICEPLUGIN_H

#include "tools/editor/editor_plugin.h"
#include "editor_server.h"

namespace gdexplorer {
	class EditorServicePlugin : public EditorPlugin
	{
		OBJ_TYPE(EditorServicePlugin, EditorPlugin);
		EditorNode *editor;
		EditorServer *server;
	protected:
		void _notification(int p_what);
	public:
		EditorServicePlugin(EditorNode* editor);
		~EditorServicePlugin();
	};
}


#endif // GD_EXPLORER_EDITORSERVICEPLUGIN_H
