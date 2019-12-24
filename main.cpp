#include "log_manager.hpp"

int main()
{
	LOG_INIT("my_application.log");
		
	LOG_TRACE << "this is a trace" << std::endl;
	LOG_INFO << "this is an info" << std::endl;
	LOG_DEBUG << "this is a debug" << std::endl;
	LOG_WARN << "this is a warn" << std::endl;
	LOG_ERROR << "this is a error" << std::endl;
	
	return 0;
}