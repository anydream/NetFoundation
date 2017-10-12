#pragma once

//////////////////////////////////////////////////////////////////////////
template <class T>
class SharedUniquePtr
{
public:
	SharedUniquePtr() noexcept
	{
	}

	explicit SharedUniquePtr(T *rawPtr) noexcept
		: Ptr_(rawPtr)
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

	T* Get() const
	{
		return Ptr_.get();
	}

private:
	std::unique_ptr<T> Ptr_;
};
