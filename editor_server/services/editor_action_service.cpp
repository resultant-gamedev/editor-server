#include "editor_action_service.h"
#include <core/os/os.h>
#include <core/globals.h>
#include <core/os/os.h>
#include <tools/doc/doc_data.h>
#include <tools/editor/editor_help.h>

namespace gdexplorer {

	Dictionary _resolveDocData(const DocData* p_doc);

	Dictionary EditorActionService::resolve(const Dictionary &_data) const {
		Dictionary data = _data;
		if(data.has("command")) {
			const String& command = data["command"];
			if(command == "alter") {
				String content = data.has("content")? data["content"] : "";
				String title = data.has("title")? data["title"] : "";
				OS::get_singleton()->alert(content, title);
			}
			else if(command == "projectdir") {
				data["path"] = Globals::get_singleton()->globalize_path("res://");
			}
			else if(command == "editorpath") {
				data["path"] = OS::get_singleton()->get_executable_path();
			}
			else if(command == "version") {
				data.clear();
				data["version"] = __DATE__ " " __TIME__;
				return data;
			}
			else if(command == "gendoc") {
				String path = data.has("path")?data["path"]:"";
				bool done = false;
				if(!path.empty()) {
					const DocData* doc = EditorHelp::get_doc_data();
					Dictionary docdata = _resolveDocData(doc);
					FileAccess* file = FileAccess::open(path, FileAccess::WRITE);
					if(file && file->get_error() == OK) {
						file->store_string(docdata.to_json());
						file->close();
						done = file->get_error() == OK;
						memdelete(file);
					}
				}
				data["done"] = done;
			}
		}
		return super::resolve(data);
	}

	Dictionary _resolveDocData(const DocData* p_doc) {
		Dictionary result;
		if(p_doc) {
			result["version"] = p_doc->version;

			auto parseArg = [](const DocData::ArgumentDoc& arg) -> Dictionary {
				Dictionary argd;
				argd["name"] = arg.name;
				argd["type"] = arg.type;
				argd["default_value"] = arg.default_value;
				return argd;
			};

			auto parseConstant = [](const DocData::ConstantDoc& c) -> Dictionary {
				Dictionary cd;
				cd["name"] = c.name;
				cd["value"] = c.value;
				cd["description"] = c.description;
				return cd;
			};

			auto parseProperty = [](const DocData::PropertyDoc& p) -> Dictionary {
				Dictionary pd;
				pd["name"] = p.name;
				pd["type"] = p.type;
				pd["description"] = p.description;
				return pd;
			};

			auto parseMethod = [&parseArg](const DocData::MethodDoc& m) -> Dictionary {
				Dictionary md;
				md["name"] = m.name;
				md["return_type"] = m.return_type;
				md["qualifiers"] = m.qualifiers;
				md["description"] = m.description;
				Vector<Variant> arguments;
				for(int i=0; i< m.arguments.size(); ++i)
					arguments.push_back(parseArg(m.arguments[i]));
				md["arguments"] = arguments;
				return md;
			};

			auto parseClass = [&parseMethod, &parseConstant, &parseProperty](const DocData::ClassDoc& c) -> Dictionary {
				Dictionary cd;
				cd["name"] = c.name;
				cd["inherits"] = c.inherits;
				cd["category"] = c.category;
				cd["brief_description"] = c.brief_description;
				cd["description"] = c.description;

				Vector<Variant> methods;
				for(int i=0; i< c.methods.size(); ++i)
					methods.push_back(parseMethod(c.methods[i]));
				cd["methods"] = methods;

				Vector<Variant> signals;
				for(int i=0; i< c.signals.size(); ++i)
					signals.push_back(parseMethod(c.signals[i]));
				cd["signals"] = signals;

				Vector<Variant> constants;
				for(int i=0; i< c.constants.size(); ++i)
					constants.push_back(parseConstant(c.constants[i]));
				cd["constants"] = constants;

				Vector<Variant> properties;
				for(int i=0; i< c.properties.size(); ++i)
					properties.push_back(parseProperty(c.properties[i]));
				cd["properties"] = properties;

				Vector<Variant> theme_properties;
				for(int i=0; i< c.theme_properties.size(); ++i)
					theme_properties.push_back(parseProperty(c.theme_properties[i]));
				cd["theme_properties"] = theme_properties;

				return cd;
			};

			Dictionary classes = Dictionary();
			for(Map<String,DocData::ClassDoc>::Element* E= p_doc->class_list.front();E;E=E->next())
				classes[E->key()] = parseClass(E->value());
			result["classes"] = classes;
		}
		return result;
	}

}
