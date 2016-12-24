#ifndef GD_EXPLORER_SERVICE_H
#define GD_EXPLORER_SERVICE_H

#include <core/reference.h>
#include <core/dictionary.h>

namespace gdexplorer {

	class EditorServerService : public Reference {
		OBJ_TYPE(EditorServerService, Reference);
	protected:
		static void _bind_methods();
	public:
		EditorServerService() = default;
		virtual ~EditorServerService() = default;
//		EditorServerService& operator=(EditorServerService&) = default;
		virtual Dictionary resolve(const Dictionary& data) const;
	};

}

#endif // GD_EXPLORER_SERVICE_H
