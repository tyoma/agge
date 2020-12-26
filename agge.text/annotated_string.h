#pragma once

#include "range.h"

#include <iterator>
#include <string>
#include <vector>

namespace agge
{
	template <typename CharT, typename AnnotationT>
	class annotated_string
	{
	public:
		typedef std::basic_string<CharT> string_type;
		class range;
		class const_iterator;

	public:
		annotated_string();
		annotated_string(const CharT *from);

		void operator +=(const string_type &addition);
		void annotate(const AnnotationT &annotation);

		bool empty() const;
		size_t size() const;
		const_iterator ranges_begin() const;
		const_iterator ranges_end() const;

	private:
		typedef std::vector< std::pair<AnnotationT, size_t /*position*/> > annotations_t;

	private:
		string_type _underlying;
		annotations_t _annotations;
	};

	template <typename CharT, typename AnnotationT>
	class annotated_string<CharT, AnnotationT>::range : public agge::range<const string_type>
	{
	public:
		range(const string_type &from);

	public:
		const AnnotationT *annotation;
	};

	template <typename CharT, typename AnnotationT>
	class annotated_string<CharT, AnnotationT>::const_iterator
	{
	public:
		const_iterator(const range &from, typename annotations_t::const_iterator next_annotation,
			typename annotations_t::const_iterator annotations_end);

		typedef std::forward_iterator_tag iterator_category;
		typedef range value_type;
		typedef ptrdiff_t difference_type;
		typedef void pointer; // The iterator does not provide stable pointers.
		typedef const range &reference;

	public:
		range operator *() const;
		const range *operator ->() const;
		const_iterator &operator ++();
		bool operator ==(const const_iterator &rhs) const;
		bool operator !=(const const_iterator &rhs) const;

	private:
		void fetch_annotation();

	private:
		range _current;
		const annotations_t *_annotations;
		typename annotations_t::const_iterator _next_annotation, _annotations_end;
	};



	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::annotated_string()
	{	}

	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::annotated_string(const CharT *from)
		: _underlying(from)
	{	}

	template <typename CharT, typename AnnotationT>
	inline void annotated_string<CharT, AnnotationT>::operator +=(const string_type &addition)
	{	_underlying += addition;	}

	template <typename CharT, typename AnnotationT>
	inline void annotated_string<CharT, AnnotationT>::annotate(const AnnotationT &annotation)
	{
		const size_t position = _underlying.size();

		if (!_annotations.empty() && _annotations.back().second == position)
			_annotations.back().first = annotation;
		else
			_annotations.push_back(std::make_pair(annotation, position));
	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::empty() const
	{	return _underlying.empty();	}

	template <typename CharT, typename AnnotationT>
	inline size_t annotated_string<CharT, AnnotationT>::size() const
	{	return _underlying.size();	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::const_iterator
		annotated_string<CharT, AnnotationT>::ranges_begin() const
	{	return const_iterator(range(_underlying), _annotations.begin(), _annotations.end());	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::const_iterator
		annotated_string<CharT, AnnotationT>::ranges_end() const
	{
		range r(_underlying);

		return r.set_end(), const_iterator(r, _annotations.end(), _annotations.end());
	}


	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::range::range(const string_type &from)
		: agge::range<const string_type>(from)
	{	}


	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::const_iterator::const_iterator(const range &from,
			typename annotations_t::const_iterator next_annotation,
			typename annotations_t::const_iterator annotations_end)
		: _current(from), _next_annotation(next_annotation), _annotations_end(annotations_end)
	{
		if (_annotations_end == _next_annotation)
			_current.annotation = 0, _current.extend_end();
		else if (size_t start = _next_annotation->second)
			_current.annotation = 0, _current.end_index = start;
		else
			fetch_annotation();
	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::range
		annotated_string<CharT, AnnotationT>::const_iterator::operator *() const
	{	return _current;	}

	template <typename CharT, typename AnnotationT>
	inline const typename annotated_string<CharT, AnnotationT>::range
		*annotated_string<CharT, AnnotationT>::const_iterator::operator ->() const
	{	return &_current;	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::const_iterator
		&annotated_string<CharT, AnnotationT>::const_iterator::operator ++()
	{
		_current.begin_index = _current.end_index;
		if (_annotations_end != _next_annotation)
			fetch_annotation();
		return *this;
	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::const_iterator::operator ==(const const_iterator &rhs) const
	{	return _current == rhs._current;	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::const_iterator::operator !=(const const_iterator &rhs) const
	{	return !(*this == rhs);	}

	template <typename CharT, typename AnnotationT>
	inline void annotated_string<CharT, AnnotationT>::const_iterator::fetch_annotation()
	{
		_current.annotation = &_next_annotation++->first;
		if (_annotations_end != _next_annotation)
			_current.end_index = _next_annotation->second;
		else
			_current.extend_end();
	}
}
