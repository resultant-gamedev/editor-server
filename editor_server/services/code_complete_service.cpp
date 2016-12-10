#include "code_complete_service.h"
#include <core/os/file_access.h>
#include <io/resource_loader.h>
#include <core/globals.h>
#include <tools/editor/editor_node.h>
#include <core/list.h>
#include <initializer_list>

#ifdef GDSCRIPT_ENABLED
#include "modules/gdscript/gd_script.h"
#endif

namespace gdexplorer {

	Node* _find_node_for_script(Node* p_base, Node* p_current, const CodeCompleteService::Request& request);
	String _get_text_for_completion(const CodeCompleteService::Request& p_request, String& r_text);
	String _filter_completion_candidates(int p_col, const String& p_line, const List<String>& p_options, List<String>& p_keywords,Vector<String> &r_suggestions);
	static bool _is_symbol(CharType c) {
		return c!='_' && ((c>='!' && c<='/') || (c>=':' && c<='@') || (c>='[' && c<='`') || (c>='{' && c<='~') || c=='\t');
	}
	static bool _is_completable(CharType c) {
		return !_is_symbol(c) || c=='"' || c=='\'';
	};

	Dictionary CodeCompleteService::resolve(const Dictionary &_data) const {
		Dictionary data(_data);
		Request request(data["request"]);
		Result result = complete_code(request);

		Dictionary r_data;
		r_data["valid"] = result.valid;
		r_data["prefix"] = result.prefix;
		r_data["hint"] = result.hint.replace(String::chr(0xFFFF), "\n");
		r_data["suggestions"]=result.suggestions;
		data["result"] = r_data;

		return super::resolve(data);
	}

	CodeCompleteService::CodeCompleteService() {
#ifdef GDSCRIPT_ENABLED
		GDScriptLanguage::get_singleton()->get_reserved_words(&keywords);
#endif
		for(const String& _keyword : {
			"Vector2", "Vector3","Plane","Quat","AABB","Matrix3","Transform", "Color",
			"Image","InputEvent","Rect2","NodePath"}){
			keywords.push_back(_keyword);
		}
		List<StringName> keywordsn;
		ObjectTypeDB::get_type_list(&keywordsn);
		for(List<StringName>::Element *E=keywordsn.front();E;E=E->next()) {
			keywords.push_back(E->get());
		}
	}

	bool CodeCompleteService::Request::valid() const {
		return !(column <= 1 && row <= 1) && !script_path.empty() && !script_text.empty();
	}

	CodeCompleteService::Request::Request(const Dictionary &request):row(1), column(1), script_text(""), script_path("") {
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

		if (request.has("cursor")) {
			Dictionary cursor = request["cursor"];
			row = cursor.has("row")?int(cursor["row"]):1;
			column = cursor.has("column")?int(cursor["column"]):1;
			row = (row <= 1)? 1 : row;
			column = (column <= 1)? 1 : column;
		}
	}

	CodeCompleteService::Result CodeCompleteService::complete_code(const CodeCompleteService::Request &request) const {
		Result result;
		if(request.valid()) {
			Node *node = EditorNode::get_singleton()->get_tree()->get_edited_scene_root();
			if(node)
				node = _find_node_for_script(node, node, request);
			String complete_code = request.script_text;
			String current_line = _get_text_for_completion(request, complete_code);
			if(!current_line.empty()) {
				List<String> options;
#ifdef GDSCRIPT_ENABLED
				GDScriptLanguage::get_singleton()->complete_code(complete_code, request.script_path.get_base_dir(), node, &options, result.hint);
#endif
				if (options.size())
					result.prefix = _filter_completion_candidates(request.column-1, current_line, options, keywords, result.suggestions);
			}
			result.valid = result.prefix.length() > 0;
		}
		return result;
	}


	Node* _find_node_for_script(Node* p_base, Node* p_current, const CodeCompleteService::Request& request) {

		if (p_current->get_owner() !=p_base && p_base != p_current)
			return nullptr;
		Ref<Script> c = p_current->get_script();
		if (!c.is_null() && c.is_valid() && c->get_path() == request.script_path)
			return p_current;
		for(int i=0; i<p_current->get_child_count(); i++) {
			Node *found = _find_node_for_script(p_base,p_current->get_child(i),request);
			if (found)
				return found;
		}
		return nullptr;
	}

	String _get_text_for_completion(const CodeCompleteService::Request& p_request, String& r_text) {
		Vector<String> substrings = r_text.replace("\r","").split("\n");
		const int row = p_request.row - 1;
		const int len = substrings.size();
		if(row >= len)
			return String();
		r_text.clear();
		for (int i=0; i<len; i++) {
			if (i==row) {
				r_text+=substrings[i].substr(0,p_request.column);
				r_text+=String::chr(0xFFFF); //not unicode, represents the cursor
				r_text+=substrings[i].substr(p_request.column,substrings[i].size());
			} else {
				r_text+=substrings[i];
			}

			if (i!=len-1)
				r_text+="\n";
		}

		return substrings[row];
	}

	String _filter_completion_candidates(int p_col, const String& p_line, const List<String>& p_options, List<String>& p_keywords, Vector<String> &r_suggestions){
		int cofs = CLAMP(p_col, 0, p_line.length());
		const int column = cofs;

		String s;
		bool cancel=false;

		bool inquote=false;
		int first_quote=-1;
		int c=cofs-1;
		while(c>=0) {
			if (p_line[c]=='"' || p_line[c]=='\'') {
				inquote=!inquote;
				if (first_quote==-1)
					first_quote=c;
			}
			c--;
		}

		bool pre_keyword=false;
		if (!inquote && first_quote==cofs-1) {
			cancel=true;
		} if (inquote && first_quote!=-1) {
			s=p_line.substr(first_quote,cofs-first_quote);
		} else if (cofs>0 && p_line[cofs-1]==' ') {
			int kofs=cofs-1;
			String kw;
			while (kofs>=0 && p_line[kofs]==' ')
				kofs--;
			while(kofs>=0 && p_line[kofs]>32 && _is_completable(p_line[kofs])) {
				kw=String::chr(p_line[kofs])+kw;
				kofs--;
			}
			pre_keyword=p_keywords.find(kw);

		} else {
			while(cofs>0 && p_line[cofs-1]>32 && _is_completable(p_line[cofs-1])) {
				s=String::chr(p_line[cofs-1])+s;
				if (p_line[cofs-1]=='\'' || p_line[cofs-1]=='"')
					break;
				cofs--;
			}
		}

		if (column > 0 && p_line[column - 1] == '(' && !pre_keyword && !p_options[0].begins_with("\"")) {
			cancel = true;
		}

		Set<String> completion_prefixes;
		completion_prefixes.insert(".");
		completion_prefixes.insert(",");
		completion_prefixes.insert("(");
		if (cancel || (!pre_keyword && s=="" && (cofs==0 || !completion_prefixes.has(String::chr(p_line[cofs-1]))))) {
			//none to complete, cancel
			return String();
		}

		r_suggestions.clear();
		Vector<float> sim_cache;
		for(int i=0;i<p_options.size();i++) {
			if (s == p_options[i]) {
				// A perfect match, stop completion
				return s;
			}
			if (s.is_subsequence_ofi(p_options[i])) {
				// don't remove duplicates if no input is provided
				if (s != "" && r_suggestions.find(p_options[i]) != -1) {
					continue;
				}
				// Calculate the similarity to keep completions in good order
				float similarity;
				if (p_options[i].to_lower().begins_with(s.to_lower())) {
					// Substrings are the best candidates
					similarity = 1.1f;
				} else {
					// Otherwise compute the similarity
					similarity = s.to_lower().similarity(p_options[i].to_lower());
				}

				int comp_size = r_suggestions.size();
				if (comp_size == 0) {
					r_suggestions.push_back(p_options[i]);
					sim_cache.push_back(similarity);
				} else {
					float comp_sim;
					int pos = 0;
					do {
						comp_sim = sim_cache[pos++];
					} while(pos < comp_size && similarity < comp_sim);
					pos = similarity > comp_sim ? pos - 1 : pos; // Pos will be off by one
					r_suggestions.insert(pos, p_options[i]);
					sim_cache.insert(pos, similarity);
				}
			}
		}
		if (r_suggestions.size()==0) {
			return String();
		}
		return r_suggestions[0];
	}
}
