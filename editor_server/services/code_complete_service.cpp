#include "code_complete_service.h"
#include <core/os/file_access.h>
#include "io/resource_loader.h"
#include "globals.h"
#include "modules/gdscript/gd_script.h"
#include <tools/editor/editor_node.h>
#include <core/list.h>

namespace gdexplorer {

	Node* _find_node_for_script(Node* p_base, Node* p_current, const Ref<Script>& p_script);
	String _get_text_for_completion(const CodeCompleteService::Request& p_request, String& r_text);
	String _filter_completion_candidates(int p_col, const String& p_line, const List<String>& p_options, Vector<String> &r_suggestions);

	Dictionary CodeCompleteService::resolve(const Dictionary &_data) const {
		Dictionary data(_data);
		Request request(data["request"]);
		Result result = complete_code(request);
		return super::resolve(data);
	}

	bool CodeCompleteService::Request::valid() const {
		return !(column == 1 && row == 1) && script_path.length();
	}

	CodeCompleteService::Request::Request(const Dictionary &request):row(1), column(1), script_text(""), script_path("") {
		String text = request.has("text")? request["text"]:"";
		script_text = text;

		String path = request.has("path")? request["path"]:"";
		path = Globals::get_singleton()->localize_path(path);
		if(path == "res://" || !path.begins_with("res://"))
			path = "";
		script_path = path;

		if (request.has("cursor")) {
			Dictionary cursor = request["cursor"];
			row = cursor.has("row")?int(cursor["row"]):1;
			column = cursor.has("column")?int(cursor["column"]):1;
		}
	}

	CodeCompleteService::Result CodeCompleteService::complete_code(const CodeCompleteService::Request &request) const {
		Result result;
		if(request.valid()) {
			Ref<GDScript> script = ResourceLoader::load(request.script_path);
			if (!script.is_valid() || !script->cast_to<GDScript>())
				return Result();
			String script_text = request.script_text;
			if (script_text.empty()) {
				ERR_FAIL_COND_V(!script->has_source_code(), Result());
				script_text = script->get_source_code();
			}

			Node *node = EditorNode::get_singleton()->get_tree()->get_edited_scene_root();
			if(node)
				node = _find_node_for_script(node, node, script);

			String current_line = _get_text_for_completion(request, script_text);

			List<String> options;
			script->get_language()->complete_code(script_text, script->get_path().get_base_dir(), node, &options, result.hint);
			if (options.size()) {
//				result.prefix = _filter_completion_candidates(request.column, current_line, options, result.suggestions);
			}
			result.valid = true;
		}
		return result;
	}


	Node* _find_node_for_script(Node* p_base, Node* p_current, const Ref<Script>& p_script) {

		if (p_current->get_owner()!=p_base && p_base!=p_current)
			return NULL;
		Ref<Script> c = p_current->get_script();
		if (c==p_script)
			return p_current;
		for(int i=0;i<p_current->get_child_count();i++) {
			Node *found = _find_node_for_script(p_base,p_current->get_child(i),p_script);
			if (found)
				return found;
		}
		return NULL;
	}

	String _get_text_for_completion(const CodeCompleteService::Request& p_request, String& r_text) {

		Vector<String> substrings = r_text.replace("\r","").split("\n");
		r_text.clear();

		int len = substrings.size();

		for (int i=0;i<len;i++) {
			if (i==p_request.row) {
				r_text+=substrings[i].substr(0,p_request.column);
				r_text+=String::chr(0xFFFF); //not unicode, represents the cursor
				r_text+=substrings[i].substr(p_request.column,substrings[i].size());
			} else {
				r_text+=substrings[i];
			}

			if (i!=len-1)
				r_text+="\n";
		}

		return substrings[p_request.row];
	}
}

