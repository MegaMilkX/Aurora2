// Game Programming Gems 1 - 1.6 - A Generic Handle-Based Resource Manager by Scott Bilas

#ifndef HANDLE_MGR_H
#define HANDLE_MGR_H

#include <cassert>
#include <vector>

template<typename TAG>
class handle
{
    union
    {
        enum
        {
            MAX_BITS_INDEX = 16;
            MAX_BITS_MAGIC = 16;
            MAX_INDEX = (1 << MAX_BITS_INDEX) - 1;
            MAX_MAGIC = (1 << MAX_BITS_MAGIC) - 1;
        };
        struct
        {
            unsigned m_index : MAX_BITS_INDEX;
            unsigned m_magic : MAX_BITS_MAGIC;
        };
        unsigned int m_handle;
    };
public:
    // lifetime
    handle() : m_handle(0) {}
    void init(unsigned int idx)
    {
        assert(is_null());
        assert(index <= MAX_INDEX);

        static unsigned int s_auto_magic = 0;
        if(++s_auto_magic > MAX_MAGIC)
            s_auto_magic = 1;

        m_index = idx;
        m_magic = s_auto_magic;
    }
    // query
    unsigned int get_index() const { return m_index; }
    unsigned int get_magic() const { return m_magic; }
    unsigned int get_handle() const { return m_handle; }
    bool is_null() const { return !m_handle; }

    operator unsigned int() const { return m_handle; }
};

template<typename TAG>
inline bool operator!=(handle<TAG> l, handle<TAG> r)
{ return l.get_handle() != r.get_handle(); }
template<typename TAG>
inline bool operator==(handle<TAG> l, handle<TAG> r)
{ return l.get_handle() == r.get_handle(); }

template<typename DATA, typename HANDLE>
class handle_mgr
{
private:
    typedef std::vector<DATA> user_vec_t;
    typedef std::vector<unsigned int> magic_vec_t;
    typedef std::vector<unsigned int> free_vec_t;

    user_vec_t m_user_data;
    magic_vec_t m_magic_numbers;
    free_vec_t m_free_slots;
public:
    handle_mgr() {}
    ~handle_mgr() {}

    DATA* acquire(HANDLE& hdl)
    {
        unsigned int idx;
        if(m_free_slots.empty())
        {
            idx = m_magic_numbers.size();
            hdl.init(idx);
            m_user_data.push_back(DATA());
            m_magic_numbers.push_back(hdl.get_magic());
        }
        else
        {
            idx = m_free_slots.back();
            hdl.init(idx);
            m_free_slots.pop_back();
            m_magic_numbers[idx] = hdl.get_magic();
        }
        return m_user_data.begin() + idx;
    }
    void release(HANDLE hdl)
    {
        unsigned int idx = hdl.get_index();
        assert(idx < m_user_data.size());
        assert(m_magic_numbers[idx] == hdl.get_magic());

        m_magic_numbers[idx] = 0;
        m_free_slots.push_back(idx);
    }

    DATA* deref(HANDLE hdl)
    {
        if(hdl.is_null()) return 0;

        unsigned int idx = hdl.get_index();
        if(idx >= m_user_data.size() ||
            m_magic_numbers[idx] != hdl.get_magic())
        {
            assert(0);
            return 0;
        }
        return m_user_data.begin() + idx;
    }
    const DATA* deref(HANDLE hdl) const
    {
        typedef handle_mgr<DATA, HANDLE> this_type;
        return const_cast<this_type*>(this)->deref(hdl);
    }

    unsigned int get_used_handle_count() const
    { return m_magic_numbers.size() - m_free_slots.size(); }
    bool has_used_handles() const
    { return !!get_used_handle_count(); }
};

#endif
