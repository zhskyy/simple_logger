#include "log_manager.hpp"
#include <iostream>
#include <fstream>

std::map<std::string, std::shared_ptr<lg::logger_interface>> lg::log_manager::consumers_;
std::unique_ptr<lg::log_manager> lg::log_manager::instance_;
std::once_flag lg::log_manager::onceFlag_;

std::string lg::logger_interface::current_date_time()
{
	const auto now = time(nullptr);
	struct tm  tstruct;
	localtime_s(&tstruct, &now);
	std::stringstream ts;
	ts << "[" << 1900 + tstruct.tm_year << "-" << tstruct.tm_mon << "-" << tstruct.tm_mday << "T" << tstruct.tm_hour << ":" << tstruct.tm_min << ":" << tstruct.tm_sec << "]";
	return ts.str();
}

std::string lg::logger_interface::head(log_level l)
{
	std::stringstream ts;
	ts << "[";
	const auto o = levels_str.find(l);
	ts << (o == levels_str.end() ? "-----" : o->second);
	ts << "] ";
	return ts.str();
}

void lg::console_logger::accept(const std::string& data, log_level level)
{
	if(level >= min_level())
	{
		std::cout << head(level) << current_date_time() << " " << data;
	}
}

lg::file_logger::file_logger(const std::string& file_name, log_level min_level)
	: logger_interface(min_level), filename_(file_name)
{
	std::ofstream fout;
	fout.open( filename_.empty() ? std::string("app.log") : filename_,
		std::ios_base::out | std::ios_base::app);
	fout << std::endl << " ------------------------------------- " << std::endl;
}

void lg::file_logger::accept(const std::string& data, log_level level)
{
	if (level >= min_level())
	{
		std::ofstream fout;
		fout.open((filename_.empty() ? std::string("app.log") : filename_),
			std::ios_base::out | std::ios_base::app);
		fout << head(level) << current_date_time() << " " << data;
	}
}

lg::log_manager* lg::log_manager::instance()
{
	std::call_once(onceFlag_, [] {
		instance_.reset(new log_manager);
	});
	return instance_.get();
}

void lg::log_manager::add_consumer(const std::string& name, std::shared_ptr<logger_interface> logger)
{
	if (consumers_.end() == consumers_.find(name))
		consumers_[name] = logger;
}

lg::log_manager& lg::log_manager::operator<<(std::ostream&(* mip)(std::ostream&))
{
	mip(m_stream_);

	if (mip == static_cast<std::ostream&  (*)(std::ostream&)>(std::flush)
		|| mip == static_cast<std::ostream&  (*)(std::ostream&)>(std::endl))
		this->flush();

	return *this;
}

lg::log_manager& lg::log_manager::operator<<(std::ios_base&(* mip)(std::ios_base&))
{
	mip(m_stream_);
	return *this;
}

lg::log_manager& lg::log_manager::operator()(const log_level e)
{
	m_log_level_ = e;
	return *this;
}

void lg::log_manager::flush()
{
	for (const auto& consumer : consumers_)
	{
		consumer.second->accept(m_stream_.str(), m_log_level_);
	}

	m_log_level_ = info_level;
	m_stream_.str(std::string());
	m_stream_.clear();
}
