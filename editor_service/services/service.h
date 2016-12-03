#ifndef GD_EXPLORER_SERVICE_H
#define GD_EXPLORER_SERVICE_H

#include <core/dictionary.h>

namespace gdexplorer {

	struct Service {
		Service() {};
		virtual ~Service() {}
		virtual Dictionary resolve(const Dictionary& data) { return data; }
	};
}

#endif // GD_EXPLORER_SERVICE_H
