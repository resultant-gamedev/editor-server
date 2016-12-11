#include "script_parse_service.h"
#include <core/globals.h>
#include <core/script_language.h>
#include <io/resource_loader.h>
#include <tools/editor/editor_node.h>
#include <core/array.h>
#ifdef GDSCRIPT_ENABLED
#include "modules/gdscript/gd_parser.h"
#include "modules/gdscript/gd_compiler.h"
#endif

namespace gdexplorer {

	Dictionary ScriptParseService::resolve(const Dictionary &_data) const {
		Dictionary data(_data);
		Request request(data["request"]);
		Result result = parse_script(request);
		data["result"] = Dictionary(result);
		return super::resolve(data);
	}

	bool ScriptParseService::Request::valid() const {
		return !script_path.empty() && !script_text.empty();
	}

	ScriptParseService::Request::Request(const Dictionary &request) {
		String path = request.has("path")? request["path"]:"";
		path = Globals::get_singleton()->localize_path(path);
		if(path == "res://" || !path.begins_with("res://"))
			path = "";
		script_path = path;

		script_text = request.has("text")? request["text"]:"";
		if (!script_path.empty() && script_text.empty()) {
			Ref<Script> script = ResourceLoader::load(script_path);
			if (!script.is_null() && script.is_valid() && script->cast_to<Script>())
				script_text = script->get_source_code();
		}
	}

	ScriptParseService::Result ScriptParseService::parse_script(const ScriptParseService::Request &request) const {
		Result result;
		if(request.valid()) {
#ifdef GDSCRIPT_ENABLED
			GDParser parser;
			int err = parser.parse(request.script_text, request.script_path.get_base_dir(), true, request.script_path, false);
			result.valid = (err == OK);
			if(!result.valid) {
				Error e;
				e.message = parser.get_error();
				e.row = parser.get_error_line();
				e.column = parser.get_error_column();
				result.errors.push_back(e);
			}

			GDScript script;
			script.set_source_code(request.script_text);
			script.set_script_path(request.script_path);
			script.set_path(request.script_path);

			GDCompiler compiler;
			if(OK != compiler.compile(&parser, &script)){
				Error e;
				e.message = compiler.get_error();
				e.row = compiler.get_error_line();
				e.column = compiler.get_error_column();
				if(-1 == result.errors.find(e))
					result.errors.push_back(e);
				result.valid = false;
			} else {
				auto _functions = script.get_member_functions();
				for(auto E = _functions.front(); E; E=E->next()) {
					Member m;
					m.name = E->key();
					m.line = script.get_member_line(E->key());
					result.functions.push_back(m);
				}

				auto _members = script.get_members();
				for(auto E = _members.front(); E; E=E->next()) {
					Member m;
					m.name = E->get();
					m.line = script.get_member_line(E->get());
					result.members.push_back(m);
				}

				auto _constants = script.get_constants();
				for(auto E = _constants.front(); E; E=E->next()) {
					Member m;
					m.name = E->key();
					m.line = script.get_member_line(E->key());
					result.constants.push_back(m);
				}

				List<MethodInfo> _signals;
				script.get_script_signal_list(&_signals);
				for(auto E = _signals.front(); E; E=E->next()) {
					Member m;
					m.name = E->get().name;
					m.line = script.get_member_line(m.name);
					result.signals.push_back(m);
				}

				result.native_calss = script.get_native()->get_name();

				auto base = script.get_base();
				if(!base.is_null() && base.is_valid()) {
					result.base_class = base->get_name();
					if(result.base_class.empty())
						result.base_class = base->get_path();
				}
				if(result.base_class.empty())
					result.base_class = result.native_calss;
				result.is_tool = script.is_tool();


//				Node *node = EditorNode::get_singleton()->get_tree()->get_edited_scene_root();
//				if(node)
//					_find_nodes_for_script(node, node, result.attached_nodes, request.script_path);

				result.valid = true;
			}
#endif
		}
		return result;
	}

	gdexplorer::ScriptParseService::Result::operator Dictionary() const {
		Dictionary data;
		data["valid"] = valid;
		data["is_tool"] = is_tool;
		data["base"] = base_class;
		data["native"] = native_calss;

		Vector<Variant> _errors;
		for(int i=0; i<errors.size(); ++i )
			_errors.push_back(errors[i]);
		data["errors"] = _errors;

		auto export_members = [](const Vector<Member>& mems) {
			Dictionary mem;
			for(int i=0; i<mems.size(); ++i )
				mem[mems[i].name] = mems[i].line;
			return mem;
		};

		data["members"] = export_members(members);
		data["functions"] = export_members(functions);
		data["signals"] = export_members(signals);
		data["constants"] = export_members(constants);

		return data;
	}

	//	void _find_nodes_for_script(Node* p_base, Node* p_current, Vector<String>& r_container,const String& script_path) {

	//		if (p_current->get_owner() != p_base && p_base != p_current)
	//			return;
	//		Ref<Script> c = p_current->get_script();
//		if (!c.is_null() && c.is_valid() && c->get_path() == script_path) {
//			String path = p_current->get_path();
//			if(!path.empty()) {
//				r_container.push_back(path);
//			}
//		}
//		for(int i=0; i < p_current->get_child_count(); i++)
//			_find_nodes_for_script(p_base, p_current->get_child(i), r_container, script_path);
//	}

}


