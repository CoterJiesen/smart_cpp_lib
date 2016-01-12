// filesystem standard header
#pragma once
#ifndef _FILESYSTEM_
#define _FILESYSTEM_
#ifndef RC_INVOKED
#include <fstream>
#include <ctime>	// for time_t
#include <new>	// for nothrow
#include <string>


 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,3)

 #if _HAS_CPP0X

 #else /* _HAS_CPP0X */
 #error C++0X not fully supported
 #endif /* _HAS_CPP0X */

namespace win32 {
	namespace file {
template<class _Path>
	struct slash;
template<class _Path>
	struct dot;
template<class _Path>
	struct colon;
template<class _Path>
	struct _Altslash;

	using namespace std;
		// ENUM file_type
enum file_type {	// types of files
	status_unknown, file_not_found, regular_file, directory_file,
	symlink_file, block_file, character_file, fifo_file,
	socket_file, type_unknown
	};

		// CLASS file_status
class file_status
	{	// stores file status
public:
	explicit file_status(file_type _Ftype = status_unknown)
		: _Myftype(_Ftype)
		{	// default construct
		}

	file_type type() const
		{	// get file type
		return (_Myftype);
		}

	void type(file_type _Ftype)
		{	// set file type
		_Myftype = _Ftype;
		}
	
private:
	file_type _Myftype;
	};

		// STRUCT space_info
struct space_info
	{	// space information for a file
	_ULonglong capacity;
	_ULonglong free;
	_ULonglong available;
	};

		// TEMPLATE CLASS _Path_iterator
template<class _Path>
	class _Path_iterator
		: public iterator<
			bidirectional_iterator_tag,
			typename _Path::string_type,
			ptrdiff_t,
			typename _Path::string_type *,
			typename _Path::string_type&>
	{	// bidirectional iterator for basic_path
public:
	typedef _Path_iterator<_Path> _Myt;
	typedef _Path path_type;
	typedef typename _Path::string_type string_type;
	typedef const typename _Path::string_type *pointer;
	typedef const typename _Path::string_type& reference;

	_Path_iterator()
		: _Myptr(0), _Myoff(0)
		{	// construct singular iterator
		}

	_Path_iterator(const path_type& _Pval, size_t _Off)
		: _Myptr(&_Pval), _Myoff(_Off)
		{	// construct iterator
		_Getval();
		}

	_Path_iterator(const _Path_iterator& _Right)
		: _Myptr(_Right._Myptr),
			_Myelem(_Right._Myelem),
			_Myoff(_Right._Myoff)
		{	// copy construct
		}

	_Path_iterator& operator=(const _Path_iterator& _Right)
		{	// copy assign
		_Myptr = _Right._Myptr;
		_Myelem = _Right._Myelem;
		_Myoff = _Right._Myoff;
		return (*this);
		}

	_Path_iterator(_Path_iterator&& _Right)
		: _Myptr(_Right._Myptr),
			_Myelem(_STD move(_Right._Myelem)),
			_Myoff(_Right._Myoff)
		{	// move construct
		}

	_Path_iterator& operator=(_Path_iterator&& _Right)
		{	// move assign
		_Myptr = _Right._Myptr;
		_Myelem = _STD move(_Right._Myelem);
		_Myoff = _Right._Myoff;
		return (*this);
		}

	reference operator*() const
		{	// return designated value
		return (_Myelem);
		}

	pointer operator->() const
		{	// return pointer to class object
		return (_STD pointer_traits<pointer>::pointer_to(**this));
		}

	_Myt& operator++()
		{	// preincrement
 #pragma warning(push)
 #pragma warning(disable: 6011)	/* quiet analyze noise */

 #if _ITERATOR_DEBUG_LEVEL == 2
		if (_Myptr == 0 || _Myptr->_Mystr.size() <= _Myoff)
			_DEBUG_ERROR(
				"basic_path<T>::iterator is not incrementable");

 #elif _ITERATOR_DEBUG_LEVEL == 1
		_SCL_SECURE_VALIDATE(_Myptr != 0 && _Myoff < _Myptr->_Mystr.size());
 #endif /* _ITERATOR_DEBUG_LEVEL */

		size_t _Pend = _Myptr->_Prefix_end();
		size_t _Size = _Myptr->_Mystr.size();

		if (_Myoff < _Pend)
			_Myoff = _Pend;	// point past "x:"
		else if (_Myoff == _Pend && _Pend < _Size
			&& _Myptr->_Mystr[_Pend] == slash<path_type>::value)
			{	// point past root "/" and following slashes
			for (++_Myoff; _Myoff < _Size; ++_Myoff)
				if (_Myptr->_Mystr[_Myoff]
					!= slash<path_type>::value)
					break;
			}
		else
			{	// point past slashes followed by stuff
			for (; _Myoff < _Size; ++_Myoff)
				if (_Myptr->_Mystr[_Myoff]
					!= slash<path_type>::value)
					break;
			for (; _Myoff < _Size; ++_Myoff)
				if (_Myptr->_Mystr[_Myoff]
					== slash<path_type>::value)
					break;
			}
		_Getval();
		return (*this);

 #pragma warning(pop)
		}

	_Myt operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_Myt& operator--()
		{	// predecrement
 #if _ITERATOR_DEBUG_LEVEL == 2
		if (_Myptr == 0 || _Myoff == 0)
			_DEBUG_ERROR(
				"basic_path<T>::iterator is not decrementable");

 #elif _ITERATOR_DEBUG_LEVEL == 1
		_SCL_SECURE_VALIDATE(_Myptr != 0 && _Myoff != 0);
 #endif /* _ITERATOR_DEBUG_LEVEL */

		size_t _Myoff_sav = _Myoff;
		size_t _Off_back = 0;
		_Myoff = 0;
		do
			{	// scan down to previous
			_Off_back = _Myoff;
			++*this;
			} while (_Myoff < _Myoff_sav);
		_Myoff = _Off_back;
		_Getval();
		return (*this);
		}

	_Myt operator--(int)
		{	// postdecrement
		_Myt _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	bool operator==(const _Myt& _Right) const
		{	// test for iterator equality
		return (_Myptr == _Right._Myptr && _Myoff == _Right._Myoff);
		}

	bool operator!=(const _Myt& _Right) const
		{	// test for iterator inequality
		return (!(*this == _Right));
		}

private:
	void _Getval()
		{	// determine _Myelem
		size_t _Pend = _Myptr->_Prefix_end();
		size_t _Size = _Myptr->_Mystr.size();

		_Myelem.clear();
		if (_Size <= _Myoff)
			;	// off end, no field
		else if (_Myoff < _Pend)
			_Myelem = _Myptr->_Mystr.substr(0, _Pend);	// get "x:"
		else if (_Myoff == _Pend && _Pend < _Size
			&& _Myptr->_Mystr[_Pend] == slash<path_type>::value)
			_Myelem = slash<path_type>::value;	// get "/"
		else
			{	// determine next field as slashes followed by stuff
			size_t _Nslash = 0;
			size_t _Nstuff = 0;

			for (; _Myoff + _Nslash < _Size; ++_Nslash)
				if (_Myptr->_Mystr[_Myoff + _Nslash]
					!= slash<path_type>::value)
					break;
			for (; _Myoff + _Nslash + _Nstuff < _Size; ++_Nstuff)
				if (_Myptr->_Mystr[_Myoff + _Nslash + _Nstuff]
					== slash<path_type>::value)
					break;
			if (0 < _Nstuff)
				_Myelem = _Myptr->_Mystr.substr(_Myoff + _Nslash,
					_Nstuff);	// get "stuff"
			else if (0 < _Nslash)
				_Myelem = dot<path_type>::value;	// get "."
			}
		}

	const path_type *_Myptr;	// pointer to basic_path
	string_type _Myelem;	// current element substring
	size_t _Myoff;	// current offset in basic_path	
	};

		// TEMPLATE CLASS basic_path AND FRIENDS
template<class _String,
	class _Traits>
	class basic_path
	{	// stores a pathname
public:
	typedef basic_path<_String, _Traits> path_type;
	typedef _String string_type;
	typedef _Traits traits_type;
	typedef typename _String::value_type value_type;
	typedef typename _Traits::external_string_type external_string_type;

	basic_path()
		{	// default construct
		}

	basic_path(const string_type& _Str)
		{	// construct from string
		*this /= _Str;
		}

	basic_path(const value_type *_Ptr)
		{	// construct from NTBS
		*this /= _Ptr;
		}

	template<class _InIt>
		basic_path(_InIt _First, _InIt _Last)
		{	// construct from [_First, _Last)
		append(_First, _Last);
		}

	basic_path(const basic_path& _Right)
		: _Mystr(_Right._Mystr)
		{	// copy construct
		}

	basic_path& operator=(const basic_path& _Right)
		{	// copy assign
		_Mystr = _Right._Mystr;
		return (*this);
		}

	~basic_path() 
		{	// destroy the object
		}

	basic_path(basic_path&& _Right)
		: _Mystr(_STD move(_Right._Mystr))
		{	// move construct
		}

	basic_path& operator=(basic_path&& _Right)
		{	// move assign
		_Mystr = _STD move(_Right._Mystr);
		return (*this);
		}

	basic_path& operator=(const string_type& _Str)
		{	// assign from string
		if (&_Mystr == &_Str)
			return (*this);	// self assignment
		else
			{	// strings differ, clear and append
			clear();
			return (*this /= _Str);
			}
		}

	basic_path& operator=(const value_type *_Ptr)
		{	// assign from NTBS
		clear();
		return (*this /= _Ptr);
		}

	template<class _InIt>
		basic_path& assign(_InIt _First, _InIt _Last)
		{	// assign from [_First, _Last)
		clear();
		return (append(_First, _Last));
		}

	basic_path& operator/=(const basic_path& _Pval)
		{	// append copy
		return (append(_Pval._Mystr.c_str(),
			_Pval._Mystr.c_str() + _Pval._Mystr.size()));
		}

	basic_path& operator/=(const string_type& _Str)
		{	// append string
		return (append(_Str.c_str(),
			_Str.c_str() + _Str.size()));
		}

	basic_path& operator/=(const value_type *_Ptr)
		{	// append NTBS
		const value_type *_Ptr2 = _Ptr;
		for (; *_Ptr2 != 0; ++_Ptr2)
			;
		return (append(_Ptr, _Ptr2));
		}

	template<class _InIt>
		basic_path& append(_InIt _First, _InIt _Last)
		{	// append [_First, _Last)
		string_type _Str = _Mystr;	// avoid overlap
		size_t _Oldsize = _Str.size();
		for (; _First != _Last && *_First != value_type(0); ++_First)
			_Str.push_back(*_First);

		if (_Oldsize + 3 <= _Str.size()
			&& _Str[_Oldsize] == slash<path_type>::value
			&& _Str[_Oldsize + 1] == slash<path_type>::value
			&& _Str[_Oldsize + 2] == colon<path_type>::value)
			_Str.erase(_Oldsize, 3);	// drop leading escape sequence

		for (size_t _Idx = _Oldsize; _Idx < _Str.size(); ++_Idx)
			if (_Str[_Idx] == _Altslash<path_type>::value)
				_Str[_Idx] = slash<path_type>::value;	// convert '\' to '/'

		if (0 < _Oldsize && _Oldsize < _Str.size()
			&& _Str[_Oldsize - 1] != colon<path_type>::value
			&& _Str[_Oldsize - 1] != slash<path_type>::value
			&& _Str[_Oldsize] != slash<path_type>::value)
			_Str.insert(_Oldsize, 1, slash<path_type>::value);	// add '/'

		_Mystr = _Str;	// copy back new filename
		return (*this);
		}

	void clear()
		{	// clear the stored string
		_Mystr.clear();
		}

	void swap(basic_path& _Right)
		{	// swap the stored strings
		_Swap_adl(_Mystr, _Right._Mystr);
		}

	basic_path& remove_leaf()
		{	// remove leaf
		if (!empty() && begin() != --end())
			{	// leaf to remove, back up over it
			size_t _Rend = _Root_end();
			size_t _Idx = _Mystr.size();

			for (; _Rend < _Idx; --_Idx)
				if (_Mystr[_Idx - 1] == slash<path_type>::value)
					break;	// back up over stuff at end
			for (; _Rend < _Idx; --_Idx)
				if (_Mystr[_Idx - 1] != slash<path_type>::value)
					break;	// back up over trailing non-root slashes
			_Mystr.erase(_Idx);
			}
		return (*this);
		}

	basic_path& remove_filename()
		{	// remove leaf
		return (remove_leaf());
		}

	basic_path& replace_extension(const string_type& _Newext = string_type())
		{	// replace extension with _Newext
		if (_Newext.empty() || _Newext[0] == dot<path_type>::value)
			*this = branch_path()
				/ basic_path((stem() + _Newext));
		else
			*this = branch_path()
				/ basic_path((stem() + dot<path_type>::value + _Newext));
		return (*this);
		}

	string_type string() const
		{	// get the stored string
		return (_Mystr);
		}

	string_type file_string() const
		{	// get the stored string, native form
		string_type _Ans;
		size_t _Idx = 0;
		size_t _Size = _Mystr.size();

		if (2 <= _Size && _Mystr[0] == slash<path_type>::value)
			while (_Idx < _Size - 1
				&& _Mystr[_Idx + 1] == slash<path_type>::value)
				++_Idx;	// compress leading slashes
		for (; _Idx < _Size; ++_Idx)
			if (_Mystr[_Idx] == slash<path_type>::value)
				_Ans.append(1, _Altslash<path_type>::value);	// convert '/' to '\'
			else
				_Ans.append(1, _Mystr[_Idx]);
		return (_Ans);
		}

	operator string_type() const
		{	// convert to file string
		return (file_string());
		}

	string_type directory_string() const
		{	// get the stored string, directory form
		return (file_string());
		}

	external_string_type external_file_string() const
		{	// get external form of file string
		return (_Traits::to_external(*this, file_string()));
		}

	external_string_type external_directory_string() const
		{	// get external form of directory string
		return (_Traits::to_external(*this, directory_string()));
		}

	string_type root_name() const
		{	// get root name
		return (_Mystr.substr(0, _Prefix_end()));
		}

	string_type root_directory() const
		{	// get root directory
		size_t _Idx = _Prefix_end();
		if (_Idx < _Mystr.size()
			&& _Mystr[_Idx] == slash<path_type>::value)
			return (string_type(1, slash<path_type>::value));
		else
			return (string_type());
		}

	string_type leaf() const
		{	// get leaf
		return (empty() ? string_type() : *--end());
		}

	string_type filename() const
		{	// get leaf
		return (leaf());
		}

	basic_path root_path() const
		{	// get root path
		return (basic_path(_Mystr.c_str(),
			_Mystr.c_str() + _Root_end()));
		}

	basic_path relative_path() const
		{	// get relative path
		size_t _Idx = _Root_end();
		while (_Idx < _Mystr.size()
			&& _Mystr[_Idx] == slash<path_type>::value)
			++_Idx;	// skip extra '/' after root

		return (basic_path(_Mystr.c_str() + _Idx,
			_Mystr.c_str() + _Mystr.size()));
		}

	basic_path branch_path() const
		{	// get branch path
		basic_path _Ans;
		if (!empty())
			{	// append all but last element
			iterator _End = --end();
			for (iterator _Next = begin(); _Next != _End; ++_Next)
				_Ans /= *_Next;
			}
		return (_Ans);
		}

	basic_path parent_path() const
		{	// get branch path
		return (branch_path());
		}

	string_type basename() const
		{	// pick off base name in leaf before dot
		string_type _Str = leaf();
		size_t _Idx = _Str.rfind(dot<path_type>::value);
		return (_Str.substr(0, _Idx));
		}

	string_type stem() const
		{	// pick off base name in leaf before dot
		return (basename());
		}

	string_type extension() const
		{	// pick off .extension in leaf, including dot
		string_type _Str = leaf();
		size_t _Idx = _Str.rfind(dot<path_type>::value);
		return (_Idx == string_type::npos
			? string_type() : _Str.substr(_Idx));
		}

	bool empty() const
		{	// test if stored string is empty
		return (_Mystr.empty());
		}

	bool is_complete() const
		{	// test if root name is complete
		return (has_root_name() && has_root_directory());
		}

	bool has_root_name() const
		{	// test if root name is nonempty
		return (!root_name().empty());
		}

	bool has_root_directory() const
		{	// test if root directory is nonempty
		return (!root_directory().empty());
		}

	bool has_leaf() const
		{	// test if leaf is nonempty
		return (!leaf().empty());
		}

	bool has_filename() const
		{	// test if leaf is nonempty
		return (has_leaf());
		}

	bool has_root_path() const
		{	// test if root path is nonempty
		return (!root_path().empty());
		}

	bool has_relative_path() const
		{	// test if relative path is nonempty
		return (!relative_path().empty());
		}

	bool has_branch_path() const
		{	// test if branch path is nonempty
		return (!branch_path().empty());
		}

	bool has_parent_path() const
		{	// test if branch path is nonempty
		return (has_branch_path());
		}

	typedef _Path_iterator<path_type> iterator;
	typedef iterator const_iterator;

	iterator begin() const
		{	// get beginning of path
		return (iterator(*this, (size_t)0));
		}

	iterator end() const
		{	// get end of path
		return (iterator(*this, _Mystr.size()));
		}

	size_t _Prefix_end() const
		{	// get end of prefix
		size_t _Idx = _Mystr.find(colon<path_type>::value, 0);
		if (_Idx == _Mystr.npos)
			_Idx = 0;
		else
			++_Idx;
		return (_Idx);
		}

	size_t _Root_end() const
		{	// get end of root
		size_t _Idx = _Prefix_end();
		if (_Idx < _Mystr.size()
			&& _Mystr[_Idx] == slash<path_type>::value)
			++_Idx;
		return (_Idx);
		}

	const value_type *_Ptr() const
		{	// get the stored string NTBS
		return (_Mystr.c_str());
		}

	string_type _Mystr;
	};

template<class _String,
	class _Traits> inline
	void swap(basic_path<_String, _Traits>& _Left,
		basic_path<_String, _Traits>& _Right)
	{	// swap two paths
	_Left.swap(_Right);
	}

template<class _String,
	class _Traits> inline
	basic_path<_String, _Traits>
		operator/(const basic_path<_String, _Traits>& _Left,
			const basic_path<_String, _Traits>& _Right)
	{	// concatenate paths
	basic_path<_String, _Traits> _Ans = _Left;
	return (_Ans /= _Right);
	}

template<class _String,
	class _Traits> inline
	bool operator==(const basic_path<_String, _Traits>& _Left,
		const basic_path<_String, _Traits>& _Right)
	{	// test for basic_path equality
	return (_Left.string() == _Right.string());
	}

template<class _String,
	class _Traits> inline
	bool operator!=(const basic_path<_String, _Traits>& _Left,
		const basic_path<_String, _Traits>& _Right)
	{	// test for basic_path inequality
	return (!(_Left == _Right));
	}

template<class _String,
	class _Traits> inline
	bool operator<(const basic_path<_String, _Traits>& _Left,
		const basic_path<_String, _Traits>& _Right)
	{	// test if _Left < _Right
	return (_Left.string() < _Right.string());
	}

template<class _String,
	class _Traits> inline
	bool operator>(const basic_path<_String, _Traits>& _Left,
		const basic_path<_String, _Traits>& _Right)
	{	// test if _Left > _Right
	return (_Right < _Left);
	}

template<class _String,
	class _Traits> inline
	bool operator<=(const basic_path<_String, _Traits>& _Left,
		const basic_path<_String, _Traits>& _Right)
	{	// test if _Left <= _Right
	return (!(_Right < _Left));
	}

template<class _String,
	class _Traits> inline
	bool operator>=(const basic_path<_String, _Traits>& _Left,
		const basic_path<_String, _Traits>& _Right)
	{	// test if _Left >= _Right
	return (!(_Left < _Right));
	}

template<class _Path> inline
	basic_ostream<typename _Path::string_type::value_type,
		typename _Path::string_type::traits_type>&
	operator<<(basic_ostream<typename _Path::string_type::value_type,
		typename _Path::string_type::traits_type>& _Ostr,
		_Path _Pval)
	{	// insert a basic_path
	return (_Ostr << _Pval.string());
	}

template<class _Path> inline
	basic_istream<typename _Path::string_type::value_type,
		typename _Path::string_type::traits_type>&
	operator>>(basic_istream<typename _Path::string_type::value_type,
		typename _Path::string_type::traits_type>& _Istr,
		_Path& _Pval)
	{	// extract a basic_path
	typename _Path::string_type _Str;
	_Istr >> _Str;
	if (!_Istr.fail())
		_Pval = _Str;
	return (_Istr);
	}

		// TEMPLATE CLASS basic_filesystem_error AND FRIENDS
template<class _Path>
	class basic_filesystem_error
		: public system_error
	{	// base of all filesystem-error exceptions
public:
	typedef _Path path_type;

	explicit basic_filesystem_error(const string& _Message,
		error_code _Errcode = error_code())
		: system_error(_Errcode, _Message)
		{	// construct from message string and error code
		}

	basic_filesystem_error(const string& _Message,
		const path_type _Pval1,
		error_code _Errcode)
		: system_error(_Errcode, _Message),
			_Mypval1(_Pval1)
		{	// construct from message string and error code
		}

	basic_filesystem_error(const string& _Message,
		const path_type _Pval1,
		const path_type _Pval2,
		error_code _Errcode)
		: system_error(_Errcode, _Message),
			_Mypval1(_Pval1), _Mypval2(_Pval2)
		{	// construct from message string and error code
		}

	virtual ~basic_filesystem_error() 
		{	// destroy the object
		}

	basic_filesystem_error(basic_filesystem_error&& _Right)
		: system_error(_Right.code(), _Right.what()),
			_Mypval1(_STD move(_Right._Mypval1)),
			_Mypval2(_STD move(_Right._Mypval2))
		{	// move construct
		}

	basic_filesystem_error& operator=(basic_filesystem_error&& _Right)
		{	// move assign
		*((system_error *)this) = *(system_error *)&_Right;
		_Mypval1 = _STD move(_Right._Mypval1);
		_Mypval2 = _STD move(_Right._Mypval2);
		return (*this);
		}

	const path_type& path1() const _THROW0()
		{	// return stored first path
		return (_Mypval1);
		}

	const path_type& path2() const _THROW0()
		{	// return stored second path
		return (_Mypval2);
		}

 #if _HAS_EXCEPTIONS

 #else /* _HAS_EXCEPTIONS */
protected:
	virtual void _Doraise() const
		{	// perform class-specific exception handling
		_RAISE(*this);
		}
 #endif /* _HAS_EXCEPTIONS */

private:
	path_type _Mypval1;
	path_type _Mypval2;
	};

		// CLASSES path_traits AND wpath_traits
struct path_traits;
struct wpath_traits;

typedef basic_path<string, path_traits> path;
typedef basic_path<wstring, wpath_traits> wpath;

typedef basic_filesystem_error<path> filesystem_error;
typedef basic_filesystem_error<wpath> wfilesystem_error;

struct path_traits
	{	// traits for char pathnames
	typedef string external_string_type;
	typedef string internal_string_type;

	path_traits()
		: _Is_locked(false)
		{	// default construct
		}

	external_string_type to_external(const path&,
		const internal_string_type& _Istr)
		{	// convert to external format
		return (_Istr);
		}

	internal_string_type to_internal(const path&,
		const external_string_type& _Xstr)
		{	// convert to internal format
		return (_Xstr);
		}

	void imbue(locale _Loc);	// deferred

	bool imbue(locale _Loc, nothrow_t)
		{	// imbue unlocked with new locale, return lock status
		if (!_Is_locked)
			_Myloc = _Loc;

		bool _Retval = _Is_locked;
		_Is_locked = true;
		return (_Retval);
		}

private:
	bool _Is_locked;
	locale _Myloc;
	};

struct wpath_traits
	{	// traits for wchar_t pathnames
	typedef wstring external_string_type;
	typedef wstring internal_string_type;

	wpath_traits()
		: _Is_locked(false)
		{	// default construct
		}

	external_string_type to_external(const wpath&,
		const internal_string_type& _Istr)
		{	// convert to external format
		return (_Istr);
		}

	internal_string_type to_internal(const wpath&,
		const external_string_type& _Xstr)
		{	// convert to internal format
		return (_Xstr);
		}

	void imbue(locale _Loc);	// deferred

	bool imbue(locale _Loc, nothrow_t)
		{	// imbue unlocked with new locale, return lock status
		if (!_Is_locked)
			_Myloc = _Loc;

		bool _Retval = _Is_locked;
		_Is_locked = true;
		return (_Retval);
		}

private:
	bool _Is_locked;
	locale _Myloc;
	};

template<class _Path>
	struct is_basic_path
		: false_type
	{	// tests whether _Path is a basic_path type
	};

template<>
	struct is_basic_path<path>
		: true_type
	{	// tests whether path is a basic path type
	};

template<>
	struct is_basic_path<wpath>
		: true_type
	{	// tests whether wpath is a basic path type
	};

		// STRUCT slash AND FRIENDS
template<>
	struct slash<path>
	{	// wraps a slash character
static const char value = '/';
	};
template<>
	struct slash<wpath>
	{	// wraps a slash character
static const wchar_t value = L'/';
	};

template<>
	struct dot<path>
	{	// wraps a dot character
static const char value = '.';
	};
template<>
	struct dot<wpath>
	{	// wraps a dot character
static const wchar_t value = L'.';
	};

template<>
	struct colon<path>
	{	// wraps a colon character
static const char value = ':';
	};
template<>
	struct colon<wpath>
	{	// wraps a colon character
static const wchar_t value = L':';
	};

template<>
	struct _Altslash<path>
	{	// wraps a backslash character
static const char value = '\\';
	};
template<>
	struct _Altslash<wpath>
	{	// wraps a backslash character
static const wchar_t value = L'\\';
	};

		// DEFERRED path_traits DEFINITIONS
inline void path_traits::imbue(locale _Loc)
	{	// imbue with new locale
	if (_Is_locked)
		_THROW_NCEE(filesystem_error,
			"path_traits::imbue locked");
	_Is_locked = true;
	_Myloc = _Loc;
	}

inline void wpath_traits::imbue(locale _Loc)
	{	// imbue with new locale
	if (_Is_locked)
		_THROW_NCEE(filesystem_error,
			"wpath_traits::imbue locked");
	_Is_locked = true;
	_Myloc = _Loc;
	}

 #define _MAX_FILESYS_NAME	260	/* longest Windows or Posix filename + 1 */

	// narrow filenames
 void *__CLRCALL_PURE_OR_CDECL _Open_dir(char *, const char *, int&, file_type&);
 char *__CLRCALL_PURE_OR_CDECL _Read_dir(char *, void *, file_type&);
 void __CLRCALL_PURE_OR_CDECL _Close_dir(void *);
 char *__CLRCALL_PURE_OR_CDECL _Current_get(char *);
 bool __CLRCALL_PURE_OR_CDECL _Current_set(const char *);

 int __CLRCALL_PURE_OR_CDECL _Make_dir(const char *);
 bool __CLRCALL_PURE_OR_CDECL _Remove_dir(const char *);

 file_type __CLRCALL_PURE_OR_CDECL _Stat(const char *, int&);
 file_type __CLRCALL_PURE_OR_CDECL _Lstat(const char *, int&);
 _ULonglong __CLRCALL_PURE_OR_CDECL _File_size(const char *);
 time_t __CLRCALL_PURE_OR_CDECL _Last_write_time(const char *);
 void __CLRCALL_PURE_OR_CDECL _Last_write_time(const char *, time_t);
 space_info __CLRCALL_PURE_OR_CDECL _Statvfs(const char *);
 int __CLRCALL_PURE_OR_CDECL _Equivalent(const char *, const char *);

 int __CLRCALL_PURE_OR_CDECL _Link(const char *, const char *);
 int __CLRCALL_PURE_OR_CDECL _Symlink(const char *, const char *);
 int __CLRCALL_PURE_OR_CDECL _Rename(const char *, const char *);
 int __CLRCALL_PURE_OR_CDECL _Unlink(const char *);
 int __CLRCALL_PURE_OR_CDECL _Copy_file(const char *, const char *, bool);

	// wide filenames
 void *__CLRCALL_PURE_OR_CDECL _Open_dir(wchar_t *, const wchar_t *, int&, file_type&);
 wchar_t *__CLRCALL_PURE_OR_CDECL _Read_dir(wchar_t *, void *, file_type&);
// void __CLRCALL_PURE_OR_CDECL _Close_dir(void *);
 wchar_t *__CLRCALL_PURE_OR_CDECL _Current_get(wchar_t *);
 bool __CLRCALL_PURE_OR_CDECL _Current_set(const wchar_t *);

 int __CLRCALL_PURE_OR_CDECL _Make_dir(const wchar_t *);
 bool __CLRCALL_PURE_OR_CDECL _Remove_dir(const wchar_t *);

 file_type __CLRCALL_PURE_OR_CDECL _Stat(const wchar_t *, int&);
 file_type __CLRCALL_PURE_OR_CDECL _Lstat(const wchar_t *, int&);
 _ULonglong __CLRCALL_PURE_OR_CDECL _File_size(const wchar_t *);
 time_t __CLRCALL_PURE_OR_CDECL _Last_write_time(const wchar_t *);
 void __CLRCALL_PURE_OR_CDECL _Last_write_time(const wchar_t *, time_t);
 space_info __CLRCALL_PURE_OR_CDECL _Statvfs(const wchar_t *);
 int __CLRCALL_PURE_OR_CDECL _Equivalent(const wchar_t *, const wchar_t *);

 int __CLRCALL_PURE_OR_CDECL _Link(const wchar_t *, const wchar_t *);
 int __CLRCALL_PURE_OR_CDECL _Symlink(const wchar_t *, const wchar_t *);
 int __CLRCALL_PURE_OR_CDECL _Rename(const wchar_t *, const wchar_t *);
 int __CLRCALL_PURE_OR_CDECL _Unlink(const wchar_t *);
 int __CLRCALL_PURE_OR_CDECL _Copy_file(const wchar_t *, const wchar_t *, bool);

		// TEMPLATE CLASS basic_directory_entry AND FRIENDS
template<class _Path>
	class basic_directory_entry
	{	// represents a directory entry
public:
	typedef basic_directory_entry<_Path> _Myt;
	typedef _Path path_type;
	typedef typename _Path::string_type string_type;

	basic_directory_entry()
		: _Myftype(status_unknown),
			_Mysymftype(status_unknown)
		{	// default construct
		}

	explicit basic_directory_entry(const path_type& _Pval,
		file_status _Statarg = file_status(),
		file_status _Symstatarg = file_status())
		: _Mypval(_Pval),
			_Myftype(_Statarg.type()),
			_Mysymftype(_Symstatarg.type())
		{	// construct from path and status
		}

	basic_directory_entry(const basic_directory_entry& _Right)
		: _Mypval(_Right._Mypval),
			_Myftype(_Right._Myftype),
			_Mysymftype(_Right._Mysymftype)
		{	// copy construct
		}

	basic_directory_entry& operator=(const basic_directory_entry& _Right)
		{	// copy assign
		_Mypval = _Right._Mypval;
		_Myftype = _Right._Myftype;
		_Mysymftype = _Right._Mysymftype;
		return (*this);
		}

	basic_directory_entry(basic_directory_entry&& _Right)
		: _Mypval(_STD move(_Right._Mypval)),
			_Myftype(_Right._Myftype),
			_Mysymftype(_Right._Mysymftype)
		{	// move construct
		}

	basic_directory_entry& operator=(basic_directory_entry&& _Right)
		{	// move assign
		_Mypval = _STD move(_Right._Mypval);
		_Myftype = _Right._Myftype;
		_Mysymftype = _Right._Mysymftype;
		return (*this);
		}

	void assign(const path_type& _Pval,
		file_status _Statarg = file_status(),
		file_status _Symstatarg = file_status())
		{	// assign path and status
		_Mypval = _Pval;
		_Myftype = _Statarg.type();
		_Mysymftype = _Symstatarg.type();
		}

	void replace_leaf(const string_type& _Str,
		file_status _Statarg = file_status(),
		file_status _Symstatarg = file_status())
		{	// replace leaf and assign status
		_Mypval = _Mypval.branch_path() / _Str;
		_Myftype = _Statarg.type();
		_Mysymftype = _Symstatarg.type();
		}

	const path_type& path() const
		{	// get path
		return (_Mypval);
		}

	operator const path_type&() const
		{	// get path
		return (_Mypval);
		}

	file_status status() const
		{	// get file status
		int _Errno = 0;
		if (_Myftype != status_unknown)
			;
		else if (_Mysymftype != status_unknown
			&& _Mysymftype != symlink_file)
			_Myftype = _Mysymftype;
		else
			_Myftype = _Stat(_Mypval._Ptr(), _Errno);
		return (file_status(_Myftype));
		}

	file_status status(error_code& _Code) const
		{	// get file status
		int _Errno = 0;
		if (_Myftype != status_unknown)
			;
		else if (_Mysymftype != status_unknown
			&& _Mysymftype != symlink_file)
			_Myftype = _Mysymftype;
		else
			_Myftype = _Stat(_Mypval._Ptr(), _Errno);
		_Code = error_code(_Errno, _STD system_category());
		return (file_status(_Myftype));
		}

	file_status symlink_status() const
		{	// get file status
		int _Errno = 0;
		if (_Mysymftype == status_unknown)
			_Mysymftype = _Lstat(_Mypval._Ptr(), _Errno);
		return (file_status(_Mysymftype));
		}

	file_status symlink_status(error_code& _Code) const
		{	// get file symlink status
		int _Errno = 0;
		if (_Mysymftype == status_unknown)
			_Mysymftype = _Lstat(_Mypval._Ptr(), _Errno);
		_Code = error_code(_Errno, _STD system_category());
		return (file_status(_Mysymftype));
		}

	bool operator==(const _Myt& _Right) const
		{	// test if *this == _Right
		return (_Mypval == _Right._Mypval);
		}

	bool operator!=(const _Myt& _Right) const
		{	// test if *this == _Right
		return (!(*this == _Right));
		}

	bool operator<(const _Myt& _Right) const
		{	// test if *this < _Right
		return (_Mypval < _Right._Mypval);
		}

	bool operator>(const _Myt& _Right) const
		{	// test if *this > _Right
		return (_Right < *this);
		}
	bool operator<=(const _Myt& _Right) const
		{	// test if *this <= _Right
		return (!(_Right < *this));
		}

	bool operator>=(const _Myt& _Right) const
		{	// test if *this <= _Right
		return (!(*this < _Right));
		}

private:
	path_type _Mypval;
	mutable file_type _Myftype;
	mutable file_type _Mysymftype;
	};

typedef basic_directory_entry<path> directory_entry;
typedef basic_directory_entry<wpath> wdirectory_entry;

template<class _Path>
	class basic_directory_iterator
		: public iterator<input_iterator_tag, basic_directory_entry<_Path> >
	{	// walks a directory
public:
	typedef basic_directory_iterator<_Path> _Myt;
	typedef basic_directory_entry<_Path> value_type;
	typedef typename _Path::string_type string_type;
	typedef _Path path_type;
	typedef const value_type *pointer;

	basic_directory_iterator()
		{	// default construct
		_Mypdir = 0;
		}

	basic_directory_iterator(const _Path& _Pval)
		{	// construct from _Pval
		typename string_type::value_type _Dest[_MAX_FILESYS_NAME];
		int _Errno = 0;
		file_type _Ftype;

		_Mypdir = _Open_dir(_Dest, _Pval._Ptr(), _Errno, _Ftype);
		if (_Mypdir != 0)
			_Myentry.assign(_Dest, file_status(_Ftype));
		}

	basic_directory_iterator(const _Path& _Pval, error_code& _Code)
		{	// construct from _Pval, errors to _Code
		typename string_type::value_type _Dest[_MAX_FILESYS_NAME];
		int _Errno = 0;
		file_type _Ftype;

		_Mypdir = _Open_dir(_Dest, _Pval._Ptr(), _Errno, _Ftype);
		if (_Mypdir != 0)
			_Myentry.assign(_Dest, file_status(_Ftype));
		_Code = error_code(_Errno, _STD system_category());
		}

	~basic_directory_iterator() 
		{	// destroy the object
		_Close();
		}

	basic_directory_iterator(const _Myt& _Right);	// not defined
	_Myt& operator=(const _Myt& _Right);	// not defined

	basic_directory_iterator(_Myt&& _Right)
		: _Mypdir(_Right._Mypdir),
			_Myentry(_STD move(_Right._Myentry))
		{	// move construct
		_Right._Mypdir = 0;
		}

	_Myt& operator=(_Myt&& _Right)
		{	// move assign
		if (this != &_Right)
			{	// different, move
			_Mypdir = _Right._Mypdir;
			_Myentry = _STD move(_Right._Myentry);
			_Right._Mypdir = 0;
			}
		return (*this);
		}

	const value_type& operator*() const
		{	// return designated value
 #if _ITERATOR_DEBUG_LEVEL == 2
		if (_Mypdir == 0)
			_DEBUG_ERROR(
				"basic_directory_iterator is not dereferencable");

 #elif _ITERATOR_DEBUG_LEVEL == 1
		_SCL_SECURE_VALIDATE(_Mypdir != 0);
 #endif /* _ITERATOR_DEBUG_LEVEL */

		return (_Myentry);
		}

	pointer operator->() const
		{	// return pointer to class object
		return &(_Myentry);
		}

	_Myt& operator++()
		{	// preincrement
 #if _ITERATOR_DEBUG_LEVEL == 2
		if (_Mypdir == 0)
			_DEBUG_ERROR(
				"basic_directory_iterator is not incrementable");

 #elif _ITERATOR_DEBUG_LEVEL == 1
		_SCL_SECURE_VALIDATE(_Mypdir != 0);
 #endif /* _ITERATOR_DEBUG_LEVEL */

		_Get();
		return (*this);
		}

	_Myt operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	bool _Equal(const _Myt& _Right) const
		{	// test for equality
		return (_Mypdir == _Right._Mypdir);
		}

private:
	void _Close()
		{	// close directory
		if (_Mypdir != 0)
			_Close_dir(_Mypdir);
		_Mypdir = 0;
		}
		
	void _Get()
		{	// peek at next input element
		if (_Mypdir != 0)
			{	// read a directory entry
			typename string_type::value_type _Dest[_MAX_FILESYS_NAME];
			file_type _Ftype;
			string_type _Str = _Read_dir(_Dest, _Mypdir, _Ftype);
			if (_Str.empty())
				_Close();	// end of directory
			else
				_Myentry.assign(_Str, file_status(_Ftype));
			}
		}

	void *_Mypdir;
	value_type _Myentry;
	};

template<class _Path> inline
	bool operator==(
		const basic_directory_iterator<_Path>& _Left,
		const basic_directory_iterator<_Path>& _Right)
	{	// test for basic_directory_iterator equality
	return (_Left._Equal(_Right));
	}

template<class _Path> inline
	bool operator!=(
		const basic_directory_iterator<_Path>& _Left,
		const basic_directory_iterator<_Path>& _Right)
	{	// test for basic_directory_iterator inequality
	return (!(_Left == _Right));
	}

typedef basic_directory_iterator<path> directory_iterator;
typedef basic_directory_iterator<wpath> wdirectory_iterator;

		// TEMPLATE CLASS basic_recursive_directory_iterator
template<class _Path>
	struct _Directory_level
	{	// records level information
	_Directory_level *_Prev;
	void *_Mypdir;
	_Path _Dir;
	};

template<class _Path>
	class basic_recursive_directory_iterator
		: public iterator<input_iterator_tag, basic_directory_entry<_Path> >
	{	// walks a directory
public:
	typedef basic_recursive_directory_iterator<_Path> _Myt;
	typedef basic_directory_entry<_Path> value_type;
	typedef typename _Path::string_type string_type;
	typedef typename string_type::value_type char_type;
	typedef _Path path_type;
	typedef const value_type *pointer;

	basic_recursive_directory_iterator()
		: _Prev(0), _No_push(false)
		{	// default construct
		_Mypdir = 0;
		}

	basic_recursive_directory_iterator(const _Path& _Pval)
		: _Prev(0), _No_push(false)
		{	// construct from _Pval
		char_type _Dest[_MAX_FILESYS_NAME];
		int _Errno = 0;
		file_type _Ftype;

		_Dir = _Pval;
		_Mypdir = _Open_dir(_Dest, _Pval._Ptr(), _Errno, _Ftype);
		_Get(_Dest, _Ftype);
		}

	basic_recursive_directory_iterator(const _Path& _Pval, error_code& _Code)
		: _Prev(0), _No_push(false)
		{	// construct from _Pval, errors to _Code
		char_type _Dest[_MAX_FILESYS_NAME];
		int _Errno = 0;
		file_type _Ftype;

		_Dir = _Pval;
		_Mypdir = _Open_dir(_Dest, _Pval._Ptr(), _Errno, _Ftype);
		_Get(_Dest, _Ftype);
		_Code = error_code(_Errno, _STD system_category());
		}

	~basic_recursive_directory_iterator() 
		{	// destroy the object
		_Close();
		}

	basic_recursive_directory_iterator(const _Myt& _Right);	// not defined
	_Myt& operator=(const _Myt& _Right);	// not defined

	basic_recursive_directory_iterator(_Myt&& _Right)
		: _Prev(_Right._Prev),
			_No_push(_Right._No_push),
			_Mypdir(_Right._Mypdir),
			_Dir(_STD move(_Right._Dir)),
			_Myentry(_STD move(_Right._Myentry))
		{	// move construct
		_Right._Mypdir = 0;
		}

	_Myt& operator=(_Myt&& _Right)
		{	// move assign
		if (this != &_Right)
			{ // different, move
			_Prev = _Right._Prev;
			_No_push = _Right._No_push;
			_Mypdir = _Right._Mypdir;
			_Dir = _STD move(_Right._Dir);
			_Myentry = _STD move(_Right._Myentry);
			_Right._Mypdir = 0;
			}
		return (*this);
		}

	int level() const
		{	// get recursion depth
		int _Ans = 0;
		for (const _Directory_level<_Path> *_Plevel = _Prev; _Plevel != 0;
			_Plevel = _Plevel->_Prev)
			++_Ans;
		return (_Ans);
		}

	void pop()
		{	// pop a level
		_Close();
		if (_Prev != 0)
			{	// level to pop, do it
			_Directory_level<_Path> *_Ptr = _Prev;
			_Prev = _Ptr->_Prev;
			_No_push = false;
			_Mypdir = _Ptr->_Mypdir;
			_Dir = _Ptr->_Dir;
			delete _Ptr;
			_Get();
			}
		}

	void no_push()
		{	// disable directory recursion
		_No_push = true;
		}

	 const value_type& operator*() const
		{	// return designated value
 #if _ITERATOR_DEBUG_LEVEL == 2
		if (_Mypdir == 0)
			_DEBUG_ERROR(
				"basic_recursive_directory_iterator is not dereferencable");

 #elif _ITERATOR_DEBUG_LEVEL == 1
		_SCL_SECURE_VALIDATE(_Mypdir != 0);
 #endif /* _ITERATOR_DEBUG_LEVEL */

		return (_Myentry);
		}

	pointer operator->() const
		{	// return pointer to class object
		return (_STD pointer_traits<pointer>::pointer_to(**this));
		}

	_Myt& operator++()
		{	// preincrement
 #if _ITERATOR_DEBUG_LEVEL == 2
		if (_Mypdir == 0)
			_DEBUG_ERROR(
				"basic_recursive_directory_iterator is not incrementable");

 #elif _ITERATOR_DEBUG_LEVEL == 1
		_SCL_SECURE_VALIDATE(_Mypdir != 0);
 #endif /* _ITERATOR_DEBUG_LEVEL */

		_Get();
		return (*this);
		}

	_Myt operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	bool _Equal(const _Myt& _Right) const
		{	// test for equality
		return (_Mypdir == _Right._Mypdir);
		}

private:
	void _Close()
		{	// close directory
		if (_Mypdir != 0)
			_Close_dir(_Mypdir);
		_Mypdir = 0;
		_Myentry.assign(string_type());
		}
		
	void _Get(char_type *_Pstr = 0, file_type _Ftype = status_unknown)
		{	// peek at next input element
		if (_Mypdir != 0)
			{	// read a directory entry
			char_type _Dest[_MAX_FILESYS_NAME];
			if (_Pstr == 0)
				_Pstr = _Read_dir(_Dest, _Mypdir, _Ftype);
			if (_Pstr[0] != char_type(0))
				_Myentry.assign(_Dir / path_type(_Pstr), file_status(_Ftype));
			else
				pop();

			void *_Newpdir = 0;
			int _Errno = 0;

			if (!_No_push && is_directory(_Myentry.status()))
				{	// try to push a level
				path_type _Pval = _Myentry.path();
				if ((_Newpdir =
					_Open_dir(_Dest, _Pval._Ptr(), _Errno, _Ftype)) == 0)
					_Get();	// skip empty directory
				else
					{	// push a level
					_Directory_level<_Path> *_Newp =
						new _Directory_level<_Path>;
					_Newp->_Prev = _Prev;
					_Newp->_Mypdir = _Mypdir;
					_Newp->_Dir = _Dir;
					_Prev = _Newp;

					_Mypdir = _Newpdir;
					_Dir = _Myentry.path();
					_Myentry.assign(_Dir / path_type(_Dest),
					file_status(_Ftype));
					}
				}
			}
		}

	_Directory_level<_Path> *_Prev;
	bool _No_push;
	void *_Mypdir;
	path_type _Dir;
	value_type _Myentry;
	};

template<class _Path> inline
	bool operator==(
		const basic_recursive_directory_iterator<_Path>& _Left,
		const basic_recursive_directory_iterator<_Path>& _Right)
	{	// test for basic_recursive_directory_iterator equality
	return (_Left._Equal(_Right));
	}

template<class _Path> inline
	bool operator!=(
		const basic_recursive_directory_iterator<_Path>& _Left,
		const basic_recursive_directory_iterator<_Path>& _Right)
	{	// test for basic_recursive_directory_iterator inequality
	return (!(_Left == _Right));
	}

typedef basic_recursive_directory_iterator<path>
	recursive_directory_iterator;
typedef basic_recursive_directory_iterator<wpath>
	wrecursive_directory_iterator;

		// FUNCTIONS status AND symlink_status
template<class _Path> inline
	file_status status(const _Path& _Pval)
	{	// get file status
	int _Errno = 0;
	file_type _Ftype = _Stat(_Pval._Ptr(), _Errno);

	return (file_status(_Ftype));
	}

template<class _Path> inline
	file_status status(const _Path& _Pval, error_code& _Code)
	{	// get file status
	int _Errno = 0;
	file_type _Ftype = _Stat(_Pval._Ptr(), _Errno);

	_Code = error_code(_Errno, _STD system_category());
	return (file_status(_Ftype));
	}

template<class _Path> inline
	file_status symlink_status(const _Path& _Pval)
	{	// get symlink file status
	int _Errno = 0;
	file_type _Ftype = _Lstat(_Pval._Ptr(), _Errno);

	return (file_status(_Ftype));
	}

template<class _Path> inline
	file_status symlink_status(const _Path& _Pval, error_code& _Code)
	{	// get symlink file status
	int _Errno = 0;
	file_type _Ftype = _Lstat(_Pval._Ptr(), _Errno);

	_Code = error_code(_Errno, _STD system_category());
	return (file_status(_Ftype));
	}

		// PREDICATES
inline bool status_known(file_status _Stat)
	{	// test if status known
	return (_Stat.type() != status_unknown);
	}

inline bool exists(file_status _Stat)
	{	// test if file exists
	return (status_known(_Stat) && _Stat.type() != file_not_found);
	}

inline bool is_regular(file_status _Stat)
	{	// test for regular file
	return (_Stat.type() == regular_file);
	}

inline bool is_regular_file(file_status _Stat)
	{	// test for regular file
	return (is_regular(_Stat));
	}

inline bool is_directory(file_status _Stat)
	{	// test for directory
	return (_Stat.type() == directory_file);
	}

inline bool is_symlink(file_status _Stat)
	{	// test for symlink
	return (_Stat.type() == symlink_file);
	}

inline bool is_other(file_status _Stat)
	{	// test for other file types
	return (exists(_Stat)
		&& !is_regular(_Stat) && !is_directory(_Stat) && !is_symlink(_Stat));
	}

template<class _Path> inline
	bool exists(const _Path& _Pval)
	{	// test if path exists
	return (exists(status(_Pval)));
	}

template<class _Path> inline
	bool is_regular(const _Path& _Pval)
	{	// test if path is regular
	return (is_regular(status(_Pval)));
	}

template<class _Path> inline
	bool is_regular_file(const _Path& _Pval)
	{	// test if path is regular
	return (is_regular(_Pval));
	}

template<class _Path> inline
	bool is_directory(const _Path& _Pval)
	{	// test if path is directory
	return (is_directory(status(_Pval)));
	}

template<class _Path> inline
	bool is_symlink(const _Path& _Pval)
	{	// test if path is symlink
	return (is_symlink(status(_Pval)));
	}

template<class _Path> inline
	bool is_other(const _Path& _Pval)
	{	// test if path is other file type
	return (is_other(status(_Pval)));
	}

template<class _Path> inline
	bool is_empty(const _Path& _Pval)
	{	// test if path is empty
	file_status _Stat = status(_Pval);

	if (!exists(_Stat))
		_THROW_NCEE(basic_filesystem_error<_Path>,
			"is_empty(p): invalid argument");
	else if (is_directory(_Stat))
		return (basic_directory_iterator<_Path>(_Pval)
			== basic_directory_iterator<_Path>());
	else
		return (file_size(_Pval) == 0);
	}

template<class _Path1,
	class _Path2> inline
	bool equivalent(const _Path1& _Pval1, const _Path2& _Pval2)
	{	// test if paths designate same file
	int _Ans = _Equivalent(_Pval1._Ptr(), _Pval2._Ptr());

	if (_Ans < 0)
		_THROW_NCEE(basic_filesystem_error<_Path1>,
			"equivalent(p1, p2): invalid arguments");
	return (0 < _Ans);
	}

		// ATTRIBUTES
template<class _Path> inline
	_Path current_path()
	{	// get current working directory
	typename _Path::value_type _Dest[_MAX_FILESYS_NAME];
	return (_Current_get(_Dest));
	}

template<class _Path> inline
	void current_path(const _Path& _Pval)
	{	// set current working directory
	_Current_set(_Pval._Ptr());
	}

template<class _Path>
	class _Wrap_init_dir
	{	// wrap the initial directory
public:
	static _Path _Mypath;	// save path at program startup
	};

template<class _Path>
	_Path _Wrap_init_dir<_Path>::_Mypath = current_path<_Path>();

template<class _Path> inline
	_Path initial_path()
	{	// get initial working directory
	return (_Wrap_init_dir<_Path>::_Mypath);
	}

template<class _Path> inline
	_ULonglong file_size(const _Path& _Pval)
	{	// get file size
	return (_File_size(_Pval._Ptr()));
	}

template<class _Path> inline
	space_info space(const _Path& _Pval)
	{	// get space information for volume
	return (_Statvfs(_Pval._Ptr()));
	}

template<class _Path> inline
	time_t last_write_time(const _Path& _Pval)
	{	// get last write time
	return (_Last_write_time(_Pval._Ptr()));
	}

template<class _Path> inline
	void last_write_time(const _Path& _Pval, time_t _Newtime)
	{	// set last write time
	_Last_write_time(_Pval._Ptr(), _Newtime);
	}

		// OPERATIONS
template<class _Path> inline
	bool create_directory(const _Path& _Pval)
	{	// create a directory
	int _Ans = _Make_dir(_Pval._Ptr());

	if (_Ans < 0)
		_THROW_NCEE(basic_filesystem_error<_Path>,
			"create_directory(p): invalid argument");
	else
		return (0 < _Ans);
	}

template<class _Path> inline
	bool remove_directory(const _Path& _Pval)
	{	// remove a directory
	return (_Remove_dir(_Pval._Ptr()));
	}

template<class _Path1,
	class _Path2> inline
	void create_hard_link(const _Path1& _Oldpval,
		const _Path2& _Newpval)
	{	// link _Newpval to _Oldpval
	if (_Link(_Oldpval._Ptr(), _Newpval._Ptr()) != 0)
		_THROW_NCEE(basic_filesystem_error<_Path1>,
			"create_hard_link(p1, p2): invalid arguments");
	}

template<class _Path1,
	class _Path2> inline
	bool create_hard_link(const _Path1& _Oldpval,
		const _Path2& _Newpval,
		error_code& _Code)
	{	// link _Newpval to _Oldpval
	int _Ans = _Link(_Oldpval._Ptr(), _Newpval._Ptr());
	_Code = error_code(_Ans, _STD system_category());
	return (_Ans == 0);
	}

template<class _Path1,
	class _Path2> inline
	void create_sym_link(const _Path1& _Oldpval,
		const _Path2& _Newpval)
	{	// symlink _Newpval to _Oldpval
	if (_Symlink(_Oldpval._Ptr(), _Newpval._Ptr()) != 0)
		_THROW_NCEE(basic_filesystem_error<_Path1>,
			"create_sym_link(p1, p2): invalid arguments");
	}

template<class _Path1,
	class _Path2> inline
	bool create_sym_link(const _Path1& _Oldpval,
		const _Path2& _Newpval,
		error_code& _Code)
	{	// symlink _Newpval to _Oldpval
	int _Ans = _Link(_Oldpval._Ptr(), _Newpval._Ptr());
	_Code = error_code(_Ans, _STD system_category());
	return (_Ans == 0);
	}

template<class _Path> inline
	bool remove(const _Path& _Pval)
	{	// remove _Pval
	return (_Unlink(_Pval._Ptr()) == 0);
	}

template<class _Path> inline
	bool remove_filename(const _Path& _Pval)
	{	// remove _Pval
	return (remove(_Pval));
	}

template<class _Path> inline
	unsigned long remove_all(const _Path& _Pval)
	{	// recursively remove a file or directory
	unsigned long _Ans = 0;

	if (is_directory(_Pval))
		{	// empty and remove a directory
		typedef basic_directory_iterator<_Path> _Myit;
		_Myit _Last;

		for (; ; )
			{	// remove a directory entry
			_Myit _First(_Pval);
			if (_First == _Last)
				break;
			_Ans += remove_all(_Pval / _First->path());
			}
		remove_directory(_Pval);
		}
	else if (remove(_Pval))
		++_Ans;

	return (_Ans);
	}

template<class _Path1,
	class _Path2> inline
	bool rename(const _Path1& _Oldpval,
		const _Path2& _Newpval)
	{	// rename _Oldpval as _Newpval
	return (_Rename(_Oldpval._Ptr(), _Newpval._Ptr()) == 0);
	}

		// ENUM copy_option

	namespace copy_option {
enum copy_option {	// names for copy options
	fail_if_exists,
	overwrite_if_exists
	};
	}	// namespace copy_option

typedef copy_option::copy_option _Copy_option;

template<class _Path1,
	class _Path2> inline
	void copy_file(const _Path1& _Oldpval,
		const _Path2& _Newpval,
		_Copy_option _Opt = copy_option::fail_if_exists)
	{	// copy _Oldpval to _Newpval, with attributes
	if (_Copy_file(_Oldpval._Ptr(), _Newpval._Ptr(),
		(int)_Opt == (int)copy_option::fail_if_exists))
		_THROW_NCEE(basic_filesystem_error<_Path1>,
			"copy(p1, p2): invalid arguments");
	}

template<class _Path> inline
	_Path complete(const _Path& _Pval,
		const _Path& _Pbase = initial_path<_Path>())
	{	// complete _Pval with _Pbase
	if (_Pval.has_root_name())
		return (_Pval);
	else if (_Pval.has_root_directory())
		return (_Path(_Pbase.root_name()) / _Pval);
	else
		return (_Pbase / _Pval);
	}

template<class _Path> inline
	_Path system_complete(const _Path& _Pval)
	{	// complete _Pval
	return (_Pval.is_complete() ? _Pval : complete(_Pval,
		current_path<_Path>()));
	}

template<class _Path> inline
	bool create_directories(const _Path& _Pval)
	{	// create directory chain
	if (_Pval.empty())
		return (false);
	else if (!exists(_Pval))
		{	// recursively create parent directories, then current
		create_directories(_Pval.branch_path());
		create_directory(_Pval);
		return (true);
		}
	else if (is_directory(_Pval))
		return (false);	// directory already there
	else
		_THROW_NCEE(basic_filesystem_error<_Path>,
			"create_directories(p): bad argument");
	}

template<class _Path> inline
	typename _Path::string_type extension(const _Path& _Pval)
	{	// pick off .extension in leaf, including dot
	return (_Pval.extension());
	}

template<class _Path> inline
	typename _Path::string_type basename(const _Path& _Pval)
	{	// pick off base name in leaf before dot
	return (_Pval.stem());
	}

template<class _Path> inline
	typename _Path::string_type stem(const _Path& _Pval)
	{	// pick off base name in leaf before dot
	return (basename(_Pval));
	}

template<class _Path> inline
	_Path replace_extension(const _Path& _Pval,
		const typename _Path::string_type& _Newext)
	{	// replace extension with _Newext
	_Path _Ans = _Pval;
	return (_Ans.replace_extension(_Newext));
	}
	}	//namespace 
}	// namespace 

 #pragma warning(pop)
 #pragma pack(pop)
#endif /* RC_INVOKED */
#endif /* _FILESYSTEM_ */

/*
 * Copyright (c) 1992-2011 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.40:0009 */
