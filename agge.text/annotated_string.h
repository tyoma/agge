#pragma once

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
		struct element_type;
		class const_iterator;

	public:
		annotated_string();
		annotated_string(const CharT *from);

		void operator +=(const string_type &addition);
		void annotate(const AnnotationT &annotation);

		bool empty() const;
		size_t size() const;
		const_iterator begin() const;
		const_iterator end() const;

	private:
		typedef std::vector< std::pair<AnnotationT, size_t /*position*/> > annotations_t;

	private:
		string_type _underlying;
		annotations_t _annotations;
	};

	template <typename CharT, typename AnnotationT>
	struct annotated_string<CharT, AnnotationT>::element_type
	{
		const typename annotated_string<CharT, AnnotationT>::string_type *underlying;
		size_t index;
		const AnnotationT *annotation;

		operator CharT() const;
	};

	template <typename CharT, typename AnnotationT>
	class annotated_string<CharT, AnnotationT>::const_iterator
	{
	public:
		const_iterator(const typename annotated_string<CharT, AnnotationT>::string_type &underlying, size_t index,
			const typename annotated_string<CharT, AnnotationT>::annotations_t &annotations);

		typedef std::forward_iterator_tag iterator_category;
		typedef typename annotated_string<CharT, AnnotationT>::element_type value_type;
		typedef ptrdiff_t difference_type;
		typedef const typename annotated_string<CharT, AnnotationT>::element_type *pointer;
		typedef void reference; // The iterator does not provide stable references.

	public:
		const typename annotated_string<CharT, AnnotationT>::element_type &operator *() const;
		const typename annotated_string<CharT, AnnotationT>::element_type *operator ->() const;
		const_iterator &operator ++();
		bool operator ==(const const_iterator &rhs) const;
		bool operator !=(const const_iterator &rhs) const;

	private:
		void seek_annotation();

	private:
		typename annotated_string<CharT, AnnotationT>::element_type _current;
		const typename annotated_string<CharT, AnnotationT>::annotations_t *_annotations;
		typename annotated_string<CharT, AnnotationT>::annotations_t::const_iterator _next_annotation;
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
		annotated_string<CharT, AnnotationT>::begin() const
	{	return const_iterator(_underlying, 0u, _annotations);	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::const_iterator
		annotated_string<CharT, AnnotationT>::end() const
	{	return const_iterator(_underlying, _underlying.size(), _annotations);	}


	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::element_type::operator CharT() const
	{	return (*underlying)[index];	}


	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::const_iterator::const_iterator(
			const typename annotated_string<CharT, AnnotationT>::string_type &underlying, size_t index,
			const typename annotated_string<CharT, AnnotationT>::annotations_t &annotations)
		: _annotations(&annotations), _next_annotation(annotations.begin())
	{
		_current.underlying = &underlying;
		_current.index = index;
		seek_annotation();
	}

	template <typename CharT, typename AnnotationT>
	inline const typename annotated_string<CharT, AnnotationT>::element_type
		&annotated_string<CharT, AnnotationT>::const_iterator::operator *() const
	{	return _current;	}

	template <typename CharT, typename AnnotationT>
	inline const typename annotated_string<CharT, AnnotationT>::element_type
		*annotated_string<CharT, AnnotationT>::const_iterator::operator ->() const
	{	return &_current;	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::const_iterator
		&annotated_string<CharT, AnnotationT>::const_iterator::operator ++()
	{	return _current.index++, seek_annotation(), *this;	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::const_iterator::operator ==(const const_iterator &rhs) const
	{	return _current.index == rhs._current.index;	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::const_iterator::operator !=(const const_iterator &rhs) const
	{	return !(*this == rhs);	}

	template <typename CharT, typename AnnotationT>
	inline void annotated_string<CharT, AnnotationT>::const_iterator::seek_annotation()
	{
		_current.annotation = _annotations->end() != _next_annotation && _current.index == _next_annotation->second
			? &_next_annotation++->first : 0;
	}
}
