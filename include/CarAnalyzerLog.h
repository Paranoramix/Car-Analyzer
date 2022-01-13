#ifndef __CAR_ANALYZER_LOG_H__
#define __CAR_ANALYZER_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "sdkconfig.h"

#define CAR_ANALYZER_LOG_LEVEL_NONE (0)
#define CAR_ANALYZER_LOG_LEVEL_ERROR (1)
#define CAR_ANALYZER_LOG_LEVEL_WARN (2)
#define CAR_ANALYZER_LOG_LEVEL_INFO (3)
#define CAR_ANALYZER_LOG_LEVEL_DEBUG (4)
#define CAR_ANALYZER_LOG_LEVEL_VERBOSE (5)

#ifndef CONFIG_CAR_ANALYZER_LOG_DEFAULT_LEVEL
#define CONFIG_CAR_ANALYZER_LOG_DEFAULT_LEVEL CAR_ANALYZER_LOG_LEVEL_NONE
#endif

#ifndef CAR_ANALYZER_LOG_LEVEL
#define CAR_ANALYZER_LOG_LEVEL CAR_ANALYZER_LOG_LEVEL_NONE
#endif

#define CAR_ANALYZER_LOG_COLOR_E
#define CAR_ANALYZER_LOG_COLOR_W
#define CAR_ANALYZER_LOG_COLOR_I
#define CAR_ANALYZER_LOG_COLOR_D
#define CAR_ANALYZER_LOG_COLOR_V
#define CAR_ANALYZER_LOG_RESET_COLOR

	const char *pathToFileName(const char *path);
	int log_printf(const char *fmt, ...);

#define CAR_ANALYZER_SHORT_LOG_FORMAT(letter, format) CAR_ANALYZER_LOG_COLOR_##letter format CAR_ANALYZER_LOG_RESET_COLOR "\r\n"
#define CAR_ANALYZER_LOG_FORMAT(letter, format) CAR_ANALYZER_LOG_COLOR_##letter "[" #letter "][%lu][%s:%u] %s(): " format CAR_ANALYZER_LOG_RESET_COLOR "\r\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__

#if CAR_ANALYZER_LOG_LEVEL >= CAR_ANALYZER_LOG_LEVEL_VERBOSE
#define CarAnalyzerLog_v(format, ...) log_printf(CAR_ANALYZER_LOG_FORMAT(V, format), ##__VA_ARGS__)
#define CarAnalyzerLogIsr_log_v(format, ...) ets_printf(CAR_ANALYZER_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define CarAnalyzerLog_v(format, ...)
#define CarAnalyzerLogIsr_log_v(format, ...)
#endif

#if CAR_ANALYZER_LOG_LEVEL >= CAR_ANALYZER_LOG_LEVEL_DEBUG
#define CarAnalyzerLog_d(format, ...) log_printf(CAR_ANALYZER_LOG_FORMAT(D, format), ##__VA_ARGS__)
#define CarAnalyzerLogIsr_log_d(format, ...) ets_printf(CAR_ANALYZER_LOG_FORMAT(D, format), ##__VA_ARGS__)
#else
#define CarAnalyzerLog_d(format, ...)
#define CarAnalyzerLogIsr_log_d(format, ...)
#endif

#if CAR_ANALYZER_LOG_LEVEL >= CAR_ANALYZER_LOG_LEVEL_INFO
#define CarAnalyzerLog_i(format, ...) log_printf(CAR_ANALYZER_LOG_FORMAT(I, format), ##__VA_ARGS__)
#define CarAnalyzerLogIsr_log_i(format, ...) ets_printf(CAR_ANALYZER_LOG_FORMAT(I, format), ##__VA_ARGS__)
#else
#define CarAnalyzerLog_i(format, ...)
#define CarAnalyzerLogIsr_log_i(format, ...)
#endif

#if CAR_ANALYZER_LOG_LEVEL >= CAR_ANALYZER_LOG_LEVEL_WARN
#define CarAnalyzerLog_w(format, ...) log_printf(CAR_ANALYZER_LOG_FORMAT(W, format), ##__VA_ARGS__)
#define CarAnalyzerLogIsr_log_w(format, ...) ets_printf(CAR_ANALYZER_LOG_FORMAT(W, format), ##__VA_ARGS__)
#else
#define CarAnalyzerLog_w(format, ...)
#define CarAnalyzerLogIsr_log_w(format, ...)
#endif

#if CAR_ANALYZER_LOG_LEVEL >= CAR_ANALYZER_LOG_LEVEL_ERROR
#define CarAnalyzerLog_e(format, ...) log_printf(CAR_ANALYZER_LOG_FORMAT(E, format), ##__VA_ARGS__)
#define CarAnalyzerLogIsr_log_e(format, ...) ets_printf(CAR_ANALYZER_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define CarAnalyzerLog_e(format, ...)
#define CarAnalyzerLogIsr_log_e(format, ...)
#endif

#if CAR_ANALYZER_LOG_LEVEL >= CAR_ANALYZER_LOG_LEVEL_NONE
#define CarAnalyzerLog_n(format, ...) log_printf(CAR_ANALYZER_LOG_FORMAT(E, format), ##__VA_ARGS__)
#define CarAnalyzerLogIsr_log_n(format, ...) ets_printf(CAR_ANALYZER_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define CarAnalyzerLog_n(format, ...)
#define CarAnalyzerLogIsr_log_n(format, ...)
#endif

#include "esp_log.h"

#ifdef CONFIG_CAR_ANALYZER_ESP_LOG
#undef ESP_LOGE
#undef ESP_LOGW
#undef ESP_LOGI
#undef ESP_LOGD
#undef ESP_LOGV
#undef ESP_EARLY_LOGE
#undef ESP_EARLY_LOGW
#undef ESP_EARLY_LOGI
#undef ESP_EARLY_LOGD
#undef ESP_EARLY_LOGV

#define ESP_LOGE(tag, ...) log_e(__VA_ARGS__)
#define ESP_LOGW(tag, ...) log_w(__VA_ARGS__)
#define ESP_LOGI(tag, ...) log_i(__VA_ARGS__)
#define ESP_LOGD(tag, ...) log_d(__VA_ARGS__)
#define ESP_LOGV(tag, ...) log_v(__VA_ARGS__)
#define ESP_EARLY_LOGE(tag, ...) isr_log_e(__VA_ARGS__)
#define ESP_EARLY_LOGW(tag, ...) isr_log_w(__VA_ARGS__)
#define ESP_EARLY_LOGI(tag, ...) isr_log_i(__VA_ARGS__)
#define ESP_EARLY_LOGD(tag, ...) isr_log_d(__VA_ARGS__)
#define ESP_EARLY_LOGV(tag, ...) isr_log_v(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CAR_ANALYZER_LOG_H__ */
