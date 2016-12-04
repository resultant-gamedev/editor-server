#ifndef GD_EXPLORER_EDITORSERVICEPLUGIN_H
#define GD_EXPLORER_EDITORSERVICEPLUGIN_H

#include "tools/editor/editor_plugin.h"
#include "editor_server.h"

namespace gdexplorer {

	class EditorServerPlugin : public EditorPlugin
	{
		OBJ_TYPE(EditorServerPlugin, EditorPlugin);
		EditorNode *editor;
		EditorServer *server;
		Vector<Variant> m_notificationParam;
	protected:
		void _notification(int p_what);
		static void _bind_methods();
	public:
		EditorServerPlugin(EditorNode* editor);
		~EditorServerPlugin();
	};
}


#endif // GD_EXPLORER_EDITORSERVICEPLUGIN_H
