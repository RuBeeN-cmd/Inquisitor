#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <utils/ansi_color.h>
#include <stdio.h>
#include <stddef.h>

typedef enum e_log_level {
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARNING,
	LEVEL_ERROR
} t_log_level;

#define DBG_PREFIX	"DBG"
#define INFO_PREFIX	"INF"
#define WARN_PREFIX	"WRN"
#define ERR_PREFIX	"ERR"

#define LOG(_is_prefix_, _level_, _format_, ...) do { \
	if (_level_ >= *get_log_level()) { \
		if (_level_ <= LEVEL_ERROR) { \
			char *_prefix_ = _is_prefix_ ? get_log_prefix(_level_) : ""; \
			int _fd_ = _level_ >= LEVEL_WARNING ? 2 : 1; \
			dprintf(_fd_, "%s" _format_, _prefix_, ##__VA_ARGS__); \
		} \
	} \
} while(0)

#define DBG(_format_, ...) LOG(1, LEVEL_DEBUG, _format_, ##__VA_ARGS__)
#define INFO(_format_, ...) LOG(1, LEVEL_INFO, _format_, ##__VA_ARGS__)
#define WARN(_format_, ...) LOG(1, LEVEL_WARNING, _format_, ##__VA_ARGS__)
#define ERR(_format_, ...) LOG(1, LEVEL_ERROR, _format_, ##__VA_ARGS__)

t_log_level *get_log_level();
void		set_log_level(t_log_level level);

char 	*get_log_prefix(t_log_level level);
void	disable_prefix();

void	parse_env(char *env[]);

#endif