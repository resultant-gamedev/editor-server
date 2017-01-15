#include "editor_server.h"
#include <os/file_access.h>
#include <core/globals.h>
#include <core/io/json.h>
#include <tools/editor/editor_settings.h>

#define CLOSE_CLIENT_COND(m_cond, m_cd) \
{ if ( m_cond ) {	\
	_close_client(m_cd);	\
	return;	 \
	} }	\

namespace gdexplorer {

	void EditorServer::_close_client(EditorServer::ClientData *cd) {
		cd->connection->disconnect_from_host();
		cd->server->wait_mutex->lock();
		cd->server->to_wait.insert(cd->thread);
		cd->server->wait_mutex->unlock();
		memdelete(cd);
	}

	void EditorServer::_subthread_start(void *s) {
		ClientData *cd = (ClientData*)s;
		cd->connection->set_nodelay(true);

		static const char* _methods[METHOD_MAX]={
			"GET",
			"HEAD",
			"POST",
			"PUT",
			"DELETE",
			"OPTIONS",
			"TRACE",
			"CONNECT"
		};

		Vector<uint8_t> request_str;

		while(!cd->quit) {
			uint8_t byte;
			Error err = cd->connection->get_data(&byte, 1);
			CLOSE_CLIENT_COND(err!=OK, cd);

			request_str.push_back(byte);
			int rs = request_str.size();
			// Read request content from connection done
			if ((rs>=2 && request_str[rs-2]=='\n' && request_str[rs-1]=='\n') ||
					(rs>=4 && request_str[rs-4]=='\r' && request_str[rs-3]=='\n' && rs>=4 && request_str[rs-2]=='\r' && request_str[rs-1]=='\n')) {

				request_str.push_back(0);

				// End of request header, parse
				String header;
				CLOSE_CLIENT_COND(header.parse_utf8((const char*)request_str.ptr()), cd);
				Vector<String> header_lines = header.split("\n");
				request_str.clear();

				Request request(cd);

				for (int i = 0; i < header_lines.size(); i++) {
					String s = header_lines[i].strip_edges();
					if (s.length() == 0)
						continue;

					if (i == 0) {
						// Parse command, url and protocol
						Vector<String> parts = s.split(" ");
						CLOSE_CLIENT_COND(parts.size() < 3, cd);

						int method_idx = -1;
						for (int j = 0; j < METHOD_MAX; j++) {
							if (_methods[j] == parts[0]) {
								method_idx = j;
								break;
							}
						}
						CLOSE_CLIENT_COND(method_idx < 0, cd);

						request.method = (Method) method_idx;
						request.url = parts[1];
						request.protocol = parts[2];
					} else {
						int sep=s.find(":");
						if (sep<0)
							sep=s.length();
						String field_name = s.substr(0,sep).strip_edges().to_lower();
						String field_value = s.substr(sep+1,s.length()).strip_edges();

						if (field_name.empty() || field_value.empty())
							continue;

						if (field_name == "content-length") {
							request.body_size = field_value.to_int();
						}

						request.header.insert(field_name, field_value);
					}
				}

				switch (request.method) {
					case METHOD_POST: {
							if (request.body_size == 0) {
								// No content... ignore request
								continue;
							}

							// Read and parse body as json
							Variant _data;
							String errmsg;
							int errline = -1;
							Error parse_err = JSON::parse(request.read_utf8_body(), _data, errmsg, errline);
							if (parse_err != OK) {
								request.response.status = "400 Bad Request";
								request.response.set_header("Accept", "application/json");
								request.response.set_header("Accept-Charset", "utf-8");
								request.send_response();
								break;
							}
							Dictionary data = _data;
							if (!data.has("action")) {
								data["error"] = "No action found in the request body";
							}
							else {
								auto services = cd->server->services;
								if(services.find(data["action"]) == services.end())
									data["error"] = "No service found for the action";
								else {
									Ref<EditorServerService>& service = services[data["action"]];
									if(!service.is_null()) {
										data = service->resolve(data);
									}
								}
							}


							// Done! Deliver <3
							request.response.status = "200 OK";
							request.response.set_header("Content-Type", "application/json; charset=UTF-8");
							request.send_response(JSON::print(data));

						} break;
					default: {
							request.response.status = "405 Method Not Allowed";
							request.response.set_header("Allow", "POST");
							request.send_response();
						} break;
				}

				CLOSE_CLIENT_COND(request.header["connection"] != "keep-alive", cd);
			}
		}

		_close_client(cd);
	}

	void EditorServer::_thread_start(void *s) {
		EditorServer *self = (EditorServer*)s;

		while(!self->quit) {
			if (self->cmd == CMD_ACTIVATE) {
				self->server->stop();
				self->active = false;
				self->cmd = CMD_NONE;
				if (self->server->listen(self->port) == OK) {
					self->active = true;
					print_line(String("[Editor Server]Server port started at:") + itos(self->port));
				}
				else {
					ERR_PRINTS(String("[Editor Server]Error open port: ") + itos(self->port));
				}
			}
			else if (self->cmd == CMD_STOP) {
				self->server->stop();
				self->active = false;
				self->cmd = CMD_NONE;
			}

			if (self->active && self->server->is_connection_available()) {
				ClientData *cd = memnew( ClientData );
				cd->connection = self->server->take_connection();
				cd->server = self;
				cd->quit = false;
				cd->thread = Thread::create(_subthread_start, cd);
			}

			self->wait_mutex->lock();
			while (self->to_wait.size()) {
				Thread *w = self->to_wait.front()->get();
				self->to_wait.erase(w);
				Thread::wait_to_finish(w);
				if(w)
					memdelete(w);
				self->wait_mutex->lock();
			}
			self->wait_mutex->unlock();

			OS::get_singleton()->delay_usec(50000);
		}
	}

	void EditorServer::_bind_methods() {
		ClassDB::bind_method(_MD("register_service", "action:String", "service:EditorServerService"), &EditorServer::register_service);
	}

	void EditorServer::start(int port) {
		this->port = port;
		cmd = CMD_ACTIVATE;
	}

	void EditorServer::stop() {
		cmd = CMD_STOP;
	}

	void EditorServer::register_service(const String &action, const Ref<EditorServerService>& service) {
		if(action.length()) {
			services[action] = service;
		}
	}

	EditorServer::EditorServer() {
		server = TCP_Server::create_ref();
		wait_mutex = Mutex::create();
		quit = false;
		active = false;
		cmd = CMD_NONE;
		port = 6570;
		thread = Thread::create(_thread_start, this);
	}

	EditorServer::~EditorServer() {
		quit = true;
		Thread::wait_to_finish(thread);
		memdelete(thread);
		memdelete(wait_mutex);
		services.clear();
	}

}

