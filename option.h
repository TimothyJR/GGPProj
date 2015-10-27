#pragma once
#include <utility>
#pragma warning(push)
#pragma warning( disable : 4624)

template <typename In, typename Func>
struct return_type_of {
	static auto test(In param, Func my_func) -> decltype(my_func(param)) {
		return my_func(param);
	};
	typedef decltype(test(std::declval<In>(), std::declval<Func>())) Output;
};

template <typename Value>
class option
{
public:
	typedef Value Value;
	template <typename Func>
	struct mapped {
		typedef typename return_type_of<Value, Func>::Output Type;
		typedef typename option<typename mapped::Type> Option;
	};

	option() : m_present(false) {

	}
	explicit option(Value value) : m_present(true), m_data(std::move(value)) {

	}

	option(option&& rhs) {
		this->m_present = rhs.m_present;
		rhs.m_present = false;
		if (rhs.is_some()) {
			this->m_data = std::move(rhs.m_data);
		}
	}
	option& operator = (option&& rhs) {
		this->m_present = rhs.m_present;
		rhs.m_present = false;
		if (rhs.is_some()) {
			this->m_data = std::move(rhs.m_data);
		}
		return *this;
	}
	~option() {
		if (this->m_present) {
			this->m_data.~Value();
		}
	}
	bool is_some() {
		return this->m_present;
	}
	bool is_none() {
		return !this->is_some();
	}

	option<Value&> as_ref() {
		return option<Value&>(this->m_data);
	}

	Value& operator *() {
		assert(this->m_present);
		return this->m_data;
	}

	const Value& unwrap() const {
		assert(this->m_present);
		return this->m_data;
	}
	Value& unwrap() {
		assert(this->m_present);
		return this->m_data;
	}

	Value&& take() {
		assert(this->m_present);
		this->m_present = false;
		return std::move(this->m_data);
	}

	Value* operator ->() {
		assert(this->m_present);
		return &this->m_data;
	}

	Value unwrap_or(Value&& default) const {
		if (this->m_present) {
			return this->m_data;
		}
		else {
			return default;
		}
	}
	static const option none;

	bool operator ==(const option& rhs) const {
		if (this->m_present == rhs.m_present) {
			if (this->m_present) {
				return this->m_data == rhs.m_data;
			}
			else {
				return true;
			}
		}
		else {
			return false;
		}
	}
	bool operator !=(const option& rhs) const {
		return !(*this == rhs);
	}


	template <typename MapFunc>
	auto map(MapFunc f) -> typename mapped<MapFunc>::Option {
		if (this->m_present) {
			return mapped<MapFunc>::Option(f(this->m_data));
		}
		else {
			return mapped<MapFunc>::Option::none;
		}
	}
	template <typename MapFunc>
	auto flat_map(MapFunc f) -> typename return_type_of<Value, MapFunc>::Output {
		if (this->m_present) {
			return f(this->m_data);
		}
		else {
			return return_type_of<Value, MapFunc>::Output();
		}
	}
private:
	bool m_present;
	union {
		Value m_data;
	};
};

template <typename Value>
class option<Value&>
{
public:
	template <typename Func>
	struct mapped {
		typedef typename return_type_of<Value, Func>::Output Type;
		typedef typename option<typename mapped::Type> Option;
	};

	option() : m_data(nullptr) {

	}
	explicit option(const Value& value) : m_data(&value) {

	}
	option(option&& rhs) {
		this->m_data = rhs.m_data;
		rhs.m_data = nullptr;
	}
	option& operator = (option&& rhs) {
		this->m_data = rhs.m_data;
		rhs.m_data = nullptr;
	}
	~option() {
		if (this->m_data != nullptr) {
			this->m_data->~Value();
		}
	}
	bool is_some() {
		return this->m_data != nullptr;
	}
	bool is_none() {
		return !this->is_some();
	}

	option<Value&> as_ref() {
		return option<Value&>(*this->m_data);
	}

	Value& unwrap() {
		assert(this->m_data != nullptr);
		return *this->m_data;
	}

	Value& operator *() {
		return this->unwrap();
	}

	Value* operator ->() {
		assert(this->m_data != nullptr);
		return this->m_data;
	}

	Value& unwrap_or(Value& default) {
		if (this->m_data != nullptr) {
			return *this->m_data;
		}
		else {
			return default;
		}
	}
	const Value& unwrap() const {
		assert(this->m_data != nullptr);
		return *this->m_data;
	}

	const Value& unwrap_or(Value& default) const {
		if (this->m_data != nullptr) {
			return *this->m_data;
		}
		else {
			return default;
		}
	}

	static const option none;

	bool operator ==(const option& rhs) const {
		if (this->m_data == rhs.m_data) {
			if (this->m_data != nullptr) {
				return *this->m_data == *rhs.m_data;
			}
			else {
				return true;
			}
		}
		else {
			return false;
		}
	}
	bool operator !=(const option& rhs) const {
		return !(*this == rhs);
	}


	template <typename MapFunc>
	auto map(MapFunc f) -> typename mapped<MapFunc>::Option {
		if (this->m_data != nullptr) {
			return mapped<MapFunc>::Option(f(*this->m_data));
		}
		else {
			return mapped<MapFunc>::Option::none;
		}
	}
	template <typename MapFunc>
	auto flat_map(MapFunc f) -> typename return_type_of<Value, MapFunc>::Output {
		if (this->m_data != nullptr) {
			return f(*this->m_data);
		}
		else {
			return return_type_of<Value, MapFunc>::Output();
		}
	}
private:
	Value* m_data;

};

template <typename Value>
const option<Value> option<Value>::none = option();


template <typename Value>
option<Value> None() {
	return option<Value>();
}
template<typename Value>

option<std::remove_reference_t<Value>> Some(Value&& value) {
	return option<std::remove_reference_t<Value>>(std::forward<Value>(value));
}
#pragma warning(pop)