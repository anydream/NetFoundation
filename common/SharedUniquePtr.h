#pragma once

#include <memory>

//////////////////////////////////////////////////////////////////////////
template <class T>
class SharedUniquePtr
{
public:
	SharedUniquePtr() noexcept
	{
	}

	explicit SharedUniquePtr(T *ptr) noexcept
		: Ptr_(ptr)
	{
	}

	SharedUniquePtr(SharedUniquePtr &&other) noexcept
		: Ptr_(std::move(other.Ptr_))
	{
	}

	SharedUniquePtr(const SharedUniquePtr &other) noexcept
		: Ptr_(std::move(const_cast<SharedUniquePtr&&>(other).Ptr_))
	{
	}

	SharedUniquePtr& operator = (SharedUniquePtr &&other) noexcept
	{
		if (this != &other)
			Ptr_ = std::move(other.Ptr_);
		return *this;
	}

	SharedUniquePtr& operator = (const SharedUniquePtr &other) noexcept
	{
		return *this = const_cast<SharedUniquePtr&&>(other);
	}

	SharedUniquePtr& operator = (nullptr_t) noexcept
	{
		Ptr_.reset();
		return *this;
	}

	T* operator -> () const
	{
		return Get();
	}

	T& operator * () const
	{
		return *Get();
	}

	T* Get() const noexcept
	{
		return Ptr_.get();
	}

	void Reset(T *ptr) noexcept
	{
		Ptr_.reset(ptr);
	}

private:
	std::unique_ptr<T> Ptr_;
};
