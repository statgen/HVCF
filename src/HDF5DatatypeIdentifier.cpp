#include "include/HDF5DatatypeIdentifier.h"

namespace sph_umich_edu {

HDF5DatatypeIdentifier::HDF5DatatypeIdentifier() {

}

HDF5DatatypeIdentifier::~HDF5DatatypeIdentifier() noexcept {
	try {
		close();
	} catch (std::exception &e) {
		// do not propagate any exceptions
	}
}

void HDF5DatatypeIdentifier::close() throw (HVCFException) {
	if (identifier >= 0) {
		if (H5Tclose(identifier) < 0) {
			throw HVCFException(__FILE__, __FUNCTION__, __LINE__, "Error while closing HDF5 datatype identifier.");
		}
		this->identifier = numeric_limits<hid_t>::min();
	}
}

}
