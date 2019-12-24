#pragma once
// at the base of https://stackoverflow.com/questions/40273809/how-to-write-iostream-like-interface-to-logging-library/40424272#40424272

#include <sstream>
#include <string>
#include <map>
#include <shared_mutex>

namespace lg {

	enum log_level
	{
		trace_level = 0,
		debug_level,
		info_level,
		warn_level,
		error_level
	};

	const std::map<log_level, std::string> levels_str = {
		{ trace_level, "trace" },
		{ debug_level, "debug" },
		{ info_level,  " info"  },
		{ warn_level,  " warn"  },
		
		{ error_level, "error" },
	};

	class logger_interface
	{
	public:
		explicit logger_interface(const log_level min_level = trace_level) : minimum_level_(min_level)  {}
		virtual ~logger_interface() = default;

		logger_interface(const logger_interface&) = default;
		logger_interface(logger_interface&&) = default;
		logger_interface& operator= (const logger_interface&) = default;
		logger_interface& operator= (logger_interface&&) = default;
		
	
		virtual void accept(const std::string& data, log_level level) = 0;
		[[nodiscard]] static std::string current_date_time();
		[[nodiscard]] static std::string head(log_level l);
		[[nodiscard]] log_level min_level() const { return minimum_level_;  }

		
	private:
		log_level minimum_level_;
	};

	class console_logger final  : public logger_interface
	{
	public:
		explicit console_logger(log_level min_level = trace_level) : logger_interface(min_level) {}
		virtual void accept(const std::string& data, log_level level) override;
	};

	class file_logger final : public logger_interface
	{
	public:
		explicit file_logger(const std::string& file_name, log_level min_level = trace_level);
		virtual void accept(const std::string& data, log_level level) override;

	private:
		std::string filename_;
		
	};
	
	class log_manager final
	{
		log_manager() : m_log_level_(info_level) {}
		static std::unique_ptr<log_manager> instance_;
		static std::once_flag onceFlag_;
		
	public:
		static log_manager* instance();
		virtual ~log_manager() = default;
		
		log_manager(const log_manager&) = delete;
		log_manager(log_manager&&) = delete;
		log_manager& operator= (const log_manager&) = delete;
		log_manager& operator= (log_manager&&) = delete;

		static void add_consumer(const std::string& name, std::shared_ptr<logger_interface> logger);
		
		template<class T>  // int, double, strings, etc
		log_manager& operator<<(const T& output)
		{
			m_stream_ << output;
			return *this;
		}

		log_manager& operator<<(std::ostream&  (*mip)(std::ostream&));// endl, flush, setw, setfill, etc.
		log_manager& operator<<(std::ios_base& (*mip)(std::ios_base&));// setiosflags, resetiosflags
		log_manager& operator()(const log_level e);
		void flush();

	private:
		std::stringstream m_stream_;
		log_level m_log_level_;

		static std::map<std::string, std::shared_ptr<logger_interface>> consumers_;
	};
}

#define LOG_ADD_CONSOLE lg::log_manager::instance()->add_consumer("console", std::shared_ptr<lg::logger_interface>(new lg::console_logger()))
#define LOG_ADD_FILE(log_name) lg::log_manager::instance()->add_consumer("file", std::shared_ptr<lg::logger_interface>(new lg::file_logger(log_name)))
#define LOG_INIT(log_name) LOG_ADD_CONSOLE; LOG_ADD_FILE(log_name)

#define LOG  (*lg::log_manager::instance())
#define LOG_TRACE  LOG(lg::log_level::trace_level)
#define LOG_DEBUG  LOG(lg::log_level::debug_level)
#define LOG_INFO  LOG(lg::log_level::info_level)
#define LOG_WARN  LOG(lg::log_level::warn_level)
#define LOG_ERROR  LOG(lg::log_level::error_level)