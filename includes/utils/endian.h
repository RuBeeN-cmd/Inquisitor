#ifndef UTILS_ENDIAN_H
#define UTILS_ENDIAN_H

#include <endian.h>

#define SWAP_ENDIAN(_endian_, _data_, _size_) \
	do { \
		int _crt_endian_ = get_endian(); \
		if (_crt_endian_ != (_endian_)) { \
			switch (_size_) \
			{ \
			case 2: \
				_data_ = __builtin_bswap16(_data_); \
				break; \
			case 4: \
				_data_ = __builtin_bswap32(_data_); \
				break; \
			case 8: \
				_data_ = __builtin_bswap64(_data_); \
				break; \
			default: \
				break; \
			} \
		} \
	} while (0)

#define SWAP_ENDIAN_ARRAY(_endian_, _data_, _size_, _nmenb_) \
	for (size_t _i_= 0; _i_ < _nmenb_; _i_++) \
	{ \
		SWAP_ENDIAN(_endian_, _data_[_i_], _size_); \
	}

int get_endian(void);

#endif