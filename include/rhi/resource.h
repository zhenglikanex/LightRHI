#pragma once

#include <atomic>
#include <cassert>

namespace light::rhi
{
	struct Resource
	{
		Resource() = default;
		virtual ~Resource() = 0 {}

		// Non-copyable and non-movable
		Resource(const Resource&) = delete;
		Resource(const Resource&&) = delete;
		Resource& operator=(const Resource&) = delete;
		Resource& operator=(const Resource&&) = delete;

        uint32_t AddRef()
		{
			return ++ref_count_;
		}

        uint32_t Release()
		{
            uint32_t count = --ref_count_;
            if(count == 0)
            {
                delete this;
            }
            return count;
		}
	private:
		std::atomic<uint32_t> ref_count_{ 1 };
	};

    //////////////////////////////////////////////////////////////////////////
    // Handle
    // Mostly a copy of Microsoft::WRL::ComPtr<T>
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class Handle
    {
    public:
        typedef T InterfaceType;

        template <bool b, typename U = void>
        struct EnableIf
        {
        };

        template <typename U>
        struct EnableIf<true, U>
        {
            typedef U type;
        };

    protected:
        InterfaceType* ptr_;
        template<class U> friend class Handle;
        
        void InternalAddRef() const noexcept
        {
            if (ptr_ != nullptr)
            {
                ptr_->AddRef();
            }
        }

        unsigned long InternalRelease() noexcept
        {
            unsigned long ref = 0;
            T* temp = ptr_;

            if (temp != nullptr)
            {
                ptr_ = nullptr;
                ref = temp->Release();
            }

            return ref;
        }

    public:
        Handle() noexcept : ptr_(nullptr)
        {
        }

        Handle(std::nullptr_t) noexcept : ptr_(nullptr)
        {
        }

        template<class U>
        Handle(U* other) noexcept : ptr_(other)
        {
            InternalAddRef();
        }

        Handle(const Handle& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        // copy ctor that allows to instanatiate class when U* is convertible to T*
        template<class U>
        Handle(const Handle<U>& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept :
            ptr_(other.ptr_)

        {
            InternalAddRef();
        }

        Handle(Handle&& other) noexcept : ptr_(nullptr)
        {
            if (this != reinterpret_cast<Handle*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
        }

        // Move ctor that allows instantiation of a class when U* is convertible to T*
        template<class U>
        Handle(Handle<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept :
            ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        ~Handle() noexcept
        {
            InternalRelease();
        }

        Handle& operator=(std::nullptr_t) noexcept
        {
            InternalRelease();
            return *this;
        }

        Handle& operator=(T* other) noexcept
        {
            if (ptr_ != other)
            {
                Handle(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        Handle& operator=(U* other) noexcept
        {
            Handle(other).Swap(*this);
            return *this;
        }

        Handle& operator=(const Handle& other) noexcept  // NOLINT(bugprone-unhandled-self-assignment)
        {
            if (ptr_ != other.ptr_)
            {
                Handle(other).Swap(*this);
            }
            return *this;
        }

        template<class U>
        Handle& operator=(const Handle<U>& other) noexcept
        {
            Handle(other).Swap(*this);
            return *this;
        }

        Handle& operator=(Handle&& other) noexcept
        {
            Handle(static_cast<Handle&&>(other)).Swap(*this);
            return *this;
        }

        template<class U>
        Handle& operator=(Handle<U>&& other) noexcept
        {
            Handle(static_cast<Handle<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(Handle&& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(Handle& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        [[nodiscard]] T* Get() const noexcept
        {
            return ptr_;
        }

        operator T* () const
        {
            return ptr_;
        }

        InterfaceType* operator->() const noexcept
        {
            return ptr_;
        }

        T** operator&()   // NOLINT(google-runtime-operator)
        {
            return &ptr_;
        }

        [[nodiscard]] T* const* GetAddressOf() const noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** GetAddressOf() noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &ptr_;
        }

        T* Detach() noexcept
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        // Set the pointer while keeping the object's reference count unchanged
        void Attach(InterfaceType* other)
        {
            if (ptr_ != nullptr)
            {
                auto ref = ptr_->Release();
                (void)ref;

                // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
                // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
                assert(ref != 0 || ptr_ != other);
            }

            ptr_ = other;
        }

        // Create a wrapper around a raw object while keeping the object's reference count unchanged
        static Handle<T> Create(T* other)
        {
            Handle<T> Ptr;
            Ptr.Attach(other);
            return Ptr;
        }

        unsigned long Reset()
        {
            return InternalRelease();
        }
    };    // Handle

    typedef Handle<Resource> ResourceHandle;

    template<class T,class ... Args>
    Handle<T> MakeHandle(Args&& ... args)
    {
        auto ptr = new T(std::forward<Args>(args)...);
        return Handle<T>::Create(ptr);
    }
}