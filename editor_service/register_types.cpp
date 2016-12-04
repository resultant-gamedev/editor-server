/* register_types.cpp */
#include "register_types.h"

#ifdef EDITOR_SERVICE
#include "editor_service_plugin.h"
#include "editor_server.h"
#include "services/service.h"
using namespace gdexplorer;
#endif

void register_editor_service_types() {
#ifdef EDITOR_SERVICE
	EditorPlugins::add_by_type<EditorServicePlugin>();
	ObjectTypeDB::register_type<EditorServer>();
	ObjectTypeDB::register_type<EditorServerService>();
#endif
}

void unregister_editor_service_types() {

}
