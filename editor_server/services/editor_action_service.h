#ifndef EDITOR_ACTION_SERVICE_H
#define EDITOR_ACTION_SERVICE_H

#include "service.h"

namespace gdexplorer {
	class EditorActionService : public EditorServerService
	{
		OBJ_TYPE(EditorActionService, EditorServerService);
		using super = EditorServerService;
	public:
		virtual Dictionary resolve(const Dictionary& data) const override;
		EditorActionService() = default;
		virtual ~EditorActionService() = default;
	};
}

#endif // EDITOR_ACTION_SERVICE_H
