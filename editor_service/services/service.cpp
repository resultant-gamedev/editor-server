#include "service.h"
namespace gdexplorer {


	void EditorServerService::_bind_methods() {
		ObjectTypeDB::bind_method(_MD("resolve", "request"),&EditorServerService::resolve);
		BIND_VMETHOD( MethodInfo("resolve") );
	}

}
