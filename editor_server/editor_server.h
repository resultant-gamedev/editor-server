#ifndef GD_EXPLORER_EDITORSERVER_H
#define GD_EXPLORER_EDITORSERVER_H

#include <core/object.h>
#include <os/thread.h>
#include <io/tcp_server.h>
#include "services/service.h"
#include <map>

namespace gdexplorer {

	class EditorServer : public Object {
		GDCLASS(EditorServer, Object);

		enum Command {
			CMD_NONE,
			CMD_ACTIVATE,
			CMD_STOP,
		};

		struct ClientData {
			Thread *thread;
			Ref<StreamPeerTCP> connection;
			EditorServer *server;
			bool quit;
		};

		enum Method {
			METHOD_GET = 0,
			METHOD_HEAD,
			METHOD_POST,
			METHOD_PUT,
			METHOD_DELETE,
			METHOD_OPTIONS,
			METHOD_TRACE,
			METHOD_CONNECT,
			METHOD_MAX
		};

		struct Response {
			Map<String, String> header;
			String status;

			void set_header(const String& p_name, const String& p_value) {
				header.insert(p_name.strip_edges().to_lower(), p_value.strip_edges());
			}
		};

		struct Request {
			ClientData *cd;
			Response response;

			Method method;
			String url;
			String protocol;
			Map<String, String> header;
			int body_size;

			String read_utf8_body() {
				return cd->connection->get_utf8_string(body_size);
			}

			void send_response(const String& p_body=String()) {
				String resp = "HTTP/1.1 " + response.status + "\r\n";
				resp += "server: Godot Editor Server\r\n";

				for (Map<String, String>::Element *E=response.header.front(); E; E=E->next()) {
					resp += E->key() + ": " + E->value() + "\r\n";
				}

				if (!response.header.has("connection"))
					resp += "connection: " + header["connection"] + "\r\n";

				resp += "content-length: " + itos(p_body.length()) + "\r\n";
				resp += "\r\n";
				resp += p_body;

				CharString utf = resp.utf8();
				cd->connection->put_data((const uint8_t*)utf.get_data(), utf.length());
			}

			Request(ClientData *cd) {
				this->cd = cd;
			}
		};

	private:
		std::map<String, Ref<EditorServerService>> services;
		Ref<TCP_Server> server;
		Set<Thread*> to_wait;
		Mutex *wait_mutex;
		Thread *thread;
		Command cmd;
		bool quit;
		int port;
		bool active;
	private:
		static void _close_client(ClientData *cd);
		static void _subthread_start(void *s);
		static void _thread_start(void *s);

	protected:
		static void _bind_methods();

	public:
		void start(int port);
		void stop();
		bool is_active() const { return active; }
		int get_port() const { return port; }
		void register_service(const String& action, const Ref<EditorServerService>& service);
		EditorServer();
		~EditorServer();
	};
}

#endif // GD_EXPLORER_EDITORSERVER_H
