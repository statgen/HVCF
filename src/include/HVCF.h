#ifndef SRC_HVCF_H_
#define SRC_HVCF_H_

#include <iostream>
#include <limits>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <iterator>
#include <algorithm>
#include <chrono>
#include "hdf5.h"
#include "HVCFOpenException.h"
#include "HVCFCloseException.h"
#include "HVCFWriteException.h"
#include "HVCFReadException.h"
#include "HDF5FileIdentifier.h"
#include "HDF5GroupIdentifier.h"
#include "HDF5DatasetIdentifier.h"
#include "HDF5DatatypeIdentifier.h"
#include "HDF5DataspaceIdentifier.h"
#include "HDF5PropertyIdentifier.h"
#include "../../../auxc/MiniVCF/src/include/VCFReader.h"
#include "WriteBuffer.h"

using namespace std;

namespace sph_umich_edu {

class HVCF {
private:
	string name;

	HDF5FileIdentifier file_id;
	HDF5GroupIdentifier samples_group_id;
	HDF5GroupIdentifier variants_group_id;
	HDF5DatasetIdentifier samples_all_dataset_id;
	HDF5DatatypeIdentifier native_string_datatype_id;

	static constexpr char SAMPLES_GROUP[] = "samples";
	static constexpr char VARIANTS_GROUP[] = "variants";
	static constexpr char SAMPLES_ALL_DATASET[] = "ALL";
	static constexpr char HAPLOTYPES_DATASET[] = "haplotypes";
	static constexpr char VARIANT_NAMES_DATASET[] = "names";
	static constexpr char VARIANT_POSITIONS_DATASET[] = "positions";

	hid_t create_strings_1D_dataset(const string& name, hid_t group_id, hsize_t chunk_size) throw (HVCFWriteException);
	hid_t create_hsize_1D_dataset(const string& name, hid_t group_id, hsize_t chunk_size) throw (HVCFWriteException);
	hid_t create_ull_1D_dataset(const string& name, hid_t group_id, hsize_t chunk_size) throw (HVCFWriteException);
	hid_t create_haplotypes_2D_dataset(const string& name, hid_t group_id, hsize_t variants_chunk_size, hsize_t samples_chunk_size) throw (HVCFWriteException);
	hid_t create_chromosome_group(const string& name) throw (HVCFWriteException);
	void write_haplotypes(hid_t group_id, const unsigned char* buffer, unsigned int n_variants, unsigned int n_haplotypes) throw (HVCFWriteException);
	void write_names(hid_t group_id, char* const* buffer, unsigned int n_variants) throw (HVCFWriteException);
	void write_positions(hid_t group_id, const unsigned long long int* buffer, unsigned int n_variants) throw (HVCFWriteException);

	unsigned long long int read_position(hid_t group_id, hsize_t index) throw (HVCFReadException);

	unordered_map<string, unique_ptr<HDF5GroupIdentifier>> chromosomes;
	unordered_map<string, unique_ptr<WriteBuffer>> write_buffers;

public:
	HVCF();
	virtual ~HVCF();

	void create(const string& name) throw (HVCFWriteException);
	void set_samples(const vector<string>& samples) throw (HVCFWriteException);
	void set_population(const string& name, const vector<string>& samples) throw (HVCFWriteException);
	void write_variant(const Variant& variant) throw (HVCFWriteException);
	void flush_write_buffer() throw (HVCFWriteException);

	hsize_t get_n_samples() throw (HVCFReadException);
	vector<string> get_samples() throw (HVCFReadException);
	vector<string> get_population(const string& name) throw (HVCFReadException);
	hsize_t get_n_variants() throw (HVCFReadException);
	hsize_t get_n_variants(const string& chromosome) throw (HVCFReadException);

	int get_variant_index_by_pos(const string& chromosome, unsigned long long int position) throw (HVCFReadException);

	void open(const string& name) throw (HVCFOpenException);

	void close() throw (HVCFCloseException);

	unsigned int get_n_opened_objects() const;
	static unsigned int get_n_all_opened_objects();
};

}

#endif
