#include "../../include/model/Enrollment.h"
#include <chrono>
#include <iomanip>
#include <sstream>

Enrollment::Enrollment(std::string studentId, std::string courseId, EnrollmentStatus status)
    : studentId_(std::move(studentId)),
      courseId_(std::move(courseId)),
      status_(status),
      enrollmentTime_(getCurrentTimeString()) {
}

Enrollment::Enrollment(Enrollment&& other) noexcept
    : studentId_(std::move(other.studentId_)),
      courseId_(std::move(other.courseId_)),
      status_(other.status_),
      enrollmentTime_(std::move(other.enrollmentTime_)) {
}

Enrollment& Enrollment::operator=(Enrollment&& other) noexcept {
    if (this != &other) {
        studentId_ = std::move(other.studentId_);
        courseId_ = std::move(other.courseId_);
        status_ = other.status_;
        enrollmentTime_ = std::move(other.enrollmentTime_);
    }
    return *this;
}

std::string Enrollment::getStatusString() const {
    switch (status_) {
        case EnrollmentStatus::ENROLLED:
            return "已选";
        case EnrollmentStatus::DROPPED:
            return "已退";
        case EnrollmentStatus::WAITLISTED:
            return "等待名单";
        default:
            return "未知";
    }
}

std::string Enrollment::getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}