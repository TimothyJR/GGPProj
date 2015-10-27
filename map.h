#pragma once

#include <memory>
#include <stdint.h>
template <typename Value, uint32_t Capacity = 33u>
class map
{
public:
	map()
	{
		this->m_hash = std::unique_ptr<uint32_t[]>(new uint32_t[Capacity]);
		memset(this->m_hash.get(), 0, Capacity * sizeof(uint32_t));
		// get space for data, but don't construct them yet
		this->m_data = std::unique_ptr<Value[]>((Value*)malloc(Capacity * sizeof(Value)));
	}
	map(map&& rhs) {
		this->m_data = std::move(rhs.m_data);
		this->m_hash = std::move(rhs.m_hash);
	}
	map& operator =(map&& rhs) {
		this->m_data = std::move(rhs.m_data);
		this->m_hash = std::move(rhs.m_hash);

		return *this;
	}

	~map() {}

	void remove(const char* key) {
		auto hash_value = hash(key, std::strlen(key));
		auto hash_index = hash_value % Capacity;

		while (0 != this->m_hash[hash_index] && hash_value != this->m_hash[hash_index]) {
			hash_index++;
		}

		this->m_hash[hash_index] = 0;
		this->m_data[hash_index].~Value();
	}

	void insert(const char* key, Value& value) {
		auto hash_value = hash(key, std::strlen(key));
		auto hash_index = hash_value % Capacity;
		// if bucket collision, look for next bucket
		while (0 != this->m_hash[hash_index] && hash_value != this->m_hash[hash_index]) {
			hash_index++;
		}
		this->m_hash[hash_index] = hash_value;
		// remember to reset memory if previous value
		// is not a new value to work with
		if (this->m_hash[hash_index] != 0) {
			this->m_data[hash_index].~Value();
		}
		this->m_data[hash_index] = value;
	}
	void insert(const char* key, Value&& value) {
		this->insert(key, value);
	}

	Value* at(const char* key) {
		auto hash_value = hash(key, std::strlen(key));
		auto hash_index = hash_value % Capacity;

		while (0 != this->m_hash[hash_index] && hash_value != this->m_hash[hash_index]) {
			hash_index++;
		}

		if (this->m_hash[hash_index] == 0) {
			return nullptr;
		}
		else {
			return &this->m_data[hash_index];
		}
	}

private:
	std::unique_ptr<uint32_t[]> m_hash;
	std::unique_ptr<Value[]> m_data;
	static uint32_t hash(const char* key, int key_length) {
		uint32_t hashed_value = 0;
		if (key[0] != '\0')
		{
			for (int i = 0; i < key_length; i++)
			{
				hashed_value = 33 * hashed_value + key[i];
			}
		}
		return hashed_value;
	}
};

