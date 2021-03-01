//
//  core_workload.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/9/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#ifndef YCSB_C_CORE_WORKLOAD_H_
#define YCSB_C_CORE_WORKLOAD_H_

#include <vector>
#include <string>
#include "ycsbdb.h"
#include "properties.h"

#ifdef NEW_GENERATORS
#include "Generator.hpp"
#include "DiscreteGenerator.hpp"
#include "CounterGenerator.hpp"
#include "NumberGenerator.hpp"
#include "AcknowledgedCounterGenerator.hpp"
#else
#include "generator.h"
#include "discrete_generator.h"
#include "counter_generator.h"
#endif

#include "utils.h"

namespace ycsbc
{
enum Operation { INSERT, READ, UPDATE, SCAN, READMODIFYWRITE };

class CoreWorkload {
    public:
	///
	/// The max execution time in seconds
	///
	static const std::string MAX_EXECUTION_TIME_PROPERTY;
	static const std::string MAX_EXECUTION_TIME_DEFAULT;

	///
	/// The print status interval in seconds
	///
	static const std::string STATUS_INTERVAL_PROPERTY;
	static const std::string STATUS_INTERVAL_DEFAULT;

	///
	/// The name of the database table to run queries against.
	///
	static const std::string TABLENAME_PROPERTY;
	static const std::string TABLENAME_DEFAULT;

	///
	/// The name of the property for the number of fields in a record.
	///
	static const std::string FIELD_COUNT_PROPERTY;
	static const std::string FIELD_COUNT_DEFAULT;

	///
	/// The name of the property for the field length distribution.
	/// Options are "uniform", "zipfian" (favoring short records), and "constant".
	///
	static const std::string FIELD_LENGTH_DISTRIBUTION_PROPERTY;
	static const std::string FIELD_LENGTH_DISTRIBUTION_DEFAULT;

	///
	/// The name of the property for the length of a field in bytes.
	///
	static const std::string FIELD_LENGTH_PROPERTY;
	static const std::string FIELD_LENGTH_DEFAULT;

	///
	/// The name of the property for deciding whether to read one field (false)
	/// or all fields (true) of a record.
	///
	static const std::string READ_ALL_FIELDS_PROPERTY;
	static const std::string READ_ALL_FIELDS_DEFAULT;

	///
	/// The name of the property for deciding whether to write one field (false)
	/// or all fields (true) of a record.
	///
	static const std::string WRITE_ALL_FIELDS_PROPERTY;
	static const std::string WRITE_ALL_FIELDS_DEFAULT;

	///
	/// The name of the property for the proportion of read transactions.
	///
	static const std::string READ_PROPORTION_PROPERTY;
	static const std::string READ_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of update transactions.
	///
	static const std::string UPDATE_PROPORTION_PROPERTY;
	static const std::string UPDATE_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of insert transactions.
	///
	static const std::string INSERT_PROPORTION_PROPERTY;
	static const std::string INSERT_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of scan transactions.
	///
	static const std::string SCAN_PROPORTION_PROPERTY;
	static const std::string SCAN_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of
	/// read-modify-write transactions.
	///
	static const std::string READMODIFYWRITE_PROPORTION_PROPERTY;
	static const std::string READMODIFYWRITE_PROPORTION_DEFAULT;

	///
	/// The name of the property for the the distribution of request keys.
	/// Options are "uniform", "zipfian" and "latest".
	///
	static const std::string REQUEST_DISTRIBUTION_PROPERTY;
	static const std::string REQUEST_DISTRIBUTION_DEFAULT;

	///
	/// The name of the property for the max scan length (number of records).
	///
	static const std::string MAX_SCAN_LENGTH_PROPERTY;
	static const std::string MAX_SCAN_LENGTH_DEFAULT;

	///
	/// The name of the property for the scan length distribution.
	/// Options are "uniform" and "zipfian" (favoring short scans).
	///
	static const std::string SCAN_LENGTH_DISTRIBUTION_PROPERTY;
	static const std::string SCAN_LENGTH_DISTRIBUTION_DEFAULT;

	///
	/// The name of the property for the order to insert records.
	/// Options are "ordered" or "hashed".
	///
	static const std::string INSERT_ORDER_PROPERTY;
	static const std::string INSERT_ORDER_DEFAULT;

	static const std::string INSERT_START_PROPERTY;
	static const std::string INSERT_START_DEFAULT;

	static const std::string RECORD_COUNT_PROPERTY;
	static const std::string OPERATION_COUNT_PROPERTY;

	static const std::string DISTRIBUTED_SETUP_PROPERTY;
	static const std::string DISTRIBUTED_SETUP_DEFAULT;

	static const std::string NUM_OF_CLIENTS_PROPERTY;
	static const std::string NUM_OF_CLIENTS_DEFAULT;

	static const std::string PARALLEL_LOADING_PROPERTY;
	static const std::string PARALLEL_LOADING_DEFAULT;

	static const std::string ZOOKEEPER_SERVER_PROPERTY;
	static const std::string ZOOKEEPER_SERVER_DEFAULT;

	///
	/// Initialize the scenario.
	/// Called once, in the main client thread, before any operations are started.
	///
	virtual void Init(const utils::Properties &p);

	virtual void BuildValues(std::vector<ycsbc::YCSBDB::KVPair> &values);
	virtual void BuildUpdate(std::vector<ycsbc::YCSBDB::KVPair> &update);

	virtual std::string NextTable()
	{
		return table_name_;
	}
	virtual std::string NextSequenceKey(); /// Used for loading data
	virtual std::string NextTransactionKey(); /// Used for transactions

	virtual Operation NextOperation()
	{
#ifdef NEW_GENERATORS
		return op_chooser_.nextValue();
#else
		return op_chooser_.Next();
#endif
	}

	virtual std::string NextFieldName();

	virtual size_t NextScanLength()
	{
#ifdef NEW_GENERATORS
		return scan_len_chooser_->nextValue();
#else
		return scan_len_chooser_->Next();
#endif
	}

	bool read_all_fields() const
	{
		return read_all_fields_;
	}
	bool write_all_fields() const
	{
		return write_all_fields_;
	}

	CoreWorkload()
		: field_count_(0), read_all_fields_(false), write_all_fields_(false), field_len_generator_(NULL),
		  key_generator_(NULL), key_chooser_(NULL), field_chooser_(NULL), scan_len_chooser_(NULL),
#ifndef NEW_GENERATORS
		  insert_key_sequence_(3),
#endif
		  ordered_inserts_(true), record_count_(0)
	{
	}

	virtual ~CoreWorkload()
	{
		if (field_len_generator_)
			delete field_len_generator_;
		if (key_generator_)
			delete key_generator_;
		if (key_chooser_)
			delete key_chooser_;
#ifdef NEW_GENERATORS
		if (transaction_insert_key_sequence_)
			delete transaction_insert_key_sequence_;
#endif
		if (field_chooser_)
			delete field_chooser_;
		if (scan_len_chooser_)
			delete scan_len_chooser_;
	}

    public:
	static Generator<uint64_t> *GetFieldLenGenerator(const utils::Properties &p);
	std::string BuildKeyName(uint64_t key_num);

	std::string table_name_;
	int field_count_;
	bool distributedsetup_;
	bool parallelloading_;
	std::string zookeeperserver_;
	bool read_all_fields_;
	bool write_all_fields_;
	Generator<uint64_t> *field_len_generator_;
	Generator<uint64_t> *key_generator_;
	DiscreteGenerator<Operation> op_chooser_;
#ifdef NEW_GENERATORS
	NumberGenerator *key_chooser_;
#else
	Generator<uint64_t> *key_chooser_;
#endif
	Generator<uint64_t> *field_chooser_;
	Generator<uint64_t> *scan_len_chooser_;
#ifdef NEW_GENERATORS
	AcknowledgedCounterGenerator *transaction_insert_key_sequence_;
#else
	CounterGenerator insert_key_sequence_;
#endif
	bool ordered_inserts_;
	size_t record_count_;
};

inline std::string CoreWorkload::NextSequenceKey()
{
#ifdef NEW_GENERATORS
	uint64_t key_num = key_generator_->nextValue();
#else
	uint64_t key_num = key_generator_->Next();
#endif
	return BuildKeyName(key_num);
}

inline std::string CoreWorkload::NextTransactionKey()
{
	uint64_t key_num;

	do {
#ifdef NEW_GENERATORS
		key_num = key_chooser_->nextValue();
	} while (key_num > transaction_insert_key_sequence_->lastValue());
#else
		key_num = key_chooser_->Next();
	} while (key_num > insert_key_sequence_.Last());
#endif

	return BuildKeyName(key_num);
}

inline std::string CoreWorkload::BuildKeyName(uint64_t key_num)
{
	if (!ordered_inserts_) {
		key_num = utils::Hash(key_num);
	}
	return std::to_string(key_num);
	//vanilla
	//return std::string("user").append(std::to_string(key_num));
}

inline std::string CoreWorkload::NextFieldName()
{
#ifdef NEW_GENERATORS
	return std::string("field").append(std::to_string(field_chooser_->nextValue()));
#else
	return std::string("field").append(std::to_string(field_chooser_->Next()));
#endif
}

} // namespace ycsbc

#endif // YCSB_C_CORE_WORKLOAD_H_
