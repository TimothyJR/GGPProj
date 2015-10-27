#pragma once
#include <wrl\client.h>
#include <utility>

template <typename PointedToType>
class dxi_ptr {
public:
	dxi_ptr()
		:m_value(nullptr)
	{}
	dxi_ptr(PointedToType* target)
		:m_value(target)
	{}
	dxi_ptr(const dxi_ptr& rhs) {
		this->m_value = rhs.m_value;
	}
	dxi_ptr& operator =(const dxi_ptr& rhs) {
		this->m_value = rhs.m_value;
		return *this;
	}
	dxi_ptr(dxi_ptr&& rhs) {
		
		this->m_value = std::move(rhs.m_value);
	}
	dxi_ptr& operator =(dxi_ptr&& rhs) {
		this->m_value = std::move(rhs.m_value);
		return *this;
	}
	PointedToType** operator &() {
		return this->m_value.GetAddressOf();
	}
	PointedToType* operator ->() {
		return this->m_value.Get();
	}
	operator PointedToType*() {
		return this->m_value.Get();
	}
	PointedToType& operator *() {
		return *this->m_value.Get();
	}
private:
	Microsoft::WRL::ComPtr<PointedToType> m_value;
};
