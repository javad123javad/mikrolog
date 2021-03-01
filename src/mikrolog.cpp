/*
 ============================================================================
 Name        : mikrolog.cpp
 Author      : Javad Rahimi
 Version     :
 Copyright   : MIT
 Description : Hello World in C++,
 ============================================================================
 */

#include "lib/logger.hpp"
#include <iostream>

using namespace std;

int main(void) {
	logger_nm::logger *logging = logger_nm::logger::get_instance("dymmy.log",logger_nm::LOG_TRACE);//get_instance("dummy_log.log", logger_nm::LOG_TRACE);
	logging->log_info("%s:%d %s", __FILE__, __LINE__, "brake or steer not in center_position");
	logging->log_trace("%s:%d %s", __FILE__, __LINE__, "brake or steer not in center_position");
	logging->log_warn("%s:%d %s", __FILE__, __LINE__, "brake or steer not in center_position");
	logging->log_error("%s:%d %s", __FILE__, __LINE__, "brake or steer not in center_position");

	cout << "Mikrolog" << endl;  /* prints Mikrolog */
	return 0;
}
