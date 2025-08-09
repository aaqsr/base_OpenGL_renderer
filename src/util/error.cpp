#include "util/error.hpp"
#include "util/logger.hpp"

IrrecoverableError::IrrecoverableError(std::string msg) : msg{std::move(msg)}
{
    Logger::enable();
    Logger::log(std::string{">>> ERROR: "} + this->msg, true);
}
