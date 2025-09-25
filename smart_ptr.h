#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

class WeakPtr;

class SharedPtr {
   public:
    SharedPtr() = default;
    explicit SharedPtr(std::string* p);

    explicit SharedPtr(const WeakPtr& pointer);

    SharedPtr(const SharedPtr& other);
    SharedPtr& operator=(const SharedPtr& other);

    ~SharedPtr();

    SharedPtr(SharedPtr&& other) noexcept;
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    [[nodiscard]] std::string* Get() const;
    std::string& operator*() const;
    std::string* operator->() const;
    void Reset(std::string* ptr = nullptr);

   private:
    class ControlBlock {
       public:
        size_t shared_count_ = 0;
        size_t weak_count_ = 0;
    };

    std::string* ptr_ = nullptr;
    ControlBlock* control_ = nullptr;

    void Release();

    friend class WeakPtr;
};

class WeakPtr {
   public:
    WeakPtr() = default;
    WeakPtr(const WeakPtr& other);
    WeakPtr(WeakPtr&& other) noexcept;
    WeakPtr& operator=(const WeakPtr& other);
    WeakPtr& operator=(WeakPtr&& other) noexcept;

    explicit WeakPtr(const SharedPtr& shared);
    ~WeakPtr();

    [[nodiscard]] SharedPtr Lock() const;
    [[nodiscard]] bool IsExpired() const;

   private:
    SharedPtr::ControlBlock* control_ = nullptr;
    std::string* ptr_ = nullptr;

    friend class SharedPtr;
};

inline void SharedPtr::Release() {
    if (control_ != nullptr && ptr_ != nullptr) {
        bool should_delete = false;
        if (--control_->shared_count_ == 0) {
            delete ptr_;
            ptr_ = nullptr;
            should_delete = (control_->weak_count_ == 0);
        }
        if (should_delete) {
            delete control_;
            control_ = nullptr;
        }
    }
}

inline SharedPtr::SharedPtr(const WeakPtr& pointer)
    : ptr_(pointer.ptr_), control_(pointer.control_) {
    if (control_ != nullptr && control_->shared_count_ > 0) {
        ++(control_->shared_count_);
    } else {
        ptr_ = nullptr;
        control_ = nullptr;
    }
}

inline SharedPtr::SharedPtr(std::string* p) : ptr_(p) {
    if (ptr_ != nullptr) {
        control_ = new ControlBlock();  // NOLINT(cppcoreguidelines-owning-memory)
        control_->shared_count_ = 1;
        control_->weak_count_ = 0;
    }
}

inline SharedPtr::SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), control_(other.control_) {
    if (control_ != nullptr && ptr_ != nullptr) {
        ++(control_->shared_count_);
    }
}

inline SharedPtr& SharedPtr::operator=(const SharedPtr& other) {
    if (this != &other) {
        Release();
        ptr_ = other.ptr_;
        control_ = other.control_;
        if (control_ != nullptr) {
            ++(control_->shared_count_);
        }
    }
    return *this;
}

inline SharedPtr::~SharedPtr() {
    Release();
}

inline SharedPtr::SharedPtr(SharedPtr&& other) noexcept
    : ptr_(other.ptr_), control_(other.control_) {
    other.ptr_ = nullptr;
    other.control_ = nullptr;
}

inline SharedPtr& SharedPtr::operator=(SharedPtr&& other) noexcept {
    if (this != &other) {
        Release();

        ptr_ = other.ptr_;
        control_ = other.control_;

        other.ptr_ = nullptr;
        other.control_ = nullptr;
    }
    return *this;
}

inline std::string* SharedPtr::Get() const {
    return ptr_;
}

inline std::string& SharedPtr::operator*() const {
    if (ptr_ == nullptr) {
        throw std::runtime_error("Dereferencing a nullptr");
    }
    return *ptr_;
}

inline std::string* SharedPtr::operator->() const {
    if (ptr_ == nullptr) {
        throw std::runtime_error("Accessing member of a nullptr");
    }
    return ptr_;
}

inline void SharedPtr::Reset(std::string* ptr) {
    Release();
    ptr_ = ptr;

    if (ptr_ != nullptr) {
        control_ = new ControlBlock();  // NOLINT(cppcoreguidelines-owning-memory)
        control_->shared_count_ = 1;
        control_->weak_count_ = 0;
    } else {
        control_ = nullptr;
    }
}

inline WeakPtr::WeakPtr(const WeakPtr& other) : control_(other.control_), ptr_(other.ptr_) {
    if (control_ != nullptr) {
        ++control_->weak_count_;
    }
}

inline WeakPtr& WeakPtr::operator=(const WeakPtr& other) {
    if (this != &other) {
        if (control_ != nullptr) {
            if (--control_->weak_count_ == 0 && control_->shared_count_ == 0) {
                delete control_;
            }
        }
        control_ = other.control_;
        ptr_ = other.ptr_;
        if (control_ != nullptr) {
            ++control_->weak_count_;
        }
    }
    return *this;
}

inline WeakPtr::WeakPtr(WeakPtr&& other) noexcept : control_(other.control_), ptr_(other.ptr_) {
    other.control_ = nullptr;
    other.ptr_ = nullptr;
}

inline WeakPtr& WeakPtr::operator=(WeakPtr&& other) noexcept {
    if (this != &other) {
        if (control_ != nullptr) {
            if (--control_->weak_count_ == 0 && control_->shared_count_ == 0) {
                delete control_;
            }
        }
        control_ = other.control_;
        ptr_ = other.ptr_;
        other.control_ = nullptr;
        other.ptr_ = nullptr;
    }
    return *this;
}

inline WeakPtr::WeakPtr(const SharedPtr& shared) : control_(shared.control_), ptr_(shared.ptr_) {
    if (control_ != nullptr) {
        ++control_->weak_count_;
    }
}

inline WeakPtr::~WeakPtr() {
    if (control_ != nullptr) {
        if (--control_->weak_count_ == 0 && control_->shared_count_ == 0) {
            delete control_;
        }
    }
}

inline SharedPtr WeakPtr::Lock() const {
    return WeakPtr::IsExpired() ? SharedPtr() : SharedPtr(*this);
}

inline bool WeakPtr::IsExpired() const {
    return control_ == nullptr || control_->shared_count_ == 0;
}