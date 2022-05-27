#pragma once
#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

class LogDuration {
public:
	using Clock = std::chrono::steady_clock;

	LogDuration(const std::string& id, std::ostream& print = std::cerr)
		: id_(id), print_(print) {
	}

	~LogDuration() {
		using namespace std::chrono;
		using namespace std::literals;
		const auto end_time = Clock::now();
		const auto dur = end_time - start_time_;
		print_ << "Operation time: "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
	}

private:
	const std::string id_;
	std::ostream& print_;
	const Clock::time_point start_time_ = Clock::now();
};

