#ifndef HETEROGENEOUSTABLE_H
#define HETEROGENEOUSTABLE_H

#include <unordered_map>
#include <memory>
#include <cassert>

template<class KeyType>
struct TableQueryCallback
{
	virtual void ReportRow(KeyType key, void *value) = 0;
};

template<class KeyType>
class GenericTable
{
public:
	virtual ~GenericTable() {}
	virtual void *AddRow(KeyType key) = 0;
	virtual void *AddRow(KeyType key, void *value) = 0;
	virtual void RemoveRow(KeyType key) = 0;
	virtual void *GetRow(KeyType key) = 0;
	virtual bool HasRow(KeyType key) = 0;
	virtual void Query(TableQueryCallback<KeyType> *callback) = 0;
};

template<class KeyType, class ValueType>
class Table : public GenericTable<KeyType>
{
public:
	virtual void *AddRow(KeyType key) override
	{
		auto &result = _rows.insert(
			std::make_pair(key, ValueType()));

		assert(result.second);
		return &(*result.first).second;
	}

	virtual void *AddRow(KeyType key, void *value) override
	{
		auto &result = _rows.insert(
			std::make_pair(key, *reinterpret_cast<ValueType*>(value)));

		assert(result.second);
		return &(*result.first).second;
	}

	virtual void RemoveRow(KeyType key) override
	{
		_rows.erase(key);
	}

	virtual void *GetRow(KeyType key) override
	{
		return &_rows.at(key);
	}

	virtual bool HasRow(KeyType key) override
	{
		return _rows.count(key) > 0;
	}

	virtual void Query(TableQueryCallback<KeyType> *callback) override
	{
		for(auto &row : _rows)
			callback->ReportRow(row.first, &row.second);
	}

private:
	std::unordered_map<KeyType, ValueType> _rows;
};

template<class KeyType>
class HeterogeneousTable
{
public:
	template<class ValueType>
	ValueType &AddRow(KeyType key)
	{
		std::unique_ptr<GenericTable<KeyType> > &table = GetTable<ValueType>();
		return *reinterpret_cast<ValueType*>(table->AddRow(key));
	}

	template<class ValueType>
	ValueType &AddRow(KeyType key, ValueType value)
	{
		std::unique_ptr<GenericTable<KeyType> > &table = GetTable<ValueType>();
		return *reinterpret_cast<ValueType*>(table->AddRow(key, &value));
	}

	template<class ValueType>
	void RemoveRow(KeyType key)
	{
		std::unique_ptr<GenericTable<KeyType> > &table = GetTable<ValueType>();
		table->RemoveRow(key);
	}

	template<class ValueType>
	ValueType &GetRow(KeyType key)
	{
		std::unique_ptr<GenericTable<KeyType> > &table = GetTable<ValueType>();
		return *reinterpret_cast<ValueType*>(table->GetRow(key));
	}

	template<class ValueType>
	bool HasRow(KeyType key)
	{
		std::unique_ptr<GenericTable<KeyType> > &table = GetTable<ValueType>();
		return table->HasRow(key);
	}

	// Callback signature: 
	// void (KeyType key, <ValueType value> | <ValueType &value>); 
	template<class ValueType, class Func>
	void Query(Func func)
	{
		struct Callback : public TableQueryCallback<KeyType>
		{
			Func &_func;

			Callback(Func &func) : _func(func) {}

			virtual void ReportRow(KeyType key, void *value) override
			{
				_func(key, *reinterpret_cast<ValueType*>(value));
			}
		} callback(func);

		std::unique_ptr<GenericTable<KeyType> > &table = GetTable<ValueType>();
		table->Query(&callback);
	}

private:
	template<class ValueType>
	std::unique_ptr<GenericTable<KeyType> > &GetTable()
	{
		std::unique_ptr<GenericTable<KeyType> > &table =
			_tables[typeid(ValueType).hash_code()];

		if(!table)
			table = std::unique_ptr<GenericTable<KeyType> >(new Table<KeyType, ValueType>);

		return table;
	}

	std::unordered_map<size_t, std::unique_ptr<GenericTable<KeyType> > > _tables;
};

#endif // HETEROGENEOUSTABLE_H