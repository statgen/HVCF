#ifndef SRC_HDF5DATATYPEIDENTIFIER_H_
#define SRC_HDF5DATATYPEIDENTIFIER_H_

#include "HDF5Identifier.h"

using namespace std;

namespace sph_umich_edu {

class HDF5DatatypeIdentifier: public HDF5Identifier {
public:
	HDF5DatatypeIdentifier();
	virtual ~HDF5DatatypeIdentifier() noexcept;

	using HDF5Identifier::operator=;

	void close() throw (HVCFException);
};

}

#endif
