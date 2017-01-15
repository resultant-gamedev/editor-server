#include "service.h"
#include "script_language.h"
namespace gdexplorer {


	void EditorServerService::_bind_methods() {
		ClassDB::add_virtual_method(get_class_static(), MethodInfo(Variant::DICTIONARY,"resolve",PropertyInfo(Variant::DICTIONARY,"request")));
	}

	Dictionary EditorServerService::resolve(const Dictionary &data) const {
		if (get_script_instance() && get_script_instance()->has_method("resolve")) {
			return get_script_instance()->call("resolve", data);
		}
		else {
			return data;
		}
	}

}
