#ifndef GD_EXPLORER_SCRIPTPARSESERVICE_H
#define GD_EXPLORER_SCRIPTPARSESERVICE_H

#include "service.h"

namespace gdexplorer {
	class ScriptParseService : public EditorServerService {
		OBJ_TYPE(ScriptParseService,EditorServerService);
		using super = EditorServerService;
	protected:
		struct Error {
			String message;
			int row = -1;
			int column = -1;
			bool operator==(const Error& e) const {
				return e.message == message && e.column == column && e.row == row;
			}
			operator Variant() const {
				Dictionary d;
				d["message"] = message;
				d["row"] = row;
				d["column"] = column;
				return d;
			}
		};

		struct Member {
			String name;
			int line = -1;
			operator Variant() const {
				Dictionary d;
				d[name] = line;
				return d;
			}
		};

		struct Request {
			bool valid() const;
			String script_text;
			String script_path;
			Request(const Dictionary& dict);
		};

		struct Result {
			bool valid = false;
			bool is_tool = false;
			String base_class;
			String native_calss;
			Vector<Error> errors;
			Vector<Member> functions;
			Vector<Member> members;
			Vector<Member> signals;
			Vector<Member> constants;
			operator Dictionary() const;
		};

		Result parse_script(const Request& request) const;
	public:
		virtual Dictionary resolve(const Dictionary& _data) const override;
		ScriptParseService() = default;
		virtual ~ScriptParseService() = default;
	};

}

#endif // GD_EXPLORER_SCRIPTPARSESERVICE_H
