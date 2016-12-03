/* register_types.cpp */
#include "register_types.h"

#ifdef EDITOR_SERVICE
#include "editor_service_plugin.h"
#endif

void register_editor_service_types() {
#ifdef EDITOR_SERVICE
	EditorPlugins::add_by_type<gdexplorer::EditorServicePlugin>();
#endif
}

void unregister_editor_service_types() {

}
