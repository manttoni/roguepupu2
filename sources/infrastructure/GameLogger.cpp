#include "infrastructure/GameLogger.hpp"

#include <algorithm>
#include <utility>

#include "utils/ECS.hpp"

GameLogger::Stream::Stream(
		GameLogger& logger,
		const entt::registry& registry)
	: logger_(&logger),
	registry_(&registry)
{
}

GameLogger::Stream::Stream(Stream&& other) noexcept
: logger_(std::exchange(other.logger_, nullptr)),
	registry_(std::exchange(other.registry_, nullptr)),
	buffer_(std::move(other.buffer_))
{
}

GameLogger::Stream::~Stream()
{
	if (logger_ == nullptr)
		return;

	auto message = std::move(buffer_).str();

	if (!message.empty())
		logger_->log_message(std::move(message));
}

GameLogger::Stream& GameLogger::Stream::operator<<(entt::entity entity)
{
	if (entity == entt::null)
	{
		buffer_ << "<null>";
		return *this;
	}

	if (!registry_->valid(entity))
	{
		buffer_ << "<invalid entity>";
		return *this;
	}

	buffer_ << ECS::get_colored_name(*registry_, entity);
	return *this;
}

GameLogger::Stream GameLogger::stream(const entt::registry& registry)
{
	return Stream{*this, registry};
}

std::vector<std::string>
GameLogger::get_last_messages(std::size_t count) const
{
	count = std::min(count, messages_.size());

	return {
		messages_.end() - static_cast<std::ptrdiff_t>(count),
			messages_.end()
	};
}

void GameLogger::log_message(std::string message)
{
	messages_.push_back(std::move(message));
}

void GameLogger::clear()
{
	messages_.clear();
}

bool GameLogger::empty() const noexcept
{
	return messages_.empty();
}

std::size_t GameLogger::size() const noexcept
{
	return messages_.size();
}
