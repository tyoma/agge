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
		struct range;
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
	struct annotated_string<CharT, AnnotationT>::range
	{
		typename annotated_string<CharT, AnnotationT>::string_type::const_iterator begin, end;
		const AnnotationT *annotation;
	};

	template <typename CharT, typename AnnotationT>
	class annotated_string<CharT, AnnotationT>::const_iterator
	{
	public:
		const_iterator(const typename annotated_string<CharT, AnnotationT>::string_type &underlying,
			const typename annotated_string<CharT, AnnotationT>::annotations_t *annotations);

		typedef std::forward_iterator_tag iterator_category;
		typedef typename annotated_string<CharT, AnnotationT>::range value_type;
		typedef ptrdiff_t difference_type;
		typedef void pointer; // The iterator does not provide stable pointers.
		typedef void reference; // The iterator does not provide stable references.

	public:
		typename annotated_string<CharT, AnnotationT>::range operator *() const;
		const typename annotated_string<CharT, AnnotationT>::range *operator ->() const;
		const_iterator &operator ++();
		bool operator ==(const const_iterator &rhs) const;
		bool operator !=(const const_iterator &rhs) const;

	private:
		void seek_annotation();

	private:
		range _current;
		const typename annotated_string<CharT, AnnotationT>::string_type *_underlying;
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
		annotated_string<CharT, AnnotationT>::ranges_begin() const
	{	return const_iterator(_underlying, &_annotations);	}

	template <typename CharT, typename AnnotationT>
	inline typename annotated_string<CharT, AnnotationT>::const_iterator
		annotated_string<CharT, AnnotationT>::ranges_end() const
	{	return const_iterator(_underlying, 0);	}


	template <typename CharT, typename AnnotationT>
	inline annotated_string<CharT, AnnotationT>::const_iterator::const_iterator(
			const typename annotated_string<CharT, AnnotationT>::string_type &underlying,
			const typename annotated_string<CharT, AnnotationT>::annotations_t *annotations)
		: _underlying(&underlying), _annotations(!underlying.empty() ? annotations : 0)
	{
		_current.begin = underlying.begin();
		_current.end = underlying.end();
		_current.annotation = 0;
		if (_annotations)
		{
			_next_annotation = _annotations->begin();
			if (_annotations->end() != _next_annotation)
			{
				if (!_next_annotation->second)
					_current.annotation = &_next_annotation++->first;
				_current.end = _annotations->end() != _next_annotation ? _underlying->begin() + _next_annotation->second
					: _underlying->end();
			}
		}
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
		if (_annotations && _underlying->end() != _current.end && _annotations->end() != _next_annotation)
		{
			_current.begin = _current.end;
			_current.annotation = &_next_annotation++->first;
			_current.end = _annotations->end() != _next_annotation ? _underlying->begin() + _next_annotation->second
				: _underlying->end();
		}
		else
		{
			_annotations = 0;
		}
		return *this;
	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::const_iterator::operator ==(const const_iterator &rhs) const
	{	return !!_annotations == !!rhs._annotations && (!_annotations || _current.begin == rhs._current.begin);	}

	template <typename CharT, typename AnnotationT>
	inline bool annotated_string<CharT, AnnotationT>::const_iterator::operator !=(const const_iterator &rhs) const
	{	return !(*this == rhs);	}
}
