#pragma once

template <typename Type>
EZ_ALWAYS_INLINE ezSizeTemplate<Type>::ezSizeTemplate() = default;

template <typename Type>
EZ_ALWAYS_INLINE ezSizeTemplate<Type>::ezSizeTemplate(Type width, Type height)
  : width(width)
  , height(height)
{
}

template <typename Type>
EZ_ALWAYS_INLINE bool ezSizeTemplate<Type>::HasNonZeroArea() const
{
  return (width > 0) && (height > 0);
}

template <typename Type>
EZ_ALWAYS_INLINE bool operator==(const ezSizeTemplate<Type>& v1, const ezSizeTemplate<Type>& v2)
{
  return v1.height == v2.height && v1.width == v2.width;
}

template <typename Type>
EZ_ALWAYS_INLINE bool operator!=(const ezSizeTemplate<Type>& v1, const ezSizeTemplate<Type>& v2)
{
  return v1.height != v2.height || v1.width != v2.width;
}
