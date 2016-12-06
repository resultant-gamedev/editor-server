#ifndef CODECOMPLETESERVICE_H
#define CODECOMPLETESERVICE_H

#include "service.h"

namespace gdexplorer {
	class CodeCompleteService : public EditorServerService
	{
		OBJ_TYPE(CodeCompleteService,EditorServerService);
		using super = EditorServerService;
	public:
		struct Request {
			bool valid() const;
			int row;
			int column;
			String script_text;
			String script_path;
			Request(const Dictionary& dict);
		};
		struct Result {
			bool valid = false;
			String prefix;
			String hint;
			Vector<String> suggestions;
		};
		Result complete_code(const Request& request) const;

	public:
		virtual Dictionary resolve(const Dictionary& _data) const override;
		CodeCompleteService() = default;
		virtual ~CodeCompleteService() = default;
	};
}


#endif // CODECOMPLETESERVICE_H
