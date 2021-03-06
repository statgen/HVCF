#ifndef SRC_HVCF_H_
#define SRC_HVCF_H_

#include <iostream>
#include <limits>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <iterator>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <future>

#define ARMA_NO_DEBUG
#include <armadillo>

#include "hdf5.h"

#include "Types.h"
#include "HVCFOpenException.h"
#include "HVCFCloseException.h"
#include "HVCFWriteException.h"
#include "HVCFReadException.h"
#include "HVCFCreateException.h"
#include "HDF5FileIdentifier.h"
#include "HDF5GroupIdentifier.h"
#include "HDF5DatasetIdentifier.h"
#include "HDF5DatatypeIdentifier.h"
#include "HDF5DataspaceIdentifier.h"
#include "HDF5PropertyIdentifier.h"
#include "HVCFConfiguration.h"
#include "../../../auxc/MiniVCF/src/include/VCFReader.h"
#include "WriteBuffer.h"
#include "../blosc/blosc_filter.h"

using namespace std;
using namespace arma;

namespace sph_umich_edu {

class HVCF {
private:
	string name;

	HDF5FileIdentifier file_id;
	HDF5GroupIdentifier samples_group_id;
	HDF5GroupIdentifier chromosomes_group_id;
	HDF5DatatypeIdentifier native_string_datatype_id;
	HDF5DatatypeIdentifier hash_index_entry_memory_datatype_id;
	HDF5DatatypeIdentifier string_index_entry_memory_datatype_id;
	HDF5DatatypeIdentifier interval_index_entry_memory_datatype_id;
	HDF5DatatypeIdentifier ull_index_entry_memory_datatype_id;
	HDF5DatatypeIdentifier variants_entry_memory_datatype_id;

	unsigned int N_VARIANTS_HASH_BUCKETS;
	unsigned int N_SAMPLES_HASH_BUCKETS;
	unsigned int MAX_VARIANTS_IN_INTERVAL_BUCKET;
	unsigned int VARIANTS_CHUNK_SIZE;
	unsigned int SAMPLES_CHUNK_SIZE;
	const char* COMPRESSION;
	unsigned int COMPRESSION_LEVEL;
	size_t METADATA_CACHE_INITIAL_SIZE;
	size_t METADATA_CACHE_MIN_SIZE;
	size_t METADATA_CACHE_MAX_SIZE;
	size_t SIEVE_BUFFER_MAX_SIZE;
	size_t CHUNK_CACHE_N_SLOTS;
	size_t CHUNK_CACHE_SIZE;

	static constexpr char CHROMOSOMES_GROUP[] = "chromosomes";
	static constexpr char SAMPLES_GROUP[] = "samples";
	static constexpr char VARIANTS_DATASET[] = "variants";
	static constexpr char HAPLOTYPES_DATASET[] = "haplotypes";
	static constexpr char SAMPLE_NAMES_DATASET[] = "names";
	static constexpr char SAMPLE_SUBSETS_DATASET[] = "subsets";

	static constexpr char VARIABLE_LENGTH_STRING_TYPE[] = "variable_length_string_type";
	static constexpr char VARIANTS_ENTRY_TYPE[] = "variants_entry_type";
	static constexpr char SUBSETS_ENTRY_TYPE[] = "subsets_entry_type";
	static constexpr char STRING_INDEX_ENTRY_TYPE[] = "string_index_entry_type";
	static constexpr char INTERVAL_INDEX_ENTRY_TYPE[] = "interval_index_entry_type";
	static constexpr char HASH_INDEX_ENTRY_TYPE[] = "hash_index_entry_type";
	static constexpr char ULL_INDEX_ENTRY_TYPE[] = "ull_index_entry_type";
	static constexpr char NAMES_INDEX_GROUP[] = "names_index";
	static constexpr char INTERVALS_INDEX_GROUP[] = "intervals_index";
	static constexpr char INTERVALS_INDEX[] = "intervals";
	static constexpr char HASH_INDEX[] = "hashes";
	static constexpr char INDEX_BUCKETS[] = "buckets";

	unordered_map<string, unique_ptr<HDF5GroupIdentifier>> chromosomes;
	unordered_map<string, unique_ptr<WriteBuffer>> write_buffers;

	samples_cache_entry samples_cache;
	unordered_map<string, unique_ptr<chromosomes_cache_entry>> chromosomes_cache;

	hid_t create_variants_entry_memory_datatype() throw (HVCFCreateException);
	hid_t create_subsets_entry_memory_datatype() throw (HVCFCreateException);
	hid_t create_ull_index_entry_memory_datatype() throw (HVCFCreateException);
	hid_t create_string_index_entry_memory_datatype() throw (HVCFCreateException);
	hid_t create_interval_index_entry_memory_datatype() throw (HVCFCreateException);
	hid_t create_hash_index_entry_memory_datatype() throw (HVCFCreateException);

	hid_t create_sample_names_dataset(hid_t group_id, hsize_t chunk_size) throw (HVCFWriteException);
	hid_t create_sample_subsets_dataset(hid_t group_id, hsize_t chunk_size) throw (HVCFWriteException);
	hid_t create_haplotypes_dataset(hid_t group_id, hsize_t variants_chunk_size, hsize_t samples_chunk_size) throw (HVCFWriteException);
	hid_t create_variants_dataset(hid_t group_id, hsize_t chunk_size) throw (HVCFWriteException);
	hid_t create_chromosome_group(const string& name) throw (HVCFWriteException);

	void initialize_ull_index_buckets(hid_t chromosome_group_id, const char* index_group_name) throw (HVCFWriteException);
	void initialize_string_index_buckets(hid_t chromosome_group_id, const char* index_group_name) throw (HVCFWriteException);

	void cache_names_index_bucket(hid_t group_id, hash_index_entry_type& hash_index_entry, vector<string_index_entry_type>& bucket, vector<string_index_entry_type>& buckets_cache) throw (HVCFWriteException);
	void write_names_index_buckets(hid_t group_id, vector<string_index_entry_type>& buckets_cache) throw (HVCFWriteException);
	void write_names_index(hid_t chromosome_group_id, const hash_index_entry_type* hash_index_entries, unsigned int n_hash_index_entries) throw (HVCFWriteException);

	void cache_intervals_index_bucket(hid_t group_id, interval_index_entry_type& interval_index_entry, vector<ull_index_entry_type>& bucket, vector<ull_index_entry_type>& buckets_cache) throw (HVCFWriteException);
	void write_intervals_index_buckets(hid_t group_id, vector<ull_index_entry_type>& buckets_cache) throw (HVCFWriteException);
	void write_intervals_index(hid_t chromosome_group_id, const interval_index_entry_type* interval_index_entries, unsigned int n_interval_index_entries) throw (HVCFWriteException);

	void read_variant_names_into_bucket(hid_t chromosome_group_id, const vector<hsize_t>& offsets, vector<string_index_entry_type>& bucket) throw (HVCFWriteException);
	void read_sample_names_into_bucket(hid_t samples_group_id, const vector<hsize_t>& offsets, vector<string_index_entry_type>& bucket) throw (HVCFWriteException);
	void read_positions_into_bucket(hid_t chromosome_group_id, const vector<hsize_t>& offsets, vector<ull_index_entry_type>& bucket) throw (HVCFWriteException);

	void write_haplotypes(hid_t group_id, const unsigned char* buffer, unsigned int n_variants, unsigned int n_haplotypes) throw (HVCFWriteException);
	void write_variants(hid_t group_id, const variants_entry_type* buffer, unsigned int n_variants) throw (HVCFWriteException);

	void create_chromosome_indices(hid_t chromosome_group_id) throw (HVCFWriteException);
	void create_samples_indices() throw (HVCFWriteException);
	void create_indices() throw (HVCFWriteException);

	void write_samples(const vector<string>& samples) throw (HVCFWriteException);
	void write_variant(const Variant& variant, future<void>& async_write) throw (HVCFWriteException);
	void flush_write_buffer(future<void>& async_write) throw (HVCFWriteException);

	void load_samples_cache() throw (HVCFReadException);
	void load_chromosomes_cache() throw (HVCFReadException);
	void load_cache() throw (HVCFReadException);
public:
	HVCF();
	HVCF(const HVCFConfiguration& configuration);
	virtual ~HVCF() noexcept;

	void create(const string& name) throw (HVCFWriteException);
	void open(const string& name) throw (HVCFOpenException);
	void close() throw (HVCFCloseException);

	void import_vcf(const string& name) throw (HVCFWriteException);

	void create_sample_subset(const string& name, const vector<string>& samples) throw (HVCFWriteException);

	hsize_t get_n_samples() throw (HVCFReadException);
	vector<string> get_samples() throw (HVCFReadException);
	unsigned int get_n_sample_subsets() throw (HVCFReadException);
	vector<string> get_sample_subsets() throw (HVCFReadException);
	unsigned int get_n_samples_in_subset(const string& name) throw (HVCFReadException);
	vector<string> get_samples_in_subset(const string& name) throw (HVCFReadException);
	unsigned int get_n_chromosomes() const;
	vector<string> get_chromosomes() const;
	bool has_chromosome(const string& chromosome) const;
	unsigned long long int get_chromosome_start(const string& chromosome) const throw (HVCFReadException);
	unsigned long long int get_chromosome_end(const string& chromosome) const throw (HVCFReadException);
	hsize_t get_n_variants() const throw (HVCFReadException);
	hsize_t get_n_variants_in_chromosome(const string& chromosome) const throw (HVCFReadException);

	long long int get_variant_offset_by_position_eq(const string& chromosome, unsigned long long int position) throw (HVCFReadException);
	long long int get_variant_offset_by_position_ge(const string& chromosome, unsigned long long int position) throw (HVCFReadException);
	long long int get_variant_offset_by_position_le(const string& chromosome, unsigned long long int position) throw (HVCFReadException);
	long long int get_variant_offset_by_name(const string& chromosome, const string& name) throw (HVCFReadException);
	long long int get_sample_offset(const string& name) throw (HVCFReadException);

	void compute_ld(const string& subset, const string& chromosome, unsigned long long int start_position, unsigned long long int end_position, vector<ld_query_result>& result) throw (HVCFReadException);
	void compute_ld(const string& subset, const string& chromosome, const string& lead_variant_name, unsigned long long int start_position, unsigned long long int end_position, vector<ld_query_result>& result) throw (HVCFReadException);
	void compute_frequencies(const string& subset, const string& chromosome, unsigned long long int start_position, unsigned long long int end_position, vector<frequency_query_result>& result) throw (HVCFReadException);
	void extract_variants(const string& chromosome, unsigned long long int start_position, unsigned long long int end_position, vector<variant_query_result>& result) throw (HVCFReadException);
	void extract_haplotypes(const string& subset, const string& chromosome, const string& variant_name, vector<variant_haplotypes_query_result>& result) throw (HVCFReadException);
	void extract_haplotypes(const string& sample, const string& chromosome, unsigned long long int start_position, unsigned long long int end_position, vector<sample_haplotypes_query_result>& result) throw (HVCFReadException);

	unsigned int get_n_opened_objects() const;
	static unsigned int get_n_all_opened_objects();

	void compute_ld_test(const string& subset, const string& chromosome, const string& lead_variant_name, unsigned long long int start_position, unsigned long long end_position, vector<ld_query_result>& result) throw (HVCFReadException);
};

}

#endif
